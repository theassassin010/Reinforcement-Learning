#!/bin/bash
mkdir generatedMDPS
for i in {1..100}
do
   cmd="python MDPGenerator.py"
   $cmd > "generatedMDPS/MDP$i.txt"
done