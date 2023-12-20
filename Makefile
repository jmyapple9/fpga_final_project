EXEC = ./legalizer
VRFY = ./verifier2

INPUT_DIR = ./input
OUTPUT_DIR = ./output

all: clean
	g++ -std=c++17 -O3 -Wall -Wextra main.cpp -o legalizer

# Define the run target
run:
	@echo "Usage: make runA, where A is the testcase number"

verify:
	@echo "Usage: make verifyA, where A is the testcase number"

runall:
	@echo "Usage: make runallA, where A is the testcase number"

run%: all
	$(EXEC) $(INPUT_DIR)/testcase$*/architecture.txt $(INPUT_DIR)/testcase$*/instance.txt $(INPUT_DIR)/testcase$*/netlist.txt $(OUTPUT_DIR)/output$*.txt

verify%:
	$(VRFY) $(INPUT_DIR)/testcase$*/architecture.txt $(INPUT_DIR)/testcase$*/instance.txt $(INPUT_DIR)/testcase$*/netlist.txt $(OUTPUT_DIR)/output$*.txt

runall%: all
	$(EXEC) $(INPUT_DIR)/testcase$*/architecture.txt $(INPUT_DIR)/testcase$*/instance.txt $(INPUT_DIR)/testcase$*/netlist.txt $(OUTPUT_DIR)/output$*.txt
	$(VRFY) $(INPUT_DIR)/testcase$*/architecture.txt $(INPUT_DIR)/testcase$*/instance.txt $(INPUT_DIR)/testcase$*/netlist.txt $(OUTPUT_DIR)/output$*.txt

clean:
	rm -f legalizer

# Phony target to prevent conflicts with files named 'run'
.PHONY: run verify runall