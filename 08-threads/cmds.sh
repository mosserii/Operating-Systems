gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 1_simple_thread.c -o ONE
gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 2_return.c -o TWO
gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 3_intermediate_thread.c -o THREE
gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 4_hard_thread.c -o FOUR
gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 5_expert_thread.c -o FIVE