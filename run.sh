#!/bin/bash

# Loop through a range of testcases
for TESTCASE in {1..4}; do
    echo "====================== Running testcase $TESTCASE ======================"
    make run$TESTCASE
    # echo "====================== Finished testcase $TESTCASE ======================"
done

for TESTCASE in {1..4}; do
    echo "====================== Verifying testcase $TESTCASE ======================"
    make verify$TESTCASE
    # echo "====================== Finished testcase $TESTCASE ======================"
done