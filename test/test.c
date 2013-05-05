#include "../src/class.h"
#include "../src/class.c"
#include "tap.h"
#include <stdio.h>
#include <string.h>

//
	// Always assert filename;
	// 	const pool size 1 greater than number in pool
	// 	interface count matches
	// 	field count matches
	//	method count matches
	

int main(void) {
	dbl();
	fields();
	return exit_status();
}


void dbl() {
	char *double_file = "./files/DoubleTest.class";
	Class *c = read_class_from_file_name(double_file);

	ok(c != NULL, "C is not NULL");
	ok((strcmp(c->file_name, double_file) == 0), "File name matches");
	ok(0 == c->minor_version, "Major version is 0 (1.7.0_10)");
	ok(51 == c->major_version, "Major version is 51 (1.7)");
	ok(1 == c->fields_count, "Fields count = 1");
	free(c);
}

void fields() {
	printf("Fields test\n");
	Class *c = read_class_from_file_name("files/Fields.class");
	ok(c->fields_count == 7, "Fields count == 7");
}
