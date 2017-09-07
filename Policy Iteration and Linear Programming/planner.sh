#!/bin/bash

if [ "$1" == "--mdp" ]; then
	mdppath=$2
elif [ "$1" == "--algorithm" ]; then
	algorithm=$2
elif [ "$1" == "--batchsize" ]; then
	batchsize=$2
elif [ "$1" == "--randomseed" ]; then
	randomseed=$2
fi

if [ "$3" == "--mdp" ]; then
	mdppath=$4
elif [ "$3" == "--algorithm" ]; then
	algorithm=$4
elif [ "$3" == "--batchsize" ]; then
	batchsize=$4
elif [ "$3" == "--randomseed" ]; then
	randomseed=$4
fi

if [ "$5" == "--mdp" ]; then
	mdppath=$6
elif [ "$5" == "--algorithm" ]; then
	algorithm=$6
elif [ "$5" == "--batchsize" ]; then
	batchsize=$6
elif [ "$5" == "--randomseed" ]; then
	randomseed=$6
fi


if [ "$7" == "--mdp" ]; then
	mdppath=$8
elif [ "$7" == "--algorithm" ]; then
	algorithm=$8
elif [ "$7" == "--batchsize" ]; then
	batchsize=$8
elif [ "$7" == "--randomseed" ]; then
	randomseed=$8
fi

if [ "$algorithm" == lp ]; then
	cmd="python main.py $mdppath $algorithm"
elif [ "$algorithm" == hpi ]; then
	cmd="python main.py $mdppath $algorithm"
elif [ "$algorithm" == rpi ]; then
	cmd="python main.py $mdppath $algorithm $randomseed"
elif [ "$algorithm" == bspi ]; then
	cmd="python main.py $mdppath $algorithm $batchsize"
else
	echo "Unrecognised algorithm option"
fi

$cmd