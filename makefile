
lexan:
	gcc -o lexan lexan.c lexmain.c

parser: lexan.o
	gcc -o parser parser.c lexan.c

codegen: lexan.o
	gcc -o codegen parser.c lexan.c genasm.c codegen.c

clean:
	rm lexan.o parser.o codegen.o lexan parser codegen
