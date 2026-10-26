#!/bin/bash
gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm || exit 1

fail=0
check() {   # name, input (printf format), expected output (printf format)
    printf -- "$2" > t_in.txt
    printf -- "$3" > t_exp.txt
    rm -rf t_out
    ./calc -o t_out t_in.txt
    if diff -q t_exp.txt t_out/t_in_timur_abdullaev_251ADB075.txt > /dev/null; then
        echo "PASS  $1"
    else
        echo "FAIL  $1"
        echo "  expected:"; sed 's/^/    /' t_exp.txt
        echo "  got:";      sed 's/^/    /' t_out/t_in_timur_abdullaev_251ADB075.txt
        fail=1
    fi
}

# valid files
check "level 4 chain"         '2 + 5 - 3\n'                   '4\n'
check "level 6 precedence"    '3 + 2 * 5 - 4\n7 / 2\n8 / 2 * 3\n' '9\n3\n12\n'
check "no final newline"      '5 + 5'                         '10\n'
check "only blank lines"      '\n\n'                          ''
check "blank lines skipped"   '1 + 1\n\n2 * 3\n'              '2\n6\n'

# errors: only ERROR:<pos>, no partial results
check "level 5 two-line"      '1 + 2\n3 +'                    'ERROR:10\n'
check "mixed file, offsets"   '2 + 5 - 3\n\n7\t-\t10\n42\n9223372036854775807 + 1\nabc\n' 'ERROR:44\n'
check "div by zero line 2"    '1 + 1\n5 + 4 / 0 * 2\n'        'ERROR:15\n'
check "CRLF line endings"     '1 + 1\r\n2 +\r\n'              'ERROR:11\n'
check "unary minus first"     '-3 + 4\n'                      'ERROR:1\n'
check "unary minus after op"  '2 * -3\n'                      'ERROR:5\n'
check "two ops in a row"      '7 + * 3\n'                     'ERROR:5\n'
check "div by zero"           '10 / 0\n'                      'ERROR:6\n'

# usage errors must exit non-zero
./calc 2>/dev/null;                      [ $? -ne 0 ] && echo "PASS  no args"      || { echo "FAIL  no args"; fail=1; }
./calc -o t_out missing.txt 2>/dev/null; [ $? -ne 0 ] && echo "PASS  missing file"  || { echo "FAIL  missing file"; fail=1; }

rm -rf t_in.txt t_exp.txt t_out
exit $fail