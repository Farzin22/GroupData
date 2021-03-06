#include "K2.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifndef MAX_INT
#define MAX_INT pow(2,32)
#endif

using namespace std;

K2::K2(int numOfNodes, vector<string> nodes, vector< vector<int> > dataset, int maxParents)
{
	vector< vector<int> > probabilities;
	this->numOfNodes = numOfNodes;
	this->dataset = dataset;
	this->maxParents = maxParents;
	this->probabilities = probabilities;
	this->nodes = nodes;
	this->network = Network(numOfNodes, dataset, nodes);
}

// Relative Posterior Probability function; see Section 8.1 in 
// A Tutorial on Learning with Bayesian Networks, Heckerman 1995

float K2::scoreNetwork(Network n)
{
	float totalScore = 0.0f;
	for (int i = 0; i < numOfNodes; i++) {
		//super K2 scoring function
		totalScore += this->scoringFunction(i, n.getParents(i));
	}
	return totalScore;
	
	/**
	long max = RAND_MAX;
	float random = (float) rand()/ (max+1);	// random number from [0,1]
	return random;
	*/
}

float K2::scoringFunction(int i, vector<int> parents)
{	
	/**
	// Print parents of node i
	cout << "Node: X_" << i << ", parents: ";
	for (unsigned parent = 0; parent < parents.size(); parent ++) {
		cout << "X_" << parents[parent] << " ";
	}
	cout << "" << endl;
	*/
	
	// Here we assume nodes can only take on two values, 0 and 1
	float ri = 2.0f;
	int psize = parents.size();
	float qi = (float) pow(2, psize); //min(psize, maxParents)); 	// the possible values the list of parents can take
	if (qi == 1) {
		qi = 0;
	}
	
	// Products
	float a;				// Products of a_ijk! from k=1 to k=r_i
	float b = 1.0f;				// Products of a * (r_i - 1)!/(N_ij + r_i - 1)! from j=1 to j=q_i
	float nj; 				// Number of cases where x_i's parents = j
	float aijk;	
	
	// Product loops
	if (qi == 0) {
		nj = 0.0f;
		a = 1.0f;
		
		for (int k = 0; k < ri; k++) {
			aijk = this->parseCases(i, -1, k, parents);
			a *= this->factorial(aijk);
			nj += aijk;
		}
		b *= (float) this->factorial(ri - 1) / this->factorial(nj + ri - 1);
		b *= a;
	}
	
	for (int j = 0; j < qi; j++) {
		nj = 0.0f;
		a = 1.0f;
		
		for (int k = 0; k < ri; k++) {
			aijk = this->parseCases(i, j, k, parents);			
			a *= this->factorial(aijk);
			nj += aijk;
		}
		b *= (float) this->factorial(ri - 1) / this->factorial(nj + ri - 1);
		b *= a;
		
	}
	return b;
}

// Find the number of cases in dataset given i, j, k in the form above
int K2::parseCases(int i, int j, int k, vector<int> parents)
{
	int jcopy = j;
	int tup[numOfNodes];
	memset( tup, 0, numOfNodes*sizeof(int) );
	int count = 0;
	int psize = parents.size();
	for (int t = 0; t < psize; t++) {
		tup[t] = 0;
	}
	
	while (jcopy != 0 && jcopy != -1 && count <= psize) {
		
		if (abs(jcopy/2.0f - jcopy/2) > 0.0f) {
			tup[psize - count- 1] = 1;
		}
		else {
			tup[psize - count - 1] = 0;
		}
		
		count++;
		jcopy = (int) jcopy/2;
		
	}
	
	vector<int> dummy;
	for (int n = 0; n < numOfNodes; n++) {
		int v = valueExists(n, parents);
			
		if (n==i) {
			dummy.push_back(k);
		}
		else if (v != -1) {
			dummy.push_back(tup[v]);
		}
		else {
			dummy.push_back(-1);
		}
	}
	return this->findCases(dummy);
}

// Finds number of cases of searchVal, with wildcard = -1
int K2::findCases(vector<int> searchVal)
{
	int cases = 0;
	for (unsigned m = 0; m < dataset.size(); m++) {		
		cases++;
		for (unsigned n = 0; n < searchVal.size(); n++) {
				if (dataset[m][n] != searchVal[n] && searchVal[n] != -1) {
					cases -= 1;
					break;
				}
			}
	}
	return cases;
}

// Non-recursive factorial function
int K2::factorial(int integer)
{
	int fact = 1;
	for (int i = 1; i <= integer; i++) {
		fact = fact * i;
	}
	return fact;
}

// Checks to see if value is in array
int K2::valueExists(int value, vector<int> array)
{
	for (int i = 0; i < (int) array.size(); i++) {
		if (array[i] == value) {
			return i;
		}
	}
	return -1;
}

void K2::findParents() 
{
	cout << " " << endl;
	cout << "K2 algorithm result:" << endl;
	
	float totalNetworkScore = 0.0f;
	for (int i = 0; i < numOfNodes; i++) {
		vector<int> parents;
		int currentNode = i;
		float score = scoringFunction(currentNode, parents);
		bool OK = true;
		vector<int> predecessors;
		while ((OK) && ((int) parents.size() < maxParents)) {
			int begin = 0;
			while (begin < (i))
			{
				predecessors.push_back(begin);
				begin++;
			}
			Nodepair pair = findMaxNode(currentNode, predecessors, parents);
			int newNode = pair.maxNode;
			float newScore = pair.maxScore;
			if (newScore >= score) {
				parents.push_back(newNode);
				score = newScore;
			}
			else {
				OK = false;
			}
			predecessors.clear();
		}
		for (unsigned j=0; j<parents.size(); j++) {
			network.setParent(i, parents[j]);
			cout << "Node: X_" << i << ", Parent: X_" << parents[j] << endl;
		}
	}
	
	totalNetworkScore = this->scoreNetwork(network);
	cout << "Score of network: " << totalNetworkScore << endl;
	
}

Nodepair K2::findMaxNode(int node, vector<int> predecessors, vector<int> parents)
{
	Nodepair np;
   	float maxScore = -1;
   	int maxNode = -1;
   	int newNode = -1;
	for (unsigned j = 0; j < predecessors.size(); j++) {
		if (this->valueExists(predecessors[j], parents) < 0) {
			newNode = predecessors[j];
			vector<int> newParents = parents;
			newParents.push_back(newNode); 
			float newScore = scoringFunction(node, newParents);
			
			if (newScore > maxScore) {
				maxScore = newScore;
				maxNode = newNode;
			}
		}
	}
	np.maxScore = maxScore;
	np.maxNode = newNode;
	return np;
}
