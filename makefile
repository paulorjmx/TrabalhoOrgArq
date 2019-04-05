all:
	rm -rf data.cdbf
	gcc handle_file.c main.c -I. -o main