#include "class.h"
#include <endian.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *args[]) {
	if (argc == 1) {
		printf("Please pass at least 1 .class file to open");
		exit(EXIT_FAILURE);
	}

	int i;
	for (i = 1; i < argc; i++) {
		char *file_name = args[i];
		FILE *file = fopen(file_name, "r");

		if (!file) {
			printf("Could not open '%s': %s\n", file_name, strerror(errno));
			continue;
		}

		// Check the file header for .class nature
		if (!is_class(file)) {
			printf("Skipping '%s': not a valid class file\n", file_name);
			continue;
		}

		const ClassFile class_file = {
			file_name,
			file
		};

		const Class class = read_class(class_file);
		print_class(stdout, class);
		fclose(file);
	}

	exit(EXIT_SUCCESS);
}

