all:
	gcc handle_file.c handle_index.c func_aux.c main.c -I. -lm -o main
run:
	./main
