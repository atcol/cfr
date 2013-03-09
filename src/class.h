#ifndef CLASS_H
#define CLASS_H
#include <endian.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* The .class file structure */
typedef struct {
	const uint16_t  minorVersion;
	const uint16_t  majorVersion;
	const uint16_t  constPoolSize;
} Class;


typedef struct {
	
} cp_item;

/*
 * Return true if class_file's first four bytes match 0xbebafeca.
 */
bool is_class(FILE *class_file);

/*
 * Parse the given class file into a Class struct.
 */
Class read_class(FILE *class_file);

/*
 * Write the name and class stats/contents to the given stream.
 */
void print_class(FILE *stream, const char *name, const Class class);

#endif //CLASS_H__
