all: tests

tests: tests.c utest.h ../ptedit_header.h
	gcc -Os tests.c -std=gnu99 -o tests -fsanitize=address

clean:
	rm -f tests
	
