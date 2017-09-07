import sys
import copy
import random
from decimal import *
import numpy as np

def generate(S, A, R, T, gamma):
	print S
	print A

	for s in range(0, S):
	    for a in range(0, A):
	        for sprime in range(0, S):
	            print R[s][a][sprime], "\t",

	        print "\n",

	for s in range(0, S):
	    for a in range(0, A):
	        for sprime in range(0, S):
	            print T[s][a][sprime], "\t",

	        print "\n",

	print gamma

S = 50
A = 2

T = [[[0 for s in range(S)] for a in range(A)] for sprime in range(S)]
R = [[[0 for s in range(S)] for a in range(A)] for sprime in range(S)]


for s in range(0, S):
    for a in range(0, A):
    	count = 0
    	for sprime in range(0, S):
    		T[s][a][sprime] = random.random()
    		count += T[s][a][sprime]
    	for sprime in range(0, S):
			T[s][a][sprime] = round((T[s][a][sprime]/count), 6)

for s in range(0, S):
    for a in range(0, A):
    	for sprime in range(0, S):
    		R[s][a][sprime] = round((random.uniform(-1, 1)), 6)

gamma = round((random.uniform(0, 1)), 2)

generate(S, A, R, T, gamma)