#include <stdio.h>
#include "tap.h"
#include "../src/class.h"

int main(void) {
	plan_tests(1);

	ClassFile *c = read_class_from_file_name("files/DoubleTest.class");

	ok(1 == 1, "testing");
	return exit_status();
}
