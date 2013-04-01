#!/bin/bash
./build.sh && valgrind --leak-check=full --track-origins=yes ./cfr test/files/Test.class > cfr_leak_opt.log
