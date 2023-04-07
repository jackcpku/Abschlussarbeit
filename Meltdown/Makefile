CC=aarch64-linux-gnu-gcc

a.out: source.c
	$(CC) -O0 source.c -pie -std=gnu99 -lpthread -fPIC

secret: secret.c
	$(CC) -O0 secret.c -pie -std=gnu99 -lpthread -fPIC -o secret.out

secret1: secret1.c
	$(CC) -O0 secret1.c -pie -std=gnu99 -lpthread -fPIC -o secret1.out

mpa: map_physical_address.c
	$(CC) -O0 map_physical_address.c -pie -std=gnu99 -lpthread -fPIC -o map_physical_address.out

clear:
	rm *.out