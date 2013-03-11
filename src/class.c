#include "class.h"
#include <endian.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Class read_class(const ClassFile class_file) {
	uint16_t minor_version;
	uint16_t major_version;
	uint16_t const_pool_size;

	fread(&minor_version,  sizeof(uint16_t), 1, class_file.file);
	fread(&major_version,  sizeof(uint16_t), 1, class_file.file);
	fread(&const_pool_size, sizeof(uint16_t), 1, class_file.file);
	
	// convert the big endian ints to host equivalents
	minor_version = be16toh(minor_version);
	major_version = be16toh(major_version);
	const_pool_size = be16toh(const_pool_size);

	Class class = {
		class_file.file_name,
		minor_version,
		major_version,
		const_pool_size
	};

	return class;
}

bool is_class(FILE *class_file) {
	uint32_t magicNum;
	size_t num_read = fread(&magicNum, sizeof(uint32_t), 1, class_file);
	return num_read == 1 && be32toh(magicNum) == 0xcafebabe;
}

void print_class(FILE *stream, const Class class) {
	fprintf(stream, "File: %s\n", class.file_name);
	fprintf(stream, "Minor number: %u \n", class.minor_version);
	fprintf(stream, "Major number: %u \n", class.major_version);
	fprintf(stream, "Constant pool size: %u \n", class.const_pool_size);
}
