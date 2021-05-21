all: myls.c
	gcc -o myls myls.c
	
clean:
	$(RM) myls
