#include <stdio.h>
#include "test_kernel_procs.h"

int
main(int argc, char **argv)
{
	if (argc < 3) {
		printf("Usage: %s device PIDs\n", argv[0]);
		return 1;
	}
	FILE *f = fopen(argv[1], "r+");
	for (int i = 0; i < argc - 2; i++) {
		unsigned pid;
		sscanf(argv[i + 2], "%u", &pid);
		fprintf(f, "%u\n", pid);
	}
	char buf[256];
	int n;
	while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
		//printf("READ %i\n", n);
		fwrite(buf, 1, n, stdout);
	}
	fclose(f);
}
