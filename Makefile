

default:
	gcc -g -o qoi src/*.c


tests:
	gcc -g test/*.c src/encoder.c src/common.c src/constants.c -lcriterion && ./a.out
