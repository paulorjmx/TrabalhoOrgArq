all:
	gcc handle_file.c func_aux.c main.c -I. -lm -o main
run:
	./main
