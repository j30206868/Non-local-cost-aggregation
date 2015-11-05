#include "StdGraph.h"

int const w = 3;
int const h = 4;

void main(){
	SGNode **nodeList;
	
	int **mat = new int*[w]; 
	mat[0] = new int[h]; 
	mat[1] = new int[h];
	mat[2] = new int[h];
	mat[0][0] = 7;		mat[0][1] = 12;			mat[0][2] = 1;			mat[0][3] = 0;
	mat[1][0] = 70;		mat[1][1] = 10;			mat[1][2] = 255;		mat[1][3] = 254;
	mat[2][0] = 50;		mat[2][1] = 255;		mat[2][2] = 239;		mat[2][3] = 100;

	nodeList = SGNode::createSGNodeList(w, h);

	buildKruskalMST((SGNode **)nodeList, (int **)mat, w, h);
	 
	
	printf("\nGroup info:\n");
	SGNode::showSGNodeListEdgeAndGroup(nodeList, w, h);

	SGNode *root = getRootOfMST(nodeList, w, h);

	CostAggregator *ca = new CostAggregator(nodeList);
	ca->upwardAggregation(w/2, h/2, -1);

	printf("Aggregated Graph:\n");
	SGNode::showSGNodeList(nodeList, w, h);

	ca->downwardAggregation(w/2, h/2, -1);

	system("PAUSE");
}