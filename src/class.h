#ifndef CLASS_H
#define CLASS_H
#include <endian.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "uthash.h"

enum cpool_t {
	STRING_UTF8      = 1, /* occupies 2+x bytes */
	INTEGER          = 3, /* 32bit two's-compliment big endian int */
	FLOAT            = 4, /* 32-bit single precision */
	LONG             = 5, /* Long: a signed 64-bit two's complement number in big-endian format (takes two slots in the constant pool table) */
	DOUBLE           = 6, /* Double: a 64-bit double-precision IEEE 754 floating-point number (takes two slots in the constant pool table) */
	CLASS            = 7, /* Class reference: an index within the constant pool to a UTF-8 string containing the fully qualified class name (in internal format) */
	STRING           = 8, /* String reference: an index within the constant pool to a UTF-8 string */
	FIELD            = 9, /* Field reference: two indexes within the constant pool, the first pointing to a Class reference, the second to a Name and Type descriptor. */
	METHOD           = 10, /* Method reference: two indexes within the constant pool, the first pointing to a Class reference, the second to a Name and Type descriptor. */
	INTERFACE_METHOD = 11, /* Interface method reference: two indexes within the constant pool, the first pointing to a Class reference, the second to a Name and Type descriptor. */
	NAME             = 12, /* Name and type descriptor: 2 indexes to UTF-8 strings, the first representing a name and the second a specially encoded type descriptor. */
	METHOD_HANDLE 	 = 15,
	METHOD_TYPE 	 = 16,
	INVOKE_DYNAMIC 	 = 18
};

typedef struct {
	char *info;
} Attribute;

/* A wrapper for FILE structs that also holds the file name.  */
typedef struct {
	char *file_name;
	FILE *file;
} ClassFile;

typedef struct {
	uint32_t high;
	uint32_t low;
} Double;

typedef struct {
	uint16_t access_flags;
	uint16_t name_idx;
	uint16_t descriptor_idx;
	uint16_t attrs_count;
	Attribute *attrs;
} Field;

typedef struct {
	uint32_t high;
	uint32_t low;
} Long;

/* Wraps references to an item in the constant pool */
typedef struct {
	uint16_t id;
	uint16_t class_idx;
	uint16_t name_idx;
	UT_hash_handle hh;
} Ref;

typedef struct {
	uint16_t length;
	char *value;
} String;

typedef struct {
	uint16_t id;
	uint8_t tag; // the tag byte
	char *label;
	union {
		String string;
		float flt;
		Double dbl;
		Long lng;
		int32_t integer;
		Ref ref; /* A method, field or interface reference */
	} value;
	UT_hash_handle hh;
} Item;

/* The .class structure */
typedef struct {
	char *file_name;
	uint16_t minor_version;
	uint16_t major_version;
	uint16_t const_pool_count;
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

/* Parse the constant pool into class from class_file. ClassFile.file MUST be at the correct seek point i.e. byte offset 11 */
uint32_t parse_const_pool(Class *class, const uint16_t const_pool_count, const ClassFile class_file);

/* Parse the given class file into a Class struct. */
Class *read_class(const ClassFile class_file);

/* Return true if class_file's first four bytes match 0xcafebabe. */
bool is_class(FILE *class_file);

/* Convert the high and low bits of dbl to a double type */
double to_double(const Double dbl);

/* Convert the high and low bits of lng to a long type */
long to_long(const Long lng);

/* Write the name and class stats/contents to the given stream. */
void print_class(FILE *stream, const Class *class);

#endif //CLASS_H__
