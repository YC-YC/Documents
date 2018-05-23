so: test_a.c test_b.c test_c.c so_test.h
	gcc test_a.c test_b.c test_c.c -fPIC -shared -o libtest.so 
test: test.c libtest.so
	gcc test.c -L. -ltest -o test
