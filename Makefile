

CC=gcc-4.9
CFLAGS = -lpthread -O3
DEFS = -DHASHTEST -DHASHTHREADED
DEPS = jwHash.h
OBJ = test.o jwHash.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(DEFS)

test: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY:
	clean

clean:
	rm *.o test *.s