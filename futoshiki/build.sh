gcc -c comparison.c -o comparison.o
gcc -c futoshiki.c -o futoshiki.o
gcc -c main.c -o main.o
gcc comparison.o futoshiki.o main.o -o futoshiki