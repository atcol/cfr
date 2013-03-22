#/bin/bash
clear; scons && clear && ./cfr test/Test.class test/RabbitQueueClient.class test/RabbitQueueConsumer.class /dev/urandom test/LongTest.class test/DoubleTest.class
