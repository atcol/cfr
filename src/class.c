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

	fread(&class->access_flags, sizeof(class->access_flags), 1, class_file.file);
	class->access_flags = be16toh(class->access_flags);

	fread(&class->this_class, sizeof(class->this_class), 1, class_file.file);
	class->this_class = be16toh(class->this_class);

	fread(&class->super_class, sizeof(class->super_class), 1, class_file.file);
	class->super_class = be16toh(class->super_class);

	fread(&class->interfaces_count, sizeof(class->interfaces_count), 1, class_file.file);
	class->interfaces_count = be16toh(class->interfaces_count);

	int idx = 0;
	while (idx < class->interfaces_count) {
		Ref *r = (Ref *) malloc(sizeof(Ref));
		fread(&r->class_idx, sizeof(r->class_idx), 1, class_file.file);
		r->class_idx = be16toh(r->class_idx);
		r->id = idx;
		printf("class_idx for interface #%u is %u\n", idx, r->class_idx);
		HASH_ADD(hh, class->interfaces, id, sizeof(r->id), r);
		idx++;
	}

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
		item->id = ++item_id;
		// Populate item based on tag_byte
		switch (tag_byte) {
			case STRING_UTF8: // String prefixed by a uint16 indicating the number of bytes in the encoded string which immediately follows
				item->label = "String";
				String s;
				fread(&s.length, sizeof(s.length), 1, class_file.file);
				s.length = be16toh(s.length);
				s.value = malloc(sizeof(char) * s.length);
				fread(s.value, sizeof(char), s.length, class_file.file);
				item->value.string = s;
				table_size_bytes += 2 + s.length;
				break;
			case INTEGER: // Integer: a signed 32-bit two's complement number in big-endian format
				item->label = "Integer";
				fread(&item->value.integer, sizeof(item->value.integer), 1, class_file.file);
				item->value.integer = be32toh(item->value.integer);
				table_size_bytes += 4;
				break;
			case FLOAT: // Float: a 32-bit single-precision IEEE 754 floating-point number
				fread(&item->value.flt, sizeof(item->value.flt), 1, class_file.file);
				item->value.flt = be32toh(item->value.flt);
				table_size_bytes += 4;
				item->label = "Float";
				break;
			case LONG: // Long: a signed 64-bit two's complement number in big-endian format (takes two slots in the constant pool table)
				fread(&item->value.lng.high, sizeof(item->value.lng.high), 1, class_file.file); // 4 bytes
				fread(&item->value.lng.low, sizeof(item->value.lng.low), 1, class_file.file); // 4 bytes
				item->value.lng.high = be32toh(item->value.lng.high);
				item->value.lng.low = be32toh(item->value.lng.low);
				//item->value.lng = ((long) be32toh(high) << 32) + be32toh(low);
				item->label = "Long";
				// 8-byte consts take 2 pool entries
				++item_id;
				++i;
				table_size_bytes += 8;
				break;
			case DOUBLE: // Double: a 64-bit double-precision IEEE 754 floating-point number (takes two slots in the constant pool table)
				fread(&item->value.dbl.high, sizeof(item->value.dbl.high), 1, class_file.file); // 4 bytes
				fread(&item->value.dbl.low, sizeof(item->value.dbl.low), 1, class_file.file); // 4 bytes
				item->value.dbl.high = be32toh(item->value.dbl.high);
				item->value.dbl.low = be32toh(item->value.dbl.low);
				item->label = "Double";
				// 8-byte consts take 2 pool entries
				++item_id;
				++i;
				table_size_bytes += 8;
				break;
			case CLASS: // Class reference: an uint16 within the constant pool to a UTF-8 string containing the fully qualified class name
				fread(&r.class_idx, sizeof(r.class_idx), 1, class_file.file);
				r.class_idx = be16toh(r.class_idx);
				item->value.ref = r;
				item->label = "Class ref";
				table_size_bytes += 2;
				break;
			case STRING: // String reference: an uint16 within the constant pool to a UTF-8 string
				fread(&r.class_idx, sizeof(r.class_idx), 1, class_file.file);
				r.class_idx = be16toh(r.class_idx);
				item->value.ref = r;
				item->label = "String ref";
				table_size_bytes += 2;
				break;
			case FIELD: // Field reference: two uint16 within the pool, 1st pointing to a Class reference, 2nd to a Name and Type descriptor
				item->label = "Field ref";
				/* FALL THROUGH TO METHOD */
			case METHOD: // Method reference: two uint16s within the pool, 1st pointing to a Class reference, 2nd to a Name and Type descriptor
				if (item->label == NULL) item->label = "Method ref";
				/* FALL THROUGH TO INTERFACE_METHOD */
			case INTERFACE_METHOD: // Interface method reference: 2 uint16 within the pool, 1st pointing to a Class reference, 2nd to a Name and Type descriptor
				fread(&r.class_idx, sizeof(r.class_idx), 1, class_file.file);
				fread(&r.name_idx, sizeof(r.name_idx), 1, class_file.file);
				r.class_idx = be16toh(r.class_idx);
				r.name_idx = be16toh(r.name_idx);
				item->value.ref = r;
				if (item->label == NULL) item->label = "Interface method ref";
				table_size_bytes += 4;
				break;
			case NAME: // Name and type descriptor: 2 uint16 to UTF-8 strings, 1st representing a name (identifier), 2nd a specially encoded type descriptor
				fread(&r.class_idx, sizeof(r.class_idx), 1, class_file.file);
				fread(&r.name_idx, sizeof(r.name_idx), 1, class_file.file);
				r.class_idx = be16toh(r.class_idx);
				r.name_idx = be16toh(r.name_idx);
				item->value.ref = r;
				if (item->label == NULL) item->label = "Name ref";
				table_size_bytes += 4;
				break;
			default:
				fprintf(stderr, "Found tag byte '%d' but don't know what to do with it\n", tag_byte);
				item = NULL;
				break;
		}
		if (item != NULL) HASH_ADD(hh, class->items, id, sizeof(item->id), item);
	}
	return table_size_bytes;
}

bool is_class(FILE *class_file) {
	uint32_t magicNum;
	size_t num_read = fread(&magicNum, sizeof(uint32_t), 1, class_file);
	return num_read == 1 && be32toh(magicNum) == 0xcafebabe;
}

double to_double(const Double dbl) {
	return -dbl.high; //FIXME check the following implementation
	//unsigned long bits = ((long) be32toh(item->dbl.high) << 32) + be32toh(item->dbl.low);
	//if (bits == 0x7ff0000000000000L) {
		//return POSITIVE_INFINITY;
	//} else if (bits == 0xfff0000000000000L) {
		//return POSITIVE_INFINITY;
	//} else if ((bits > 0x7ff0000000000000L && bits < 0x7fffffffffffffffL) 
			//|| (bits > 0xfff0000000000001L && bits < 0xffffffffffffffffL)) {
		//return NaN;
	//} else {
		//int s = ((bits >> 63) == 0) ? 1 : -1; 
		//int e = (int)((bits >> 52) & 0x7ffL);
		//long m = (e == 0) 
			//? (bits & 0xfffffffffffffL) << 1 
			//: (bits & 0xfffffffffffffL) | 0x10000000000000L;
		//return s * m * (2 << (e - 1075)); //FIXME this is wrong
	//} 
}

long to_long(const Long lng) {
	return ((long) be32toh(lng.high) << 32) + be32toh(lng.low);
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
		fprintf(stream, "Item %u %s: ", s->id, s->label);
		if (s->tag == STRING_UTF8) {
			fprintf(stream, "%s\n", s->value.string.value);
		} else if (s->tag == INTEGER) {
			fprintf(stream, "%d\n", s->value.integer);
		} else if (s->tag == FLOAT) {
			fprintf(stream, "%f\n", s->value.flt);
		} else if (s->tag == LONG) {
			fprintf(stream, "%ld\n", to_long(s->value.lng));
		} else if (s->tag == DOUBLE) {
			fprintf(stream, "%lf\n", to_double(s->value.dbl));
		} else if (s->tag == CLASS || s->tag == STRING) {
			fprintf(stream, "%u\n", s->value.ref.class_idx);
		} else if(s->tag == FIELD || s->tag == METHOD || s->tag == INTERFACE_METHOD || s->tag == NAME) {
			fprintf(stream, "%u.%u\n", s->value.ref.class_idx, s->value.ref.name_idx);
		} else {
			fprintf(stream, "Don't know how to print item # %d of type %d\n", i, s->tag);
		}
		i++;
	}

	fprintf(stream, "Access flags: %d\n", class->access_flags); //TODO use bitwise ops to for printing flags e.g. switch

	Item *cl_item; // the initial class ref item whose value.ref.class_idx we use
	Item *cl_str;
	HASH_FIND(hh, class->items, &class->this_class, sizeof(class->this_class), cl_item);
	HASH_FIND(hh, class->items, &cl_item->value.ref.class_idx, sizeof(cl_item->value.ref.class_idx), cl_str);
	fprintf(stream, "This class: %s\n", cl_str->value.string.value);

	HASH_FIND(hh, class->items, &class->super_class, sizeof(class->super_class), cl_item);
	HASH_FIND(hh, class->items, &cl_item->value.ref.class_idx, sizeof(cl_item->value.ref.class_idx), cl_str);
	fprintf(stream, "Super class: %s\n", cl_str->value.string.value);

	fprintf(stream, "Interfaces count: %u\n", class->interfaces_count);

	if (class->interfaces_count > 0) {
		fprintf(stream, "Printing %u interfaces...\n", HASH_COUNT(class->interfaces));
		uint16_t id = 0;
		while (id < class->interfaces_count) {
			Ref *r;
			Item *item;
			Item *iface;
			HASH_FIND(hh, class->interfaces, &id, sizeof(id), r);
			HASH_FIND(hh, class->items, &r->class_idx, sizeof(r->class_idx), item);
			HASH_FIND(hh, class->items, &item->value.ref.class_idx, sizeof(item->value.ref.class_idx), iface);
			fprintf(stream, "Interface: %s\n", iface->value.string.value);
			id++;
		}
	}

	fprintf(stream, "Printing %u methods...\n", HASH_COUNT(class->methods));
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
