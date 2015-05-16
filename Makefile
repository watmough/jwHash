

CC=gcc-4.9
CFLAGS = -lpthread
DEFS = -DHASHTEST -DHASHTHREADED
DEPS = jwHash.h
OBJ = jwHash.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(DEFS)

jwHash: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY:
	clean

clean:
	rm *.o jwHash *.s