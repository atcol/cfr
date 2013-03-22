#ifndef CLASS_H
#define CLASS_H
#include <endian.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "uthash.h"

enum cpool_t {
	STRING_UTF8     = 1, /* occupies 2+x bytes */
	INTEGER         = 3, /* 32bit two's-compliment big endian int */
	FLOAT           = 4, /* 32-bit single precision */
	LONG            = 5, /* Long: a signed 64-bit two's complement number in big-endian format (takes two slots in the constant pool table) */
	DOUBLE          = 6, /* Double: a 64-bit double-precision IEEE 754 floating-point number (takes two slots in the constant pool table) */
	CLASS           = 7, /* Class reference: an index within the constant pool to a UTF-8 string containing the fully qualified class name (in internal format) */
	STRING          = 8, /* String reference: an index within the constant pool to a UTF-8 string */
	FIELD           = 9, /* Field reference: two indexes within the constant pool, the first pointing to a Class reference, the second to a Name and Type descriptor. */
	METHOD          = 10, /* Method reference: two indexes within the constant pool, the first pointing to a Class reference, the second to a Name and Type descriptor. */
	INTERFACE_METHOD = 11, /* Interface method reference: two indexes within the constant pool, the first pointing to a Class reference, the second to a Name and Type descriptor. */
	NAME            = 12 /* Name and type descriptor: 2 indexes to UTF-8 strings, the first representing a name and the second a specially encoded type descriptor. */
};

/* A wrapper for FILE structs that also holds the file name.  */
typedef struct {
	char *file_name;
	FILE *file;
} ClassFile;

/* Wraps references to an item in the constant pool */
typedef struct {
	uint16_t id;
	uint16_t class_idx;
	uint16_t name_idx;
	UT_hash_handle hh;
} Ref;

typedef struct {
	int length;
	char *value;
} String;

typedef struct {
	uint16_t id;
	uint8_t tag; // the tag byte
	char *label;
	union {
		String string;
		float flt;
		double dbl;
		long lng;
		int32_t integer;
		uint16_t class_idx;
		uint16_t name_idx;
	} value;
	UT_hash_handle hh;
} Item;

/* The .class structure */
typedef struct {
	char *file_name;
	uint16_t minor_version;
	uint16_t major_version;
	uint16_t const_pool_size;
	uint32_t pool_size_bytes;
	Item *items;
	uint16_t this_class;
	uint16_t super_class;
	uint16_t interface_count;
	Ref *interfaces;
	uint16_t field_count;
	Ref *fields;
	uint16_t method_count;
	Ref *methods;
	uint16_t attribute_count;
	Ref *attributes;
} Class;

/* Return true if class_file's first four bytes match 0xcafebabe. */
bool is_class(FILE *class_file);

/* Parse the given class file into a Class struct. */
Class *read_class(const ClassFile class_file);

/* Write the name and class stats/contents to the given stream. */
void print_class(FILE *stream, const Class *class);

#endif //CLASS_H__
