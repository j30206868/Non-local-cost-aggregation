#include "StdGraph.h"

//initialize group count
uint32_t SPGroup::groupC = 0;

SPGroup *SPGroup::createNewSPGroup(){
	return new SPGroup();
}

int SPGroup::getId(){
	if(this->next == NULL)
		return this->id;
	else{
		return getRootGroup()->id;
	}
}

SPGroup *SPGroup::getRootGroup(){
	SPGroup *g = this;
	while(g->next != NULL){
		g = g->next;
	}
	return g;
}

inline void SPGroup::combineTwoSPGroup(SPGroup *g1, SPGroup *g2){
	if(g1->connCount >= g2->connCount)
		g2->getRootGroup()->next = g1->getRootGroup();
	else
		g1->getRootGroup()->next = g2->getRootGroup();
}

SGNode **SGNode::createSGNodeList(int w, int h){
	SGNode **nodeList = new SGNode*[w];
	for(int i=0 ; i < w ; i++){
		nodeList[i] = new SGNode[h];
	}
	return nodeList;
}

void SGNode::showSGNodeListEdgeAndGroup(SGNode **nodeList, int w, int h){
	for(int x=0; x<w ; x++){
		if(x > 0){
			printf("     ");
			for(int y = 0 ; y < h ; y++){
				if(nodeList[x][y].edges[ SGNode::SGEDGE_LEFT ] != NULL)
					printf("-%3.0f-       ", nodeList[x][y].edges[ SGNode::SGEDGE_LEFT ]->w );
				else
					printf("            ");
			}
			printf("\n");
		}

		printf("[%2d] ", x);
		for(int y=0 ; y<h ; y++){
			if(y > 0){
				if(nodeList[x][y].edges[ SGNode::SGEDGE_UP ] != NULL)
					printf("-%3.0f- ", nodeList[x][y].edges[ SGNode::SGEDGE_UP ]->w );
				else
					printf("      ");
			}
			printf("(%3d) ", nodeList[x][y].group->getId());
		}

		printf("\n");
	}
}

void SGNode::showSGNodeList(SGNode **nodeList, int w, int h){
	for(int x=0; x<w ; x++){
		if(x > 0){
			printf("     ");
			for(int y = 0 ; y < h ; y++){
				if(nodeList[x][y].edges[ SGNode::SGEDGE_LEFT ] != NULL)
					printf("-%3.0f-       ", nodeList[x][y].edges[ SGNode::SGEDGE_LEFT ]->w );
				else
					printf("            ");
			}
			printf("\n");
		}

		printf("[%2d] ", x);
		for(int y=0 ; y<h ; y++){
			if(y > 0){
				if(nodeList[x][y].edges[ SGNode::SGEDGE_UP ] != NULL)
					printf("-%3.0f- ", nodeList[x][y].edges[ SGNode::SGEDGE_UP ]->w );
				else
					printf("      ");
			}
			printf("(%3d) ", nodeList[x][y].grayPxl);
		}

		printf("\n");
	}
}

void SGNode::setPixel(uint32_t pixel){
	this->grayPxl = pixel;
}

void SGNode::addNewEdge(SGEdge *newEdge, int direction){
	if(this->edges[ direction ] == NULL)
		this->deg++;
	this->edges[ direction ] = newEdge;
}

void SGNode::assignSPGroup(SPGroup *newGroup){
	this->group = newGroup;
}

SGEdge *SGEdge::createNewSGEdge(int cost){
	SGEdge * newEdge = new SGEdge();
	newEdge->w = cost;
	return newEdge;
}

SGECostNode *SGECostNode::creatSGECostNode(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
	SGECostNode *newNode = new SGECostNode();
	newNode->x1 = x1;
	newNode->y1 = y1;
	newNode->x2 = x2;
	newNode->y2 = y2;
	return newNode;
}

SGECostNode **SGECostNode::initSGECostList(int len){
	SGECostNode **costList = new SGECostNode*[len];
	for(int i = 0 ; i < IntensityLimit ; i++){
		costList[i] = NULL;
	}
	return costList;
}

void SGECostNode::showSGECostList(SGECostNode **costList, int len){
	for(int i = 0 ; i < len ; i++){
		if( costList[i] != NULL ){
			printf("CostList[%3d] = ", i);
			SGECostNode *lastNode = costList[i];
			int count = 1;
			do
			{
				printf("%3d(%2d,%2d ~ %2d,%2d) ", count, lastNode->x1, lastNode->y1, lastNode->x2, lastNode->y2);
				count++;
				lastNode = lastNode->next;
			}while(lastNode != NULL);
			printf("\n");
		}
	}
}

void appendToSGECostList(SGECostNode **costList, SGECostNode *newNode, uint32_t idx){
	if( costList[idx] == NULL ){//head is NULL
		costList[idx] = newNode;
	}else{
		SGECostNode *lastNode = costList[idx];
		while( lastNode->next != NULL ){
			lastNode = lastNode->next;
		}
		lastNode->next = newNode;
	}
}

void buildKruskalMST(SGNode **nodeList, int **mat, uint32_t w, uint32_t h){
	//使用8bit灰階的話 intensityLimit 就是 256 色
	SGECostNode **costList = SGECostNode::initSGECostList( IntensityLimit );

	for(int x = 0 ; x < w ; x++){
		for(int y = 0 ; y < h ; y++){
			nodeList[x][y].setPixel( mat[x][y] );

			if( x > 0 ){//處理左方edge
				int x1 = x - 1;
				SGECostNode *newNode = SGECostNode::creatSGECostNode(x1, y, x, y);
				int diff = abs(mat[x1][y] - mat[x][y]);
				appendToSGECostList(costList, newNode, diff);
			}

			if( y > 0 ){//處理上方edge
				int y1 = y - 1;
				SGECostNode *newNode = SGECostNode::creatSGECostNode(x, y1, x, y);
				int diff = abs(mat[x][y1] - mat[x][y]);
				appendToSGECostList(costList, newNode, diff);
			}
		}
	}

	SGECostNode::showSGECostList(costList, IntensityLimit);
	addMSTEdgeToNodeList(nodeList, costList, IntensityLimit, w, h);
}

inline void connectEdgeBtwTwoSGNode(SGNode &node1, SGNode &node2, int edgeType, int cost){
	SGEdge *newEdge = SGEdge::createNewSGEdge(cost);

	if( edgeType == SGNode::EDGETYPE_HORIZONTAL){      // node1 － node2
		node1.addNewEdge(newEdge, SGNode::SGEDGE_RIGHT);
		node2.addNewEdge(newEdge, SGNode::SGEDGE_LEFT);
	}else{                                        		  
		node1.addNewEdge(newEdge, SGNode::SGEDGE_DOWN);// node1
													   //   ｜
		node2.addNewEdge(newEdge, SGNode::SGEDGE_UP);  // node2
	}
}

inline bool connectEdgeOfTwoSGNode(SGNode &node1, SGNode &node2, int edgeType, int cost){
	SPGroup *g1 = node1.group;
	SPGroup *g2 = node2.group;

	int g1ID = g1->getId();
	int g2ID = g2->getId();
	printf("g1[%2d] g2[%2d]\n",g1ID, g2ID);

	int flag = 0; //0->both are null, 1->g1 not null, 2->both not null, 3->g1 is null but g2 is not 
	if(g1ID != 0){
		flag++;
	}
	if(g2ID != 0){
		if(flag == 0)//g1 is null
			flag = 3;
		else{
			flag = 2;
		}
	}

	if(flag == 2){//both are in groups
		if( g1ID != g2ID ){
			connectEdgeBtwTwoSGNode(node1, node2, edgeType, cost);
			SPGroup::combineTwoSPGroup(g1, g2);
		}else{//can't be connected, it will cause a cycle in the graph
			return false;
		}
	}else if(flag == 3){//g1 = null
		//node1.assignSPGroup(g2);
		node1.group->next = g2;
		connectEdgeBtwTwoSGNode(node1, node2, edgeType, cost);
	}else if(flag == 1){//g2 = null
		//node2.assignSPGroup(g1);
		node2.group->next = g1;
		connectEdgeBtwTwoSGNode(node1, node2, edgeType, cost);
	}else{//both null
		//SPGroup *newGroup = SPGroup::createNewSPGroup();
		//node1.assignSPGroup(newGroup);
		//node2.assignSPGroup(newGroup);
		SPGroup::groupC++;
		node1.group->id = SPGroup::groupC;
		node2.group->id = SPGroup::groupC;
		node1.group->next = node2.group;
		connectEdgeBtwTwoSGNode(node1, node2, edgeType, cost);
	}
	return true;
}

void addMSTEdgeToNodeList(SGNode **nodeList, SGECostNode **costList, int costListLen, int w, int h){
	for(int i=0 ; i<costListLen ; i++){
		SGECostNode *costNode = costList[i];
		SGECostNode *nextNode = NULL;
		while(costNode != NULL){
			SGNode node1 = nodeList[costNode->x1][costNode->y1];
			SGNode node2 = nodeList[costNode->x2][costNode->y2];

			int edgeType;
			if(costNode->x1 == costNode->x2)
				edgeType = SGNode::EDGETYPE_VERTICLE;
			else
				edgeType = SGNode::EDGETYPE_HORIZONTAL;

			bool isConnected = connectEdgeOfTwoSGNode(nodeList[costNode->x1][costNode->y1], nodeList[costNode->x2][costNode->y2], edgeType, i);

			//debug
			//SGNode::showSGNodeListEdgeAndGroup(nodeList, w, h);
			//system("PAUSE");

			//free this node after insertion
			nextNode = costNode->next;
			free(costNode);
			costNode = nextNode;
		}
	}
}