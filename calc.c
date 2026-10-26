// Timur Abdullaev 251ADB075
// https://github.com/timaabdullaev/C_Language_Lab
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: 

#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

// spec allows only spaces and tabs
static void skip_spaces(const char **p) {
    while (**p == ' ' || **p == '\t') {
        (*p)++;
    }
}

// shared parser state so nested calls can report errors
typedef struct {
    const char *line;
    const char *p;
    size_t err_pos;
} Parser;

// no unary minus in spec so a number must start with a digit
static int parse_primary(Parser *ps, long long *out) {
    skip_spaces(&ps->p);
    if (!isdigit((unsigned char)*ps->p)) {
        ps->err_pos = (size_t)(ps->p - ps->line);
        return 0;
    }
    char *end;
    errno = 0;
    long long v = strtoll(ps->p, &end, 10);
    if (errno == ERANGE) {
        ps->err_pos = (size_t)(ps->p - ps->line);
        return 0;
    }
    ps->p = end;
    *out = v;
    return 1;
}

// term is deeper than expr so * and / bind tighter than + and -
static int parse_term(Parser *ps, long long *out) {
    long long left;
    if (!parse_primary(ps, &left)) {
        return 0;
    }

    while (1) {
        skip_spaces(&ps->p);
        char op = *ps->p;
        if (op != '*' && op != '/') {
            break;
        }
        ps->p++;
        skip_spaces(&ps->p);

        // save start of right operand for div by zero error
        size_t right_pos = (size_t)(ps->p - ps->line);
        long long right;
        if (!parse_primary(ps, &right)) {
            return 0;
        }

        if (op == '/') {
            if (right == 0) {
                ps->err_pos = right_pos;
                return 0;
            }
            // c truncates toward zero like spec says
            left /= right;
        } else {
            // right is never negative here so one check is enough
            if (right != 0 && left > LLONG_MAX / right) {
                ps->err_pos = right_pos;
                return 0;
            }
            left *= right;
        }
    }
    *out = left;
    return 1;
}

// loop keeps + and - left associative
static int parse_expr(Parser *ps, long long *out) {
    long long left;
    if (!parse_term(ps, &left)) {
        return 0;
    }

    while (1) {
        skip_spaces(&ps->p);
        char op = *ps->p;
        if (op != '+' && op != '-') {
            break;
        }
        ps->p++;
        skip_spaces(&ps->p);

        size_t right_pos = (size_t)(ps->p - ps->line);
        long long right;
        if (!parse_term(ps, &right)) {
            return 0;
        }

        // overflow checks
        if (op == '+') {
            if ((right > 0 && left > LLONG_MAX - right) ||
                (right < 0 && left < LLONG_MIN - right)) {
                ps->err_pos = right_pos;
                return 0;
            }
            left += right;
        } else {
            if ((right > 0 && left < LLONG_MIN + right) ||
                (right < 0 && left > LLONG_MAX + right)) {
                ps->err_pos = right_pos;
                return 0;
            }
            left -= right;
        }
    }
    *out = left;
    return 1;
}

// 1 on success, 0 on error, -1 on empty line
int eval_line(const char *line, long long *result, size_t *err_pos) {
    Parser ps = { line, line, 0 };

    skip_spaces(&ps.p);

    if (*ps.p == '\0') {
        return -1;
    }

    long long value;
    if (!parse_expr(&ps, &value)) {
        *err_pos = ps.err_pos;
        return 0;
    }

    // leftover chars after a full expression are invalid
    skip_spaces(&ps.p);
    if (*ps.p != '\0') {
        *err_pos = (size_t)(ps.p - line);
        return 0;
    }

    *result = value;
    return 1;
}

int main(int argc, char *argv[]) {
    // enforce usage format
    if (argc != 4 || strcmp(argv[1], "-o") != 0) {
        fprintf(stderr, "Usage: %s -o OUTDIR input.txt\n", argv[0]);
        return 1;
    }

    // create outdir and ignore if exists
    if (mkdir(argv[2], 0777) != 0) {
        if (errno != EEXIST) {
            perror("Error creating output directory");
            return 1;
        }
    }

    // extract base filename for output
    const char *input_path = argv[3];
    const char *base_name = strrchr(input_path, '/');
    if (base_name != NULL) {
        base_name++; 
    } else {
        base_name = input_path; 
    }

    size_t base_len = strlen(base_name);
    if (base_len >= 4 && strcmp(base_name + base_len - 4, ".txt") == 0) {
        base_len -= 4;
    }

    char out_path[1024];
    snprintf(out_path, sizeof(out_path), "%s/%.*s_timur_abdullaev_251ADB075.txt", argv[2], (int)base_len, base_name);

    FILE *in_file = fopen(input_path, "r");
    if (!in_file) {
        fprintf(stderr, "Error opening input file\n");
        return 1;
    }

    FILE *out_file = fopen(out_path, "w");
    if (!out_file) {
        fprintf(stderr, "Error opening output file\n");
        fclose(in_file);
        return 1;
    }

    // global offset for absolute error position
    size_t global_offset = 0;
    char line[1024];

    while (fgets(line, sizeof(line), in_file)) {
        
        size_t len = strlen(line);
        
        line[strcspn(line, "\r\n")] = '\0';
        
        long long result;
        size_t err_pos = 0;
        int status = eval_line(line, &result, &err_pos);
        
        if (status == 1) {
            fprintf(out_file, "%lld\n", result);
        } else if (status == 0) {
            // calc 1-based global error pos
            size_t error_pos = global_offset + err_pos + 1;

            // reopen in w mode to wipe partial results, only the error stays
            fclose(out_file);
            out_file = fopen(out_path, "w");
            if (!out_file) {
                fprintf(stderr, "Error opening output file\n");
                fclose(in_file);
                return 1;
            }
            fprintf(out_file, "ERROR:%zu\n", error_pos);

            // only the first error counts
            break;
        }
        
        global_offset += len;
    }

    fclose(in_file);
    fclose(out_file);

    return 0;
}