all: clean
	g++ -std=c++17 -O3 -Wall -Wextra main.cpp -o legalizer

1: all
	./legalizer ./input/testcase1/architecture.txt ./input/testcase1/instance.txt ./input/testcase1/netlist.txt ./output/output1.txt

2: all
	./legalizer ./input/testcase2/architecture.txt ./input/testcase2/instance.txt ./input/testcase2/netlist.txt ./output/output2.txt

3: all
	./legalizer ./input/testcase3/architecture.txt ./input/testcase3/instance.txt ./input/testcase3/netlist.txt ./output/output3.txt

4: all
	./legalizer ./input/testcase4/architecture.txt ./input/testcase4/instance.txt ./input/testcase4/netlist.txt ./output/output4.txt

runall: 1 2 3 4

clean:
	rm -f legalizer