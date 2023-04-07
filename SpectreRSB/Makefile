CC=aarch64-linux-gnu-gcc

a.out: source.c
	$(CC) -O0 source.c -pie -std=gnu99 -save-temps -lpthread -fPIC
	
b.out: source1.c
	$(CC) -O0 source1.c -pie -std=gnu99 -lpthread -fPIC -o b.out

clear:
	rm *.out