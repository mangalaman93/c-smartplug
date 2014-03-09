C-Smartplug
=========

Distributed Event Based System 2014 Grand Challenge

How to run
==========
### Query 1
	make
	./bin/broker_q1 40 priv/temp.csv 5457 (Terminal 1)
	sh script/house.sh 0 39 127.0.0.1 5457 (Terminal 2)

Number of houses = 40 (id from 0 to 39)

### Query 2
	make
	./bin/broker 5668 priv/temp.csv (Terminal 1)
	./bin/query2 127.0.0.1 5668 (Terminal 2)

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

Contributors
============
* Aman Mangal
* Arun Mathew
* Tanmay Randhavane
