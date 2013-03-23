#/bin/bash
clear; scons -c; scons && clear && ./cfr test/files/Test.class test/files/RabbitQueueClient.class test/files/RabbitQueueConsumer.class /dev/urandom test/files/LongTest.class test/files/DoubleTest.class test/files/Interfaces.class test/files/InvalidTagByte.class
