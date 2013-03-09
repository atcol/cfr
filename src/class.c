#include "class.h"
#include <endian.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Class read_class(FILE *class_file) {
	uint16_t minorVersion;  	//= malloc(sizeof(uint16_t));
	uint16_t majorVersion; 		// = malloc(sizeof(uint16_t));
	uint16_t constPoolSize; 	//= malloc(sizeof(uint16_t));

	fread(&minorVersion,  sizeof(uint16_t), 1, class_file);
	fread(&majorVersion,  sizeof(uint16_t), 1, class_file);
	fread(&constPoolSize, sizeof(uint16_t), 1, class_file);
	
	// convert the big endian ints to host equivalents
	minorVersion = be16toh(minorVersion);
	majorVersion = be16toh(majorVersion);
	constPoolSize = be16toh(constPoolSize);

	Class class = {
		minorVersion,
		majorVersion,
		constPoolSize
	};

	return class;
}

bool is_class(FILE *class_file) {
	uint32_t magicNum;
	size_t num_read = fread(&magicNum, sizeof(uint32_t), 1, class_file);
	return num_read == 1 && magicNum == 0xbebafeca; // big endian
}

void print_class(FILE *stream, const char *name, const Class class) {
	fprintf(stream, "File: %s\n", name);
	fprintf(stream, "Minor number: %u \n", class.minorVersion);
	fprintf(stream, "Major number: %u \n", class.majorVersion);
	fprintf(stream, "Constant pool size: %u \n", class.constPoolSize);
}
