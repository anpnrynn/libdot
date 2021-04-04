
all:dot_test1 dot_test2 dot_test3



dot_test1:dot_test1.c
	gcc dot_test1.c -o dot_test1 -I../src -L../build/src/.libs -ldot -g3

dot_test2:dot_test2.c
	gcc dot_test2.c -o dot_test2 -I../src -L../build/src/.libs/ -ldot -g3

dot_test3:dot_test3.c
	gcc dot_test3.c -o dot_test3 -I../src -L../build/src/.libs -ldot -g3

clean:
	rm -f dot_test1 dot_test2 dot_test3 core.* 
