// Timur Abdullaev 251ADB075
// https://github.com/timaabdullaev/C_Language_Lab
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: 

#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

// moves the pointer forward while it sees spaces or tabs
static void skip_spaces(const char **p) {
    while (**p == ' ' || **p == '\t') {
        (*p)++;
    }
}

// returns 1 on success 0 if the string is invalid or overflow happens
int eval_line(const char *line, long long *result) {
    const char *p = line;
    char *end;
    skip_spaces(&p);
    
    // if string consists only of spaces or is empty there is nothing to evaluate
    if (*p == '\0') {
        return -1; 
    }

    // read the first number
    errno = 0;
    long long current_val = strtoll(p, &end, 10);
    if (p == end || errno == ERANGE) {
        return 0; // error no number or out of bounds
    }
    p = end;

    // in loop we search for the operator and the next number
    while (1) {
        skip_spaces(&p);
        if (*p == '\0') {
            break;
        }

        char op = *p;
        if (op != '+' && op != '-') {
            return 0; // encounter anything other than a plus or a minus
        }
        p++; // skip the operator sign itself

        skip_spaces(&p);
        
        // read the next number
        errno = 0;
        long long next_val = strtoll(p, &end, 10);
        if (p == end || errno == ERANGE) {
            return 0; // error when after operator there is no number or out of bounds
        }
        p = end;

        // using operator with overflow checks
        if (op == '+') {
            if ((next_val > 0 && current_val > LLONG_MAX - next_val) ||
                (next_val < 0 && current_val < LLONG_MIN - next_val)) {
                return 0; // overflow on addition
            }
            current_val += next_val;
        } else {
            if ((next_val > 0 && current_val < LLONG_MIN + next_val) ||
                (next_val < 0 && current_val > LLONG_MAX + next_val)) {
                return 0; // overflow on subtraction
            }
            current_val -= next_val;
        }
    }

    // write the result according to the pointer
    *result = current_val;
    return 1;
}
int main(int argc, char *argv[]) {
    //check the number of arguments
    if (argc != 4) {
        fprintf(stderr, "Usage: %s -o OUTDIR input.txt\n", argv[0]);
        return 1;
    }

    // create the output directory 
    mkdir(argv[2], 0777);

    //Extract the base file name
    const char *input_path = argv[3];
    const char *base_name = strrchr(input_path, '/');
    if (base_name != NULL) {
        base_name++; //skip the '/' character 
    } else {
        base_name = input_path; // if there are no slashes take the whole string
    }

    // Cut off ".txt" from the end if it's there
    size_t base_len = strlen(base_name);
    if (base_len >= 4 && strcmp(base_name + base_len - 4, ".txt") == 0) {
        base_len -= 4;
    }

    // format the final path for the output file
    char out_path[1024];
    snprintf(out_path, sizeof(out_path), "%s/%.*s_timur_abdullaev_251ADB075.txt", argv[2], (int)base_len, base_name);

    // open files
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

    //read and process loop
    char line[1024];
    while (fgets(line, sizeof(line), in_file)) {
        //trim newline characters (\n and \r\n)
        line[strcspn(line, "\r\n")] = '\0';
        
        long long result;
        int status = eval_line(line, &result);
        
        if (status == 1) {
            fprintf(out_file, "%lld\n", result);
        }
        // status == -1  empty lineis ignored
        // status == 0  syntax error or overflow  is ignored for now
    }

    //close files
    fclose(in_file);
    fclose(out_file);

    return 0;
}
