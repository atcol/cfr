#ifndef CLASS_H
#define CLASS_H
#include <endian.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define u2 uint16_t
#define u4 uint32_t

typedef enum {
	ACC_PUBLIC 		= 0x0001,
	ACC_FINAL 		= 0x0010,
	ACC_SUPER 		= 0x0020,
	ACC_INTERFACE 	= 0x0200,
	ACC_ABSTRACT 	= 0x0400,
	ACC_SYNTHETIC 	= 0x1000,
	ACC_ANNOTATION 	= 0x2000,
	ACC_ENUM 		= 0x4000
} AccessFlags;

typedef struct {
	uint16_t name_idx;
	uint32_t length;
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
	uint16_t flags;
	uint16_t name_idx;
	uint16_t desc_idx;
	uint16_t attrs_count;
	Attribute *attrs;
} Field;

typedef struct {
	uint32_t high;
	uint32_t low;
} Long;

typedef struct {
	uint16_t flags;
	uint16_t name_idx;
	uint16_t desc_idx;
	uint16_t attrs_count;
	Attribute *attrs;
} Method;

/* Wraps references to an item in the constant pool */
typedef struct {
	uint16_t class_idx;
	uint16_t name_idx;
} Ref;

typedef struct {
	uint16_t length;
	char *value;
} String;

typedef struct {
	uint8_t tag; // the tag byte
	union {
		String string;
		float flt;
		Double dbl;
		Long lng;
		int32_t integer;
		Ref ref; /* A method, field or interface reference */
	} value;
} Item;

/* The .class structure */
typedef struct {
	char *file_name;
	uint16_t minor_version;
	uint16_t major_version;
	uint16_t const_pool_count;
	uint32_t pool_size_bytes;
	Item *items;
	uint16_t flags;
	uint16_t this_class;
	uint16_t super_class;
	uint16_t interfaces_count;
	Ref *interfaces;
	uint16_t fields_count;
	Field *fields;
	uint16_t methods_count;
	Method *methods;
	uint16_t attributes_count;
	Attribute *attributes;
} Class;

typedef enum {
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
} CPool_t;

static char *CPool_strings[] = {
	"Undefined", // 0
	"String_UTF8",
	"Undefined", // 2
	"Integer",
	"Float",
	"Long",
	"Double",
	"Class",
	"String",
	"Field",
	"Method",
	"InterfaceMethod",
	"Name",
	"Undefined", // 13
	"Undefined", // 14
	"MethodHandle",
	"MethodType",
	"InvokeDynamic"
};

enum RANGES {
	/* The smallest permitted value for a tag byte */
	MIN_CPOOL_TAG = 1,

	/* The largest permitted value for a tag byte */
	MAX_CPOOL_TAG = 18
};

/* Delegate to read_class(ClassFile) */
Class *read_class_from_file_name(char *f);

/* Parse the given class file into a Class struct. */
Class *read_class(const ClassFile class_file);

/* Parse the attribute properties from file into attr. Assumes class_file.file is at offset relative to reading an attribute struct.
 * See section 4.7 of the JVM spec. */
void parse_attribute(ClassFile class_file, Attribute *attr);

/* Parse the constant pool into class from class_file. ClassFile.file MUST be at the correct seek point i.e. byte offset 11.
 * The number of bytes read is returned. A return value of 0 signifies an invalid constant pool and class may have been changed.
 * See section 4.4 of the JVM spec.
 */
void parse_const_pool(Class *class, const uint16_t const_pool_count, const ClassFile class_file);

/* Parse the initial section of the given class_file up to and including the constant_pool_size section */
void parse_header(ClassFile class_file, Class *class);

/* Parse the initial section of the given class_file up to and including the constant_pool_size section */
void parse_header(ClassFile class_file, Class *class);

/* Return true if class_file's first four bytes match 0xcafebabe. */
bool is_class(FILE *class_file);

/* Return the item pointed to by cp_idx, the index of an item in the constant pool */
Item *get_item(const Class *class, const uint16_t cp_idx);

/* Resolve a Class's name by following class->items[index].ref.class_idx */
Item *get_class_string(const Class *class, const uint16_t index);

/* Convert the high and low bits of dbl to a double type */
double to_double(const Double dbl);

/* Convert the high and low bits of lng to a long type */
long to_long(const Long lng);

/* Convert the 2-byte field type to a friendly string e.g. "J" to "long" */
char *field2str(const char fld_type);

/* Convert tag byte to its string name/label */
static inline char *tag2str(uint8_t tag) {
	return CPool_strings[tag];
}

/* Write the name and class stats/contents to the given stream. */
void print_class(FILE *stream, const Class *class);

#endif //CLASS_H__
