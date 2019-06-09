all:
	@gcc main.c handle_file.c handle_index.c func_aux.c -I. -lm -o main
run:
	@./main
