#include "class.h"
#include <endian.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "uthash.h"

Class *read_class(const ClassFile class_file) {
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

	uint16_t item_id = 0;
	uint16_t field_id = 0;
	uint16_t meth_id = 0;
	uint16_t ifacemeth_id = 0;
	uint32_t table_size_bytes = 0;
	int i;
	char tag_byte;
	int32_t int32;
	float flt;
	long lng;
	double dbl;
	uint16_t uint16;
	char *str;
	Ref *method;
	Ref *field;
	
	for (i = 1; i < const_pool_size; i++) {
		fread(&tag_byte, sizeof(char), 1, class_file.file);
		printf("Tag byte: %d\n", tag_byte);
		Item *item = (Item *) malloc(sizeof(Item));
		item->tag = tag_byte;
		// Populate item based on tag_byte
		switch (tag_byte) {
			case STRING_UTF8: // String prefixed by a 16-bit number (type u2) indicating the number of bytes in the encoded string which immediately follows
				//FIXME: parse the UTF encoded string
				fread(&uint16, sizeof(uint16), 1, class_file.file);
				uint16 = be16toh(uint16);
				str = malloc(sizeof(char) * uint16 + 1);
				fread(str, sizeof(char), uint16, class_file.file);
				str[uint16] = '\0';
				String s;
				s.length = uint16;
				s.value = str;
				item->id = ++item_id;
				item->value.string = s;
				item->label = "String";
				table_size_bytes += 2 + uint16 + 1;
				HASH_ADD(hh, class->items, id, sizeof(item->id), item);
				printf("Added String '%s' with id %u@%p\n", item->value.string.value, item->id, (void *) &item->id);
				Item *t;
				HASH_FIND(hh, class->items, &item_id, sizeof(item_id), t); //FIXME: why does &item_id fail here?!
				if (t != NULL) printf("Item is %s\n", t->value.string.value);
				else printf("t is NULL :( \n");
				break;
			case INTEGER: // Integer: a signed 32-bit two's complement number in big-endian format
				fread(&int32, sizeof(int32), 1, class_file.file);
				item->id = ++item_id;
				item->value.integer = be32toh(int32);
				item->label = "Integer";
				table_size_bytes += 4;
				HASH_ADD(hh, class->items, id, sizeof(item->id), item);
				printf("Added Integer with id %d\n", item->id);
				break;
			case FLOAT: // Float: a 32-bit single-precision IEEE 754 floating-point number
				fread(&flt, sizeof(flt), 1, class_file.file);
				item->id = ++item_id;
				item->value.flt = flt;
				table_size_bytes += 4;
				item->label = "Float";
				HASH_ADD(hh, class->items, id, sizeof(item->id), item);
				printf("Added Float with id %d\n", item->id);
				break;
			case LONG: // Long: a signed 64-bit two's complement number in big-endian format (takes two slots in the constant pool table)
				fread(&lng, sizeof(lng), 1, class_file.file);
				item->id = ++item_id;
				item->value.lng = be64toh(lng);
				table_size_bytes += 8;
				item->label = "Long";
				HASH_ADD(hh, class->items, id, sizeof(item->id), item);
				printf("Added Long with id %d\n", item->id);
				break;
			case DOUBLE: // Double: a 64-bit double-precision IEEE 754 floating-point number (takes two slots in the constant pool table)
				fread(&dbl, sizeof(dbl), 1, class_file.file);
				item->id = ++item_id;
				item->value.dbl = dbl;
				item->label = "Double";
				HASH_ADD(hh, class->items, id, sizeof(item->id), item);
				table_size_bytes += 8;
				break;
			case CLASS: // Class reference: an uint16 within the constant pool to a UTF-8 string containing the fully qualified class name
				fread(&uint16, sizeof(uint16), 1, class_file.file);
				//item->id = ++item_id;
				//item->value.class_idx = be16toh(uint16);
				table_size_bytes += 2;
				//item->label = "Class";
				break;
			case STRING: // String reference: an uint16 within the constant pool to a UTF-8 string
				fread(&uint16, sizeof(uint16), 1, class_file.file);
				//item->id = ++item_id;
				//item->value.class_idx = be16toh(uint16);
				table_size_bytes += 2;
				//HASH_ADD_INT(class->
				break;
			case FIELD: // Field reference: two uint16 within the pool, 1st pointing to a Class reference, 2nd to a Name and Type descriptor
				field = (Ref *) malloc(sizeof(Ref));
				fread(&field->class_idx, sizeof(uint16), 1, class_file.file);
				fread(&field->name_idx, sizeof(uint16), 1, class_file.file);
				field->id = ++field_id;
				field->class_idx = be16toh(field->class_idx);
				field->name_idx = be16toh(field->name_idx);
				HASH_ADD(hh, class->fields, id, sizeof(field->id), field);
				table_size_bytes += 4;
				break;
			case METHOD: // Method reference: two uint16s within the pool, 1st pointing to a Class reference, 2nd to a Name and Type descriptor
				method = (Ref *) malloc(sizeof(Ref));
				fread(&method->class_idx, sizeof(uint16), 1, class_file.file);
				fread(&method->name_idx, sizeof(uint16), 1, class_file.file);
				method->id = ++meth_id;
				method->class_idx = be16toh(method->class_idx);
				method->name_idx = be16toh(method->name_idx);
				table_size_bytes += 4;
				HASH_ADD(hh, class->methods, id, sizeof(method->id), method);
				break;
			case INTERFACE_METHOD: // Interface method reference: 2 uint16 within the pool, 1st pointing to a Class reference, 2nd to a Name and Type descriptor
				method = (Ref *) malloc(sizeof(Ref));
				fread(&method->class_idx, sizeof(uint16), 1, class_file.file);
				fread(&method->name_idx, sizeof(uint16), 1, class_file.file);
				method->id = ++ifacemeth_id;
				method->class_idx = be16toh(method->class_idx);
				method->name_idx = be16toh(method->name_idx);
				HASH_ADD(hh, class->methods, id, sizeof(method->id), method);
				table_size_bytes += 4;
				break;
			case NAME: // Name and type descriptor: 2 uint16 to UTF-8 strings, 1st representing a name (identifier), 2nd a specially encoded type descriptor
				fread(&uint16, sizeof(uint16), 1, class_file.file);
				printf("Name uint16 1: %d\n", be16toh(uint16));
				fread(&uint16, sizeof(uint16), 1, class_file.file);
				printf("Name uint16 2: %d\n", be16toh(uint16));
				table_size_bytes += 4;
				break;
		}
	}

	class->pool_size_bytes = table_size_bytes;
	return class;
}

bool is_class(FILE *class_file) {
	uint32_t magicNum;
	size_t num_read = fread(&magicNum, sizeof(uint32_t), 1, class_file);
	return num_read == 1 && be32toh(magicNum) == 0xcafebabe;
}

void print_class(FILE *stream, const Class *class) {
	fprintf(stream, "File: %s\n", class->file_name);
	fprintf(stream, "Minor number: %u \n", class->minor_version);
	fprintf(stream, "Major number: %u \n", class->major_version);
	fprintf(stream, "Constant pool size: %u \n", class->const_pool_size);
	fprintf(stream, "Constant table size: %ub \n", class->pool_size_bytes);
	fprintf(stream, "Printing constant pool of %d items...\n", HASH_COUNT(class->items));

	Item *s;
	int i = 1;
	for (s = class->items; s != NULL; s = s->hh.next) {
		fprintf(stream, "Item #%d (%u, %s): ", i, s->id, s->label);
		if (s->tag == STRING_UTF8) {
			fprintf(stream, "%s\n", s->value.string.value);
		} else if (s->tag == INTEGER) {
			fprintf(stream, "%d\n", s->value.integer);
		} else if (s->tag == FLOAT) {
			fprintf(stream, "%f\n", s->value.flt);
		} else if (s->tag == LONG) {
			fprintf(stream, "%ld\n", s->value.lng);
		} else if (s->tag == DOUBLE) {
			fprintf(stream, "%lf\n", s->value.dbl);
		} else if (s->tag == CLASS) {
			fprintf(stream, "%d\n", s->value.class_idx);
		} else if (s->tag == STRING) {
			fprintf(stream, "%d\n", s->value.class_idx);
		} else if (s->tag == FIELD) {
			fprintf(stream, "%d\n", s->value.class_idx);
		} else if (s->tag == METHOD) {
			fprintf(stream, "%d\n", s->value.class_idx);
		} else {
			fprintf(stream, "Don't know how to print item # %d of type %d\n", i, s->tag);
		}
		i++;
	}

	fprintf(stream, "Printing %d methods...\n", HASH_COUNT(class->methods));
	Ref *r;
	for (r = class->methods; r != NULL; r = r->hh.next) {
		fprintf(stream, "Method class/name: %u/%u ->", r->class_idx, r->name_idx);
		Item *item;
		HASH_FIND(hh, class->items, &r->class_idx, sizeof(r->class_idx), item); // not _INT macro because uint16_t keys
		if (item != NULL) {
			fprintf(stream, "%s.", item->value.string.value);
			HASH_FIND(hh, class->items, &r->name_idx, sizeof(r->name_idx), item);
			if (item) fprintf(stream, "%s\n", item->value.string.value);
		} else {
			fprintf(stream, " (lookup failed)\n" );
		}
	}

	fprintf(stream, "Printing %d fields...\n", HASH_COUNT(class->fields));
	Ref *f;
	for (f = class->fields; f != NULL; f = f->hh.next) {
		fprintf(stream, "Field class/name: %d/%d\n", f->class_idx, f->name_idx);
		Item *item;
		HASH_FIND(hh, class->items, &f->class_idx, sizeof(f->class_idx), item);
		if (item != NULL) {
			fprintf(stream, "%s.", item->value.string.value);
			HASH_FIND(hh, class->items, &f->name_idx, sizeof(f->name_idx), item);
			if (item) fprintf(stream, "%s\n", item->value.string.value);
		} else {
			fprintf(stream, " (lookup failed)\n" );
		}
	}
}
