# cfr

Class File Reader - a Java class file parser written in C.

### Introduction

This program reads Java class files and reports on the class and its structure.

### Why?

For fun.

### Build

Run the `scons` command to build the source and produce a binary called "cfr". You can use `build.sh` which will clean and then compile the cfr binary.

### Testing
This project uses libtap for its unit testing.

The test suite is in the `test` directory, along with numerous test class files in `test/files/`. 

To run the suite, change to the `test` directory and execute `scons -c; scons && ./cfr-tests` to run the suite.

### Usage

`./cfr .class [.class ..]`

### License

Please read the LICENSE file.
