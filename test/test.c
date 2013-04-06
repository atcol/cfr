#include "../src/class.h"
#include "../src/class.c"
#include "tap.h"
#include <stdio.h>
#include <string.h>

int main(void) {
	plan_tests(1);

	char *double_file = "files/DoubleTest.class";
	Class *c = read_class_from_file_name(double_file);

	ok((strcmp(c->file_name, double_file) == 0), "testing", NULL);
	ok(0 == c->minor_version, "Major version is 0 (1.7.0_10)", NULL);
	ok(51 == c->major_version, "Major version is 51 (1.7)", NULL);
	// Always assert filename;
	// 	const pool size 1 greater than number in pool
	//
	//
	
	free((void *) c);

	c = read_class_from_file_name("files/Fields.class");
	ok(c->fields_count == 7, "Fields count == 7", NULL);

	return exit_status();
}
