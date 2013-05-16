all: shell

shell: shell.c
	cc -o shell shell_main.c shell.c

test: shell.c shell_test.c
	cc -o shell_test shell_test.c shell.c
	./shell_test

clean:
	rm -f shell
	rm -f shell_test
	rm -f core
