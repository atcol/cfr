#include "class.h"
#include <endian.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "uthash.h"

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

	Class *class = (Class *) malloc(sizeof(Class));
	class->file_name = class_file.file_name;
	class->minor_version = minor_version;
	class->major_version = major_version;
	class->const_pool_size = const_pool_size;

	uint32_t table_size_bytes = 0;
	int i;
	char tag_byte;
	int32_t int32;
	float flt;
	long lng;
	double dbl;
	uint16_t uint16;
	char *str;
	Method *method;
	Field *field;
	
	for (i = 0; i < const_pool_size; i++) {
		fread(&tag_byte, sizeof(char), 1, class_file.file);
		printf("Tag byte: %d\n", tag_byte);
		Item *item = (Item *) malloc(sizeof(Item));
		item->tag = tag_byte;

		// Populate item based on tag_byte
		switch (tag_byte) {
			case 1: // String prefixed by a 16-bit number (type u2) indicating the number of bytes in the encoded string which immediately follows
				fread(&uint16, sizeof(uint16), 1, class_file.file);
				uint16 = be16toh(uint16);
				str = malloc(sizeof(char) * uint16 + 1);
				fread(str, sizeof(char), uint16, class_file.file);
				str[uint16] = '\0';
				String s;
				s.length = uint16;
				s.value = str;
				item->value.string = s;
				HASH_ADD_INT(class->items, id, item);
				table_size_bytes += 2 + uint16 + 1;
				//FIXME: read the UTF encoding in the string
				break;
			case 3: // Integer: a signed 32-bit two's complement number in big-endian format
				fread(&int32, sizeof(int32), 1, class_file.file);
				printf("int32: %d\n", be32toh(int32));
				table_size_bytes += 4;
				break;
			case 4: // Float: a 32-bit single-precision IEEE 754 floating-point number
				fread(&flt, sizeof(flt), 1, class_file.file);
				printf("float: %f\n", flt);
				table_size_bytes += 4;
				break;
			case 5: // Long: a signed 64-bit two's complement number in big-endian format (takes two slots in the constant pool table)
				fread(&lng, sizeof(lng), 1, class_file.file);
				printf("long: %ld\n", be64toh(lng));
				table_size_bytes += 8;
				break;
			case 6: // Double: a 64-bit double-precision IEEE 754 floating-point number (takes two slots in the constant pool table)
				fread(&dbl, sizeof(dbl), 1, class_file.file);
				printf("double: %f\n", dbl);
				table_size_bytes += 8;
				break;
			case 7: // Class reference: an uint16 within the constant pool to a UTF-8 string containing the fully qualified class name
				fread(&uint16, sizeof(uint16), 1, class_file.file);
				printf("Class uint16: %d\n", be16toh(uint16));
				table_size_bytes += 2;
				break;
			case 8: // String reference: an uint16 within the constant pool to a UTF-8 string
				fread(&uint16, sizeof(uint16), 1, class_file.file);
				printf("String uint16: %d\n", be16toh(uint16));
				table_size_bytes += 2;
				break;
			case 9: // Field reference: two uint16 within the pool, 1st pointing to a Class reference, 2nd to a Name and Type descriptor
				field = (Field *) malloc(sizeof(Field));
				fread(&field->class_idx, sizeof(uint16), 1, class_file.file);
				fread(&field->name, sizeof(uint16), 1, class_file.file);
				field->class_idx = be16toh(field->class_idx);
				field->name = be16toh(field->name);
				printf("Field class: %d\n", field->class_idx);
				printf("Field name: %d\n", field->name);
				table_size_bytes += 4;
				HASH_ADD_INT(class->fields, id, field);
				break;
			case 10: // Method reference: two uint16s within the pool, 1st pointing to a Class reference, 2nd to a Name and Type descriptor
				method = (Method *) malloc(sizeof(Method));
				fread(&method->class_idx, sizeof(uint16), 1, class_file.file);
				method->class_idx = be16toh(method->class_idx);
				fread(&method->name, sizeof(uint16), 1, class_file.file);
				method->name = be16toh(method->name);
				printf("Method uint16 1: %d\n", method->class_idx);
				printf("Method uint16 2: %d\n", method->name);
				table_size_bytes += 4;
				HASH_ADD_INT(class->methods, id, method);
				break;
			case 11: // Interface method reference: 2 uint16 within the pool, 1st pointing to a Class reference, 2nd to a Name and Type descriptor
				fread(&uint16, sizeof(uint16), 1, class_file.file);
				printf("Interface uint16 1: %d\n", be16toh(uint16));
				fread(&uint16, sizeof(uint16), 1, class_file.file);
				printf("Interface uint16 2: %d\n", be16toh(uint16));
				table_size_bytes += 4;
				break;
			case 12: // Name and type descriptor: 2 uint16 to UTF-8 strings, 1st representing a name (identifier), 2nd a specially encoded type descriptor
				fread(&uint16, sizeof(uint16), 1, class_file.file);
				printf("Name uint16 1: %d\n", be16toh(uint16));
				fread(&uint16, sizeof(uint16), 1, class_file.file);
				printf("Name uint16 2: %d\n", be16toh(uint16));
				table_size_bytes += 4;
				break;
		}
	}

	printf("Table size: %d\n", table_size_bytes);

	return *class;
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
	fprintf(stream, "Constant table size: %ub \n", class.pool_size_bytes);

	fprintf(stream, "Printing constant pool of %d items...\n", HASH_COUNT(class.items));

	Item *s;
	int i = 1;
	for (s = class.items; s != NULL; s = s->hh.next) {
		if (s->tag == 1) {
			fprintf(stream, "Item %d: %s\n", i, s->value.string.value);
		} else {
			fprintf(stream, "Don't know how to print item # %d\n", i);
		}
		i++;
	}

	fprintf(stream, "Printing %d methods...\n", HASH_COUNT(class.methods));
	Method *m;
	for (m = class.methods; m != NULL; m = m->hh.next) {
		fprintf(stream, "Method class/name: %d/%d\n", m->class_idx, m->name);
	}
	fprintf(stream, "Printing %d fields...\n", HASH_COUNT(class.fields));
	Field *f;
	for (f = class.fields; f != NULL; f = f->hh.next) {
		fprintf(stream, "Field class/name: %d/%d\n", f->class_idx, f->name);
	}
}
