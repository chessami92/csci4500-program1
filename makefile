all: shell

shell: shell.c prog1.c
	cc -o prog1 prog1.c shell.c

test: shell.c shell_test.c
	cc -o prog1_test shell_test.c shell.c
	./prog1_test < TestData

clean:
	rm -f prog1
	rm -f prog1_test
	rm -f core
