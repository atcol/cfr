#include "../src/class.h"
#include "../src/class.c"
#include "tap.h"
#include <stdio.h>
#include <string.h>

// Always assert filename
// const pool size 1 greater than number in pool
// interface count matches
// field count matches
// method count matches
	

int main(void) {
	dbl();
	fields();
	empty();
	test_field2str();
	return exit_status();
}

void dbl() {
	printh("Double");
	char *double_file = "./files/DoubleTest.class";
	Class *c = read_class_from_file_name(double_file);

	ok(c != NULL, "C is not NULL");
	ok((strcmp(c->file_name, double_file) == 0), "File name matches");
	ok(0 == c->minor_version, "Major version is 0 (1.7.0_10)");
	ok(51 == c->major_version, "Major version is 51 (1.7)");
	ok(1 == c->fields_count, "Fields count = 1");
	ok(31 == c->const_pool_count, "Constant pool count is 31"); // two for double type, plus # of items is 1 less than this member's value
	ok(0 == c->attributes_count, "Attributes count = 0");

	const Item *desc = get_item(c, c->fields[0].desc_idx);
	ok(desc != NULL, "Field descriptor Item is in the constant pool");
	ok('D' == desc->value.string.value[0], "Field type tag is D");

	ok(1 == c->fields[0].attrs_count, "Attribute count for field 0 is 1");

	const Item *attr_name = get_item(c, c->fields[0].attrs[0].name_idx);
	ok(strcmp("ConstantValue", attr_name->value.string.value) == 0, "First attribute in first field has name ConstantValue");

	const Method *method = c->methods;
	const Item *m1name = get_item(c, c->methods[0].name_idx);
	const Item *m2name = get_item(c, c->methods[1].name_idx);
	ok(c->methods_count == 2, "Methods count == 2, main() and constructor");
	ok(0 == strcmp("<init>", m1name->value.string.value), "First method's name is <init>");
	ok(0 == strcmp("main", m2name->value.string.value), "Second method's name is main");
	ok(1 == method->attrs_count, "init method attribute count is 1");

	// Test that attribute #1 (index 0) has the name "Code"
	const Item *m1attr1 = get_item(c, method->attrs[0].name_idx);
	ok(0 == strcmp("Code", m1attr1->value.string.value), "Attribute #1 of method #1 has name Code");
	ok(1 == c->methods[1].attrs_count, "main method attribute count is 1");
	free(c);
}

void empty() {
	printh("Empty");
	char *empty = "./files/Empty.class";
	Class *c = read_class_from_file_name(empty);

	ok(c != NULL, "C is not NULL");
	ok(0 == (strcmp(c->file_name, empty)), "File name matches");
	ok(0 == c->minor_version, "Major version is 0 (1.7.0_10)");
	ok(51 == c->major_version, "Major version is 51 (1.7)");
	ok(0 == c->fields_count, "Fields count = 0");
	ok(10 == c->const_pool_count, "Constant pool count is 29");
	ok(0 == c->attributes_count, "Attributes count = 0");

	free(c);
}

void fields() {
	printh("Fields");
	Class *c = read_class_from_file_name("files/Fields.class");
	ok(c != NULL, "C is not NULL");
	ok(c->fields_count == 7, "Fields count == 7");
	ok(c->methods_count == 2, "Methods count == 2, main() and constructor");
}

void test_field2str() {
	printh("field2str");
	ok("byte" == field2str('B'), "B == byte");
	ok("char" == field2str('C'), "C == char");
	ok("double" == field2str('D'), "D == double");
	ok("float" == field2str('F'), "F == float");
	ok("int" == field2str('I'), "I == int");
	ok("long" == field2str('J'), "J == long");
	ok("reference" == field2str('L'), "L == reference");
	ok("short" == field2str('S'), "S == short");
	ok("boolean" == field2str('Z'), "Z == boolean");
	ok("array" == field2str('['), "[ == array");
}

/* Print a pretty test header so we can distinguish results */
void printh(const char *test_name) {
	printf("#####################\n");
	printf("Test: %s\n", test_name);
	printf("#####################\n");
}
