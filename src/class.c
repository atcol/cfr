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
	uint16_t const_pool_count;

	fread(&minor_version, sizeof(uint16_t), 1, class_file.file);
	fread(&major_version, sizeof(uint16_t), 1, class_file.file);
	fread(&const_pool_count, sizeof(uint16_t), 1, class_file.file);
	
	// convert the big endian ints to host equivalents
	minor_version = be16toh(minor_version);
	major_version = be16toh(major_version);
	const_pool_count = be16toh(const_pool_count);

	Class *class = (Class *) malloc(sizeof(Class));
	class->file_name = class_file.file_name;
	class->minor_version = minor_version;
	class->major_version = major_version;
	class->const_pool_count = const_pool_count;
	class->pool_size_bytes = parse_const_pool(class, const_pool_count, class_file);
	return class;
}

uint32_t parse_const_pool(Class *class, const uint16_t const_pool_count, const ClassFile class_file) {
	uint16_t item_id = 0;
	uint32_t table_size_bytes = 0;
	int i;
	char tag_byte;
	Ref r;

	for (i = 1; i < const_pool_count; i++) {
		fread(&tag_byte, sizeof(char), 1, class_file.file);
		Item *item = (Item *) malloc(sizeof(Item));
		item->tag = tag_byte;
		// Populate item based on tag_byte
		switch (tag_byte) {
			case STRING_UTF8: // String prefixed by a uint16 indicating the number of bytes in the encoded string which immediately follows
				item->id = ++item_id;
				String s;
				char *str;
				fread(&s.length, sizeof(s.length), 1, class_file.file);
				s.length = be16toh(s.length);
				str = malloc(sizeof(char) * s.length);
				fread(str, sizeof(char), s.length, class_file.file);
				s.value = str;
				item->value.string = s;
				item->label = "String";
				table_size_bytes += 2 + s.length;
				break;
			case INTEGER: // Integer: a signed 32-bit two's complement number in big-endian format
				item->id = ++item_id;
				fread(&item->value.integer, sizeof(item->value.integer), 1, class_file.file);
				item->value.integer = be32toh(item->value.integer);
				item->label = "Integer";
				table_size_bytes += 4;
				break;
			case FLOAT: // Float: a 32-bit single-precision IEEE 754 floating-point number
				item->id = ++item_id;
				fread(&item->value.flt, sizeof(item->value.flt), 1, class_file.file);
				table_size_bytes += 4;
				item->label = "Float";
				break;
			case LONG: // Long: a signed 64-bit two's complement number in big-endian format (takes two slots in the constant pool table)
				item->id = ++item_id;
				uint32_t high, low;
				fread(&high, sizeof(high), 1, class_file.file);
				fread(&low, sizeof(low), 1, class_file.file);
				item->value.lng = ((long) be32toh(high) << 32) + be32toh(low);
				item->label = "Long";
				++item_id;
				table_size_bytes += 8;
				break;
			case DOUBLE: // Double: a 64-bit double-precision IEEE 754 floating-point number (takes two slots in the constant pool table)
				item->id = ++item_id;
				uint32_t high, low;
				fread(&high, sizeof(high), 1, class_file.file);
				fread(&low, sizeof(low), 1, class_file.file);
				item->value.dbl = ((long) be32toh(high) << 32) + be32toh(low);
				item->label = "Double";
				++item_id;
				table_size_bytes += 8;
				break;
			case CLASS: // Class reference: an uint16 within the constant pool to a UTF-8 string containing the fully qualified class name
				item->id = ++item_id;
				fread(&r.class_idx, sizeof(r.class_idx), 1, class_file.file);
				r.class_idx = be16toh(r.class_idx);
				item->value.ref = r;
				item->label = "Class ref";
				table_size_bytes += 2;
				break;
			case STRING: // String reference: an uint16 within the constant pool to a UTF-8 string
				item->id = ++item_id;
				fread(&r.class_idx, sizeof(r.class_idx), 1, class_file.file);
				r.class_idx = be16toh(r.class_idx);
				item->value.ref = r;
				item->label = "String ref";
				table_size_bytes += 2;
				break;
			case FIELD: // Field reference: two uint16 within the pool, 1st pointing to a Class reference, 2nd to a Name and Type descriptor
				item->label = "Field ref";
				/* FALL THROUGH TO NEXT */
			case METHOD: // Method reference: two uint16s within the pool, 1st pointing to a Class reference, 2nd to a Name and Type descriptor
				if (item->label == NULL) item->label = "Method ref";
				/* FALL THROUGH TO NEXT */
			case INTERFACE_METHOD: // Interface method reference: 2 uint16 within the pool, 1st pointing to a Class reference, 2nd to a Name and Type descriptor
				item->id = ++item_id;
				fread(&r.class_idx, sizeof(r.class_idx), 1, class_file.file);
				fread(&r.name_idx, sizeof(r.name_idx), 1, class_file.file);
				r.class_idx = be16toh(r.class_idx);
				r.name_idx = be16toh(r.name_idx);
				item->value.ref = r;
				if (item->label == NULL) item->label = "Interface method ref";
				table_size_bytes += 4;
				break;
			case NAME: // Name and type descriptor: 2 uint16 to UTF-8 strings, 1st representing a name (identifier), 2nd a specially encoded type descriptor
				item->id = ++item_id;
				fread(&r.class_idx, sizeof(r.class_idx), 1, class_file.file);
				fread(&r.name_idx, sizeof(r.name_idx), 1, class_file.file);
				r.class_idx = be16toh(r.class_idx);
				r.name_idx = be16toh(r.name_idx);
				item->value.ref = r;
				if (item->label == NULL) item->label = "Name ref";
				table_size_bytes += 4;
				break;
			default:
				printf("Found tag byte '%d' but don't know what to do with it\n", tag_byte);
				break;
		}
		HASH_ADD(hh, class->items, id, sizeof(item->id), item);
	}
	return table_size_bytes;
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
	fprintf(stream, "Constant pool size: %u \n", class->const_pool_count);
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
		} else if (s->tag == CLASS || s->tag == STRING) {
			fprintf(stream, "%u\n", s->value.ref.class_idx);
		} else if(s->tag == FIELD || s->tag == METHOD || s->tag == INTERFACE_METHOD || s->tag == NAME) {
			fprintf(stream, "%u.%u\n", s->value.ref.class_idx, s->value.ref.name_idx);
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
		HASH_FIND(hh, class->items, &r->class_idx, sizeof(r->class_idx), item);
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
