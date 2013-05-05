#include "class.h"
#include <endian.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Class *read_class_from_file_name(char *file_name) {
	FILE *file = fopen(file_name, "r");
	if (!file) {
		fprintf(stderr, "Could not open '%s': %s\n", file_name, strerror(errno));
		return NULL;
	}
	// Check the file header for .class nature
	if (!is_class(file)) {
		fprintf(stderr, "Skipping '%s': not a valid class file\n", file_name);
		return NULL;
	}
	const ClassFile cf = {file_name, file};
	return read_class(cf);
}

Class *read_class(const ClassFile class_file) {
	Class *class = (Class *) malloc(sizeof(Class));

	fread(&class->minor_version, sizeof(uint16_t), 1, class_file.file);
	fread(&class->major_version, sizeof(uint16_t), 1, class_file.file);
	fread(&class->const_pool_count, sizeof(uint16_t), 1, class_file.file);
	
	// convert the big endian ints to host equivalents
	class->minor_version = be16toh(class->minor_version);
	class->major_version = be16toh(class->major_version);
	class->const_pool_count = be16toh(class->const_pool_count);

	class->file_name = class_file.file_name;
	class->pool_size_bytes = parse_const_pool(class, class->const_pool_count, class_file);
	
	if (class->pool_size_bytes == 0) {
		return NULL;
	}

	fread(&class->flags, sizeof(class->flags), 1, class_file.file);
	class->flags = be16toh(class->flags);

	fread(&class->this_class, sizeof(class->this_class), 1, class_file.file);
	class->this_class = be16toh(class->this_class);

	fread(&class->super_class, sizeof(class->super_class), 1, class_file.file);
	class->super_class = be16toh(class->super_class);

	fread(&class->interfaces_count, sizeof(class->interfaces_count), 1, class_file.file);
	class->interfaces_count = be16toh(class->interfaces_count);

	class->interfaces = calloc(class->interfaces_count, sizeof(Ref));
	int idx = 0;
	while (idx < class->interfaces_count) {
		fread(&class->interfaces[idx].class_idx, sizeof(class->interfaces[idx].class_idx), 1, class_file.file);
		class->interfaces[idx].class_idx = be16toh(class->interfaces[idx].class_idx);
		idx++;
	}

	fread(&class->fields_count, sizeof(class->fields_count), 1, class_file.file);
	class->fields_count = be16toh(class->fields_count);

	class->fields = calloc(class->fields_count, sizeof(Field));
	Field *f;
	idx = 0;
	while (idx < class->fields_count) {
		f = class->fields + idx;
		fread(&f->flags, sizeof(u2), 1, class_file.file);
		fread(&f->name_idx, sizeof(u2), 1, class_file.file);
		fread(&f->desc_idx, sizeof(u2), 1, class_file.file);
		fread(&f->attrs_count, sizeof(u2), 1, class_file.file);
		f->name_idx = be16toh(f->name_idx);
		f->desc_idx = be16toh(f->desc_idx);
		f->attrs_count = be16toh(f->attrs_count);
		f->attrs = calloc(f->attrs_count, sizeof(Attribute));

		Attribute *attr;
		int aidx = 0;
		while (aidx++ < f->attrs_count) {
			attr = f->attrs + aidx;
			fread(&attr->name_idx, sizeof(u2), 1, class_file.file);
			fread(&attr->length, sizeof(u4), 1, class_file.file);
			attr->name_idx = be16toh(attr->name_idx);
			attr->length = be32toh(attr->length);
			attr->info = calloc(attr->length, sizeof(char));
			fread(attr->info, sizeof(char), attr->length, class_file.file);
		}
		idx++;
	}

	fread(&class->methods_count, sizeof(class->methods_count), 1, class_file.file);
	class->methods_count = be16toh(class->methods_count);

	class->methods = calloc(class->methods_count, sizeof(Method));
	idx = 0;
	while (idx < class->methods_count) {
		fread(&class->methods[idx].flags, sizeof(u2), 1, class_file.file);
		fread(&class->methods[idx].name_idx, sizeof(u2), 1, class_file.file);
		fread(&class->methods[idx].desc_idx, sizeof(u2), 1, class_file.file);
		fread(&class->methods[idx].attrs_count, sizeof(u2), 1, class_file.file);
		class->methods[idx].name_idx = be16toh(class->methods[idx].name_idx);
		class->methods[idx].desc_idx = be16toh(class->methods[idx].desc_idx);
		class->methods[idx].attrs_count = be16toh(class->methods[idx].attrs_count);
		//TODO read attributes!
		idx++;
	}

	fread(&class->attributes_count, sizeof(class->attributes_count), 1, class_file.file);
	class->attributes_count = be16toh(class->attributes_count);

	class->attributes = calloc(class->attributes_count, sizeof(Ref));
	idx = 0;
	while (idx < class->attributes_count) {
		//fread(&class->attributes[idx].class_idx, sizeof(class->attributes[idx].class_idx), 1, class_file.file);
		//class->attributes[idx].class_idx = be16toh(class->attributes[idx].class_idx);
		idx++;
	}
	return class;
}

uint32_t parse_const_pool(Class *class, const uint16_t const_pool_count, const ClassFile class_file) {
	const int MAX_ITEMS = const_pool_count - 1;
	uint32_t table_size_bytes = 0;
	int i;
	char tag_byte;
	Ref r;

	class->items = calloc(MAX_ITEMS, sizeof(Class));
	for (i = 1; i <= MAX_ITEMS; i++) {
		fread(&tag_byte, sizeof(char), 1, class_file.file);
		if (tag_byte < MIN_CPOOL_TAG || tag_byte > MAX_CPOOL_TAG) {
			fprintf(stderr, "Tag byte '%d' is outside permitted range %u to %u\n", tag_byte, MIN_CPOOL_TAG, MAX_CPOOL_TAG);
			table_size_bytes = 0;
			break; // fail fast
		}

		String s;
		uint16_t ptr_idx = i - 1;
		Item *item = class->items + ptr_idx;
		item->tag = tag_byte;

		// Populate item based on tag_byte
		switch (tag_byte) {
			case STRING_UTF8: // String prefixed by a uint16 indicating the number of bytes in the encoded string which immediately follows
				fread(&s.length, sizeof(s.length), 1, class_file.file);
				s.length = be16toh(s.length);
				s.value = malloc(sizeof(char) * s.length);
				fread(s.value, sizeof(char), s.length, class_file.file);
				item->value.string = s;
				table_size_bytes += 2 + s.length;
				break;
			case INTEGER: // Integer: a signed 32-bit two's complement number in big-endian format
				fread(&item->value.integer, sizeof(item->value.integer), 1, class_file.file);
				item->value.integer = be32toh(item->value.integer);
				table_size_bytes += 4;
				break;
			case FLOAT: // Float: a 32-bit single-precision IEEE 754 floating-point number
				fread(&item->value.flt, sizeof(item->value.flt), 1, class_file.file);
				item->value.flt = be32toh(item->value.flt);
				table_size_bytes += 4;
				break;
			case LONG: // Long: a signed 64-bit two's complement number in big-endian format (takes two slots in the constant pool table)
				fread(&item->value.lng.high, sizeof(item->value.lng.high), 1, class_file.file); // 4 bytes
				fread(&item->value.lng.low, sizeof(item->value.lng.low), 1, class_file.file); // 4 bytes
				item->value.lng.high = be32toh(item->value.lng.high);
				item->value.lng.low = be32toh(item->value.lng.low);
				// 8-byte consts take 2 pool entries
				++i;
				table_size_bytes += 8;
				break;
			case DOUBLE: // Double: a 64-bit double-precision IEEE 754 floating-point number (takes two slots in the constant pool table)
				fread(&item->value.dbl.high, sizeof(item->value.dbl.high), 1, class_file.file); // 4 bytes
				fread(&item->value.dbl.low, sizeof(item->value.dbl.low), 1, class_file.file); // 4 bytes
				item->value.dbl.high = be32toh(item->value.dbl.high);
				item->value.dbl.low = be32toh(item->value.dbl.low);
				// 8-byte consts take 2 pool entries
				++i;
				table_size_bytes += 8;
				break;
			case CLASS: // Class reference: an uint16 within the constant pool to a UTF-8 string containing the fully qualified class name
				fread(&r.class_idx, sizeof(r.class_idx), 1, class_file.file);
				r.class_idx = be16toh(r.class_idx);
				item->value.ref = r;
				table_size_bytes += 2;
				break;
			case STRING: // String reference: an uint16 within the constant pool to a UTF-8 string
				fread(&r.class_idx, sizeof(r.class_idx), 1, class_file.file);
				r.class_idx = be16toh(r.class_idx);
				item->value.ref = r;
				table_size_bytes += 2;
				break;
			case FIELD: // Field reference: two uint16 within the pool, 1st pointing to a Class reference, 2nd to a Name and Type descriptor
				/* FALL THROUGH TO METHOD */
			case METHOD: // Method reference: two uint16s within the pool, 1st pointing to a Class reference, 2nd to a Name and Type descriptor
				/* FALL THROUGH TO INTERFACE_METHOD */
			case INTERFACE_METHOD: // Interface method reference: 2 uint16 within the pool, 1st pointing to a Class reference, 2nd to a Name and Type descriptor
				fread(&r.class_idx, sizeof(r.class_idx), 1, class_file.file);
				fread(&r.name_idx, sizeof(r.name_idx), 1, class_file.file);
				r.class_idx = be16toh(r.class_idx);
				r.name_idx = be16toh(r.name_idx);
				item->value.ref = r;
				table_size_bytes += 4;
				break;
			case NAME: // Name and type descriptor: 2 uint16 to UTF-8 strings, 1st representing a name (identifier), 2nd a specially encoded type descriptor
				fread(&r.class_idx, sizeof(r.class_idx), 1, class_file.file);
				fread(&r.name_idx, sizeof(r.name_idx), 1, class_file.file);
				r.class_idx = be16toh(r.class_idx);
				r.name_idx = be16toh(r.name_idx);
				item->value.ref = r;
				table_size_bytes += 4;
				break;
			default:
				fprintf(stderr, "Found tag byte '%d' but don't know what to do with it\n", tag_byte);
				item = NULL;
				break;
		}
		if (item != NULL) class->items[i-1] = *item;
	}
	return table_size_bytes;
}

bool is_class(FILE *class_file) {
	uint32_t magicNum;
	size_t num_read = fread(&magicNum, sizeof(uint32_t), 1, class_file);
	return num_read == 1 && be32toh(magicNum) == 0xcafebabe;
}

Item *get_item(const Class *class, const uint16_t cp_idx) {
	if (cp_idx < class->const_pool_count) return &class->items[cp_idx-1];
	else return NULL;
}

Item *get_class_string(const Class *class, const uint16_t index) {
	Item *i1 = get_item(class, index);
	return get_item(class, i1->value.ref.class_idx);
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

char *field2str(const char fld_type) {
	switch(fld_type) {
		case 'B': 
			return "byte";
		case 'C': 
			return "char";
		case 'D': 
			return "double";
		case 'F': 
			return "float";
		case 'I': 
			return "int";
		case 'J': 
			return "long";
		case 'L': 
			return "reference";
		case 'S': 
			return "short";
		case 'Z': 
			return "boolean";
		case '[': 
			return "array";
		default:
			return "Undefined";
	}
}

