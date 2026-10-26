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

// 1 on success, 0 on error, -1 on empty line
int eval_line(const char *line, long long *result, size_t *err_pos) {
    const char *p = line;
    char *end;

    skip_spaces(&p);
    
    if (*p == '\0') {
        return -1; 
    }

    const char *num_start = p;
    
    // allow leading negative sign
    if (*p == '-') {
        p++;
    }
    
    // strict digit check to block multiple operators
    if (!isdigit((unsigned char)*p)) {
        *err_pos = (size_t)(num_start - line);
        return 0; 
    }
    p = num_start;

    errno = 0;
    long long current_val = strtoll(p, &end, 10);
    if (errno == ERANGE) {
        *err_pos = (size_t)(num_start - line);
        return 0; 
    }
    p = end;

    // process left to right
    while (1) {
        skip_spaces(&p);
        
        if (*p == '\0') {
            break;
        }

        char op = *p;
        if (op != '+' && op != '-') {
            *err_pos = (size_t)(p - line);
            return 0; 
        }
        p++; 

        skip_spaces(&p);
        
        num_start = p;
        // block multiple or dangling operators
        if (!isdigit((unsigned char)*p)) {
            *err_pos = (size_t)(p - line);
            return 0;
        }
        
        errno = 0;
        long long next_val = strtoll(p, &end, 10);
        if (errno == ERANGE) {
            *err_pos = (size_t)(num_start - line);
            return 0; 
        }
        p = end;

        // overflow checks
        if (op == '+') {
            if ((next_val > 0 && current_val > LLONG_MAX - next_val) ||
                (next_val < 0 && current_val < LLONG_MIN - next_val)) {
                *err_pos = (size_t)(num_start - line);
                return 0; 
            }
            current_val += next_val;
        } else {
            if ((next_val > 0 && current_val < LLONG_MIN + next_val) ||
                (next_val < 0 && current_val > LLONG_MAX + next_val)) {
                *err_pos = (size_t)(num_start - line);
                return 0; 
            }
            current_val -= next_val;
        }
    }

    *result = current_val;
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
            fprintf(out_file, "ERROR:%zu\n", error_pos);
        }
        
        global_offset += len;
    }

    fclose(in_file);
    fclose(out_file);

    return 0;
}