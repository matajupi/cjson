CC=gcc
CFLAGS=-Werror -std=c11

test: bin/test
	./bin/test

bin/test: bin/cjson.o bin/lexer.o bin/parser.o bin/context.o bin/util.o bin/test.o
	$(CC) $(LDFLAGS) -o $@ $^

bin/cjson.o: cjson.h cjson.c
	$(CC) $(CFLAGS) -o $@ -c cjson.c

bin/lexer.o: cjson.h lexer.c
	$(CC) $(CFLAGS) -o $@ -c lexer.c

bin/parser.o: cjson.h parser.c
	$(CC) $(CFLAGS) -o $@ -c parser.c

bin/context.o: cjson.h context.c
	$(CC) $(CFLAGS) -o $@ -c context.c

bin/util.o: cjson.h util.c
	$(CC) $(CFLAGS) -o $@ -c util.c

bin/test.o: test.c
	$(CC) $(CFLAGS) -o $@ -c $^

clean:
	rm -rf bin
	mkdir bin

.PHONY: clean test
