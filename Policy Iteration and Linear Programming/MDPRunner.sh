#!/bin/bash

# Howards pi

mdpdir=$1

mkdir HPIResults
for i in {1..100}
do
   cmd="./planner.sh --mdp mdpdir/MDP$i.txt --algorithm hpi"
   $cmd > "HPIResults/sol$i.txt"
   echo cmd
done


# Random pi
mkdir RPIResults
for i in {1..100}
do
   cmd="./planner.sh --mdp mdpdir/MDP$i.txt --algorithm rpi --randomseed $i"
   $cmd > "RPIResults/sol$i.txt"
done


# Batch Switching pi
for j in 2 4 6 8 10 12 14 16 18 20
do
	mkdir BSPIResults_"$j"
	for i in {1..100}
	do
	   cmd="./planner.sh --mdp mdpdir/MDP$i.txt --algorithm bspi --batchsize $j"
	   $cmd > "BSPIResults_50/sol$i.txt"
	done
done

python getAvgValues.py