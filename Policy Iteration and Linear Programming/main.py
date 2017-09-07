import sys
import numpy as np
import pulp
import copy
import random

# printing the output
def printOutput(policy, value):
	for a_v in range(0, len(policy)):
		print str(value[a_v]) + "\t" + str(policy[a_v])

# Getting optimal policy
def getOptimalPolicy(R, T, value, gamma):
	policy = [0 for s in range(len(value))]
	for a_state in range(len(value)):
		v0 = 0
		v1 = 0
		for s in range(len(value)):
			v0 += T[a_state][0][s]*R[a_state][0][s] + gamma*T[a_state][0][s]*V[s]
			v1 += T[a_state][1][s]*R[a_state][0][s] + gamma*T[a_state][0][s]*V[s]
		if(value[a_state] == v0):
			policy[a_state] = 0
		else:
			policy[a_state] = 1
	return policy

# Expected value of Immediate Reward
def getTStarR(T, R, state, policy, n):
	rwd = 0
	for a_state in range(0, n):
		rwd += T[state][policy[state]][a_state]*R[state][policy[state]][a_state]
	return rwd

# Policy Evaluation
def getValueVector(T, R, policy, gamma):
	n = len(policy)
	a = np.zeros(n*n).reshape(n, n)
	b = np.zeros(n)

	for a_state in range(0, n):
		b[a_state] = getTStarR(T, R, a_state, policy, n)

	for i in range(0, n):
		for j in range(0, n):
			if(i == j):
				a[i][j] = 1-gamma*T[i][policy[i]][j]
			else:
				a[i][j] = (-1)*gamma*T[i][policy[i]][j]

	return np.linalg.solve(a,b)

# Changing actions for improvable states
def toggle(i):
	if(i == 1):
		return 0
	else: 
		return 1

# Getting Improvable States
def getImprovedPolicyVector(T, R, policy, gamma):
	pNewRet = copy.copy(policy)
	value = getValueVector(T, R, policy, gamma)
	for a_p in range(0, len(policy)):
		pNew = copy.copy(policy)
		pNew[a_p] = toggle(pNewRet[a_p])
		vNew = getValueVector(T, R, pNew, gamma)		
		if(vNew[a_p] > value[a_p]):
			pNewRet[a_p] = pNew[a_p]
		vNew = getValueVector(T, R, pNew, gamma)
		pNew = pNewRet
	return pNewRet


mdppath = sys.argv[1]
fp = open(mdppath, "r")

num_states = int(fp.readline())
num_actions = int(fp.readline())

T = [[[0 for s in range(num_states)] for a in range(num_actions)] for sprime in range(num_states)]
R = [[[0 for s in range(num_states)] for a in range(num_actions)] for sprime in range(num_states)]

for s in range(0, num_states):
    for a in range(0, num_actions):
    	tmp = fp.readline().strip().split("\t")
    	for sprime in range(0, len(tmp)):
    		R[s][a][sprime] = float(tmp[sprime])

for s in range(0, num_states):
    for a in range(0, num_actions):
    	tmp = fp.readline().strip().split("\t")
    	for sprime in range(0, len(tmp)):
    		T[s][a][sprime] = float(tmp[sprime])

gamma = float(fp.readline())
fp.close()

algorithm = sys.argv[2]

# Code for Linear Programming
if(algorithm == "lp"):
	model = pulp.LpProblem("Solving MDPs", pulp.LpMinimize)	
	V = pulp.LpVariable.dicts("V",range(num_states),None,None,cat='Continuous')
	model += pulp.lpSum([V[i] for i in range(0, num_states)])
	for a_state in range(0, num_states):
		for an_action in range(0, num_actions):
			model += (V[a_state] >= pulp.lpSum([T[a_state][an_action][i]*(R[a_state][an_action][i] + gamma*V[i]) for i in range(num_states)]))
	model.solve()

	value = [0 for s in range(num_states)]
	for a_state in V:
		value[a_state] = V[a_state].varValue

	printOutput(getOptimalPolicy(R, T, value, gamma), value)

# Code for Howard's Policy Iteration
elif(algorithm == "hpi"):
	policy = [0 for p in range(num_states)]
	# num_iter = 0
	while(True):
		# num_iter+=1
		pNew = getImprovedPolicyVector(T, R, policy, gamma)
		if(pNew == policy):
			break
		else:
			policy = pNew

	# fpd = open('hpi_iter.txt', "a")
	# fpd.write(str(num_iter)+"\n")
	# fpd.close()
	vPiStar = getValueVector(T, R, policy, gamma)
	printOutput(policy, vPiStar)


# Code for randomised policy iteration 
elif(algorithm == "rpi"): 
	randomseed = int(sys.argv[3])
	random.seed(randomseed)
	policy = [0 for p in range(num_states)]
	# num_iter = 0
	while(True):
		# num_iter+=1
		pNew = getImprovedPolicyVector(T, R, policy, gamma)
		if(pNew == policy):
			break
		else:
			for a_state in range(len(policy)):
				if(pNew[a_state] != policy[a_state]):
					rand = random.random()
					if(rand > 0.5):
						policy[a_state] = pNew[a_state]

	# fpd = open('rpi_iter.txt', "a")
	# fpd.write(str(num_iter)+"\n")
	# fpd.close()		
	vPiStar = getValueVector(T, R, policy, gamma)
	printOutput(policy, vPiStar)

# Code for batch switching policy iteration
elif(algorithm == "bspi"): 
	batchsize = int(sys.argv[3])
	policy = [0 for p in range(num_states)]
	# num_iter = 0
	while(True):
		# num_iter+=1
		pNew = getImprovedPolicyVector(T, R, policy, gamma)
		for a_state in range(num_states-1, -1, -1):
			if(pNew[a_state] != policy[a_state]):
				qNew = a_state/batchsize
				break
		if(pNew == policy):
			break
		else:
			for a_state in range(num_states-1, -1, -1):
				if(pNew[a_state] != policy[a_state] and a_state/batchsize == qNew):
					policy[a_state] = pNew[a_state]
	
	# fpd = open('bspi_iter.txt', "a")
	# fpd.write(str(num_iter)+"\n")
	# fpd.close()
	vPiStar = getValueVector(T, R, policy, gamma)
	printOutput(policy, vPiStar)

	
else:
	print("invalid algorithm option")