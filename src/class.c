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

	fread(&minor_version, sizeof(uint16_t), 1, class_file.file);
	fread(&major_version, sizeof(uint16_t), 1, class_file.file);
	fread(&const_pool_size, sizeof(uint16_t), 1, class_file.file);
	
	// convert the big endian ints to host equivalents
	minor_version = be16toh(minor_version);
	major_version = be16toh(major_version);
	const_pool_size = be16toh(const_pool_size);

	uint32_t table_size_bytes = 0;
	int i;
	char tag_byte;
	int32_t int32;
	float flt;
	long lng;
	double dbl;
	uint16_t index;
	char *str;
	
	fread(&tag_byte, sizeof(char), 1, class_file.file);
	for (i = 0; i < const_pool_size; i++) {
		printf("Tag byte: %d\n", tag_byte);
		switch (tag_byte) {
			case 1: 
					fread(&index, sizeof(index), 1, class_file.file);
					table_size_bytes += 2 + index;
					printf("String size (bytes): %d\n", index);
					str = malloc(sizeof(char) * index);
					fread(str, sizeof(char), index, class_file.file);
					printf("String: %s\n", str);
					free(str);
					break;
			case 3:
					fread(&int32, sizeof(int32), 1, class_file.file);
					printf("int32: %d\n", int32);
					table_size_bytes += 4;
					break;
			case 4:
					fread(&flt, sizeof(flt), 1, class_file.file);
					printf("float: %f\n", flt);
					table_size_bytes += 4;
					break;
			case 5:
					fread(&lng, sizeof(lng), 1, class_file.file);
					printf("long: %ld\n", lng);
					table_size_bytes += 8;
					break;
			case 6:
					fread(&dbl, sizeof(dbl), 1, class_file.file);
					printf("double: %f\n", dbl);
					table_size_bytes += 8;
					break;
			case 7:
					fread(&index, sizeof(index), 1, class_file.file);
					printf("Class index: %d\n", index);
					table_size_bytes += 2;
					break;
			case 8:
					fread(&index, sizeof(index), 1, class_file.file);
					printf("String index: %d\n", index);
					table_size_bytes += 2;
					break;
			case 9:
					fread(&index, sizeof(index), 1, class_file.file);
					printf("Field index 1: %d\n", index);
					fread(&index, sizeof(index), 1, class_file.file);
					printf("Field index 1: %d\n", index);
					table_size_bytes += 4;
					break;
			case 10:
					fread(&index, sizeof(index), 1, class_file.file);
					printf("Method index 1: %d\n", index);
					fread(&index, sizeof(index), 1, class_file.file);
					printf("Method index 2: %d\n", index);
					table_size_bytes += 4;
					break;
			case 11:
					fread(&index, sizeof(index), 1, class_file.file);
					printf("Interface index 1: %d\n", index);
					fread(&index, sizeof(index), 1, class_file.file);
					printf("Interface index 2: %d\n", index);
					table_size_bytes += 4;
					break;
			case 12:
					fread(&index, sizeof(index), 1, class_file.file);
					printf("Name index 1: %d\n", index);
					fread(&index, sizeof(index), 1, class_file.file);
					printf("Name index 2: %d\n", index);
					table_size_bytes += 4;
					break;
		}
		fread(&tag_byte, sizeof(char), 1, class_file.file);
	}

	printf("Table size: %d\n", table_size_bytes);

	Class class = {
		class_file.file_name,
		minor_version,
		major_version,
		const_pool_size,
		table_size_bytes
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
	fprintf(stream, "Constant table size: %ub \n", class.const_table_size_bytes);
}
