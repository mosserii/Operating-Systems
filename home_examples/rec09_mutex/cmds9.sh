gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 cond_var_ex1.c -o ONE -lpthread
gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 mutex_ex1.c -o TWO -lpthread
gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 mutex_ex2.c -o THREE -lpthread
gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 mutex_ex3.c -o FOUR -lpthread
