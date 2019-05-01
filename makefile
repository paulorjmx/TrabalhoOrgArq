all:
	gcc handle_file.c func_aux.c main.c -I. -o main
run:
	./main
