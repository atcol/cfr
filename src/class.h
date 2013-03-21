#ifndef CLASS_H
#define CLASS_H
#include <endian.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "uthash.h"

/* A wrapper for FILE structs that also holds the file name.  */
typedef struct {
	char *file_name;
	FILE *file;
} ClassFile;

typedef struct {
	int id;
	uint16_t class_idx; // class reference
	uint16_t name; // name and type ref
	UT_hash_handle hh;
} Method;

typedef struct {
	int id;
	UT_hash_handle hh;
} Interface;

typedef struct {
	int id;
	uint16_t class_idx;
	uint16_t name;
	UT_hash_handle hh;
} Field;

typedef struct {
	int id;
	uint16_t class_idx;
	uint16_t name;
	UT_hash_handle hh;
} Attribute;

typedef struct {
	int id;
	int length;
	char *value;
} String;

typedef struct {
	int id;
	uint8_t tag; // the tag byte
	union {
		String string;
		double dbl;
		long lng;
		int integer;
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
	Interface *interfaces;
	uint16_t field_count;
	Field *fields;
	uint16_t method_count;
	Method *methods;
	uint16_t attribute_count;
	Attribute *attributes;
} Class;

/* Return true if class_file's first four bytes match 0xcafebabe. */
bool is_class(FILE *class_file);

/* Parse the given class file into a Class struct. */
Class read_class(const ClassFile class_file);

/* Write the name and class stats/contents to the given stream. */
void print_class(FILE *stream, const Class class);

#endif //CLASS_H__
