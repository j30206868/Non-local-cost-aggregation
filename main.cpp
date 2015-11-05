#include "StdGraph.h"

int const w = 3;
int const h = 3;

void testFunc(SGNode &node){
	node.group = new SPGroup();
	node.group->id = 1;
}

void main(){
	SGNode testNode;
	testFunc(testNode);
	printf("testNode group: %d\n", testNode.group->id);

	/*int mat[w][h] = {{1,  4, 8},  
					 {1, 11, 6}, 
					 {9,  5, 7} };  */
	SGNode **nodeList;
	
	int **mat = new int*[w]; 
	mat[0] = new int[h]; 
	mat[1] = new int[h];
	mat[2] = new int[h];
	mat[0][0] = 1;		mat[0][1] = 4;		mat[0][2] = 8;
	mat[1][0] = 1;		mat[1][1] = 11;		mat[1][2] = 6;
	mat[2][0] = 9;		mat[2][1] = 5;		mat[2][2] = 7;

	nodeList = SGNode::createSGNodeList(w, h);

	buildKruskalMST((SGNode **)nodeList, (int **)mat, w, h);
	 
	SGNode::showSGNodeList(nodeList, w, h);

	system("PAUSE");
}