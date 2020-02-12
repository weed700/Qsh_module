CC=gcc

usr_mod_qsh: user_mod_qsh.o user_ll_qsh.o
	gcc -o usr_mod_qsh user_mod_qsh.o user_ll_qsh.o -lpthread

user_mod_qsh.o: user_mod_qsh.c
	gcc -c -o user_mod_qsh.o user_mod_qsh.c

user_ll_qsh.o: user_ll_qsh.c
	gcc -c -o user_ll_qsh.o user_ll_qsh.c

clean:
	rm -f *.o
	rm -f usr_mod_qsh
	rm -f /usr/bin/usr_mod_qsh

install:
	cp ./usr_mod_qsh /usr/bin/usr_mod_qsh
