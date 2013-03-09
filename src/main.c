#include "class.h"
#include <endian.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int main(const int argc, const char *args[]) {
	if (argc == 1) {
		printf("Please pass at least 1 .class file to open");
		exit(EXIT_FAILURE);
	}

	int i;
	for (i = 1; i < argc; i++) {
		const char *file_name = args[i];
		FILE  *class_file = fopen(file_name, "r");

		if (!class_file) {
			printf("Could not open '%s': %s\n", file_name, strerror(errno));
			continue;
		}

		if (!is_class(class_file)) {
			printf("Skipping '%s', its magic number isn't 0xbebafeca\n", file_name);
			continue;
		}

		const Class class = read_class(class_file);
		print_class(stdout, file_name, class);

		fclose(class_file);
	}

	exit(EXIT_SUCCESS);
}

