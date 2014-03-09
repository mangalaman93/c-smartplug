C-Smartplug
=========

Distributed Event Based System 2014 Grand Challenge


Run Custom Test (Query 2)
=========================
### Test 0: Unit Tests
	make test

### Test 1: Accuracy test
	mkdir bin
	g++ -Iinclude/ -std=c++0x -lm test/slidingmc_exp.cpp src/mc.cpp src/slidingmc.cpp -o bin/slidingmc_exp
	./bin/slidingmc_exp

### Test 2: Binary Seach Test
	mkdir bin
	g++ test/binarysearch.cpp -o bin/binarysearch
	./bin/slidingmc_exp

### Test 3: Max Bin Test
	mkdir bin
	g++ -Iinclude/ -std=c++0x -lm test/slidingmc_binsize.cpp src/mc.cpp src/slidingmc.cpp -o bin/slidingmc_binsize
	./bin/slidingmc_binsize
