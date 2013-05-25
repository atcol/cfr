#include "../src/class.h"
#include "../src/class.c"
#include "tap.h"
#include <stdio.h>
#include <string.h>

int main(void) {
	dbl();
	fields();
	empty();
	test_field2str();
	return exit_status();
}	

/* Print i and j before calling ok(i == j, msg); */
void iok(int i, int j, char *msg) {
	printf("%d == %d \n", i, j, msg);
	ok(i == j, msg);
}

/* Print write a comparison string into msg and call ok(0 == strcmp(str1, str2), msg); */
void strok(char *str1, char *str2, char *msg) {
	char *fmt_str = "%s - Comparison: '%s' == '%s'";
	int str_len = strlen(msg) + strlen(fmt_str) + strlen(str1) + strlen(str2) + 1;
	char *cmp_msg = malloc(sizeof(char) * str_len);
	snprintf(cmp_msg, str_len, fmt_str, msg, str1, str2);
	ok(0 == strcmp(str1, str2), cmp_msg);
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

	// Constant pool content tests; could probably make a recursive function but this way is explicit & simpler
	Item *i = get_item(c, 1);
	ok(17 == i->value.ref.name_idx, " 1 = Methodref			   6");
	ok(6 == i->value.ref.class_idx, " 1 = Methodref          17         //  java/lang/Object.\"<init>\":()V");

	i = get_item(c, 2);
	ok(18 == i->value.ref.class_idx, " 2 = Fieldref           18        //  java/lang/System.out:Ljava/io/PrintStream;");
	ok(19 == i->value.ref.name_idx, " 2 = Fieldref           19        //  java/lang/System.out:Ljava/io/PrintStream;");

	i = get_item(c, 3);
	ok(20 == i->value.ref.class_idx, " 3 = String             20            //  Hello world1.0");

	i = get_item(c, 4);
	ok(21 == i->value.ref.class_idx, " 4 = Methodref         21        //  java/io/PrintStream.println:(Ljava/lang/String;)V");
	ok(22 == i->value.ref.name_idx, " 4 = Methodref          22        //  java/io/PrintStream.println:(Ljava/lang/String;)V");

	i = get_item(c, 5);
	iok(23, i->value.ref.class_idx, " 5 = Class              23            //  DoubleTest");

	i = get_item(c, 6);
	iok(24, i->value.ref.class_idx, " 6 = Class              24            //  java/lang/Object");

	i = get_item(c, 7);
	ok(0 == strcmp("d", i->value.string.value), " 7 = Utf8               d");

	i = get_item(c, 8);
	ok(0 == strcmp("D", i->value.string.value), " 8 = Utf8               D");

	i = get_item(c, 9);
	ok(0 == strcmp("ConstantValue", i->value.string.value), " 9 = Utf8               ConstantValue");

	i = get_item(c, 10);
	ok(1.0 == to_double(i->value.dbl), " 10 = Double             1.0d");

	i = get_item(c, 12);
	strok("<init>", i->value.string.value, " 12 = Utf8               <init>");

	i = get_item(c, 13);
	strok("()V", i->value.string.value, " 13 = Utf8               ()V");

	i = get_item(c, 14);
	strok("Code", i->value.string.value, " 14 = Utf8               Code");
	//ok(, " 15 = Utf8               main");
	//ok(, " 16 = Utf8               ([Ljava/lang/String;)V");
	//ok(, " 17 = NameAndType        12:13        //  \"<init>\":()V");
	//ok(, " 18 = Class              25            //  java/lang/System");
	//ok(, " 19 = NameAndType        26:27        //  out:Ljava/io/PrintStream;");
	//ok(, " 20 = Utf8               Hello world1.0");
	//ok(, " 21 = Class              28            //  java/io/PrintStream");
	//ok(, " 22 = NameAndType        29:30        //  println:(Ljava/lang/String;)V");
	//ok(, " 23 = Utf8               DoubleTest");
	//ok(, " 24 = Utf8               java/lang/Object");
	//ok(, " 25 = Utf8               java/lang/System");
	//ok(, " 26 = Utf8               out");
	//ok(, " 27 = Utf8               Ljava/io/PrintStream;");
	//ok(, " 28 = Utf8               java/io/PrintStream");
	//ok(, " 29 = Utf8               println");
	//ok(, " 30 = Utf8               (Ljava/lang/String;)V");

	const Method *method = c->methods;
	const Item *m1name = get_item(c, c->methods[0].name_idx);
	const Item *m2name = get_item(c, c->methods[1].name_idx);
	ok(c->methods_count == 2, "Methods count == 2, main() and constructor");
	ok(0 == strcmp("<init>", m1name->value.string.value), "First method's name is <init>");
	ok(0 == strcmp("main", m2name->value.string.value), "Second method's name is main");
	ok(1 == method->attrs_count, "init method attribute count is 1");

	// Test that attribute 1 (index 0) has the name "Code"
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
	ok(10 == c->const_pool_count, "Constant pool count is 10");
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
