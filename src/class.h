#ifndef CLASS_H
#define CLASS_H
#include <endian.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

enum cpool_t {
	STRING_UTF8 = 1, /* occupies 2+x bytes */
	INTEGER = 3,
	FLOAT = 4,
	LONG = 5,
	DOUBLE = 6,
	CLASS = 7,
	STRING = 8,
	FIELD = 9,
	METHOD = 10,
	INTERFACE_METHOD = 11,
	NAME = 12
};

/*
 * A wrapper for FILE structs that also holds its file name.
 */
typedef struct {
	const char *file_name;
	FILE *file;
} ClassFile;

/* The .class file structure */
typedef struct {
	const char *file_name;
	const uint16_t minor_version;
	const uint16_t major_version;
	const uint16_t const_pool_size;
} Class;

/* An item in the Constant Pool */
typedef struct {
	const uint8_t type;
	const int len;
	const union {
		char *string;
	} value;
} CPItem;

/*
 * Return true if class_file's first four bytes match 0xbebafeca.
 */
bool is_class(FILE *class_file);

/*
 * Parse the given class file into a Class struct.
 */
Class read_class(const ClassFile class_file);

/*
 * Write the name and class stats/contents to the given stream.
 */
void print_class(FILE *stream, const Class class);

#endif //CLASS_H__
