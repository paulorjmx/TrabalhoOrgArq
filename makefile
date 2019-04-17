all:
	gcc handle_file.c main.c -I. -o main
run:
	./main
