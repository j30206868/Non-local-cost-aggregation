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
	SPGroup *root;
	if(g1->connCount >= g2->connCount){
		root = g1->getRootGroup();
		g2->getRootGroup()->next = root;
	}else{
		root = g2->getRootGroup();
		g1->getRootGroup()->next = root;
	}
	root->connCount++;
}

SGNode **SGNode::createSGNodeList(int w, int h){
	SGNode **nodeList = new SGNode*[w];
	for(int i=0 ; i < w ; i++){
		nodeList[i] = new SGNode[h];
	}
	return nodeList;
}
//
//	Debug�ϥ� (�i�Ngraph�L�X)
//
void SGNode::showSGNodeList(SGNode **nodeList, int w, int h){
	/********************** �L�Xnode��pixel value �� edge ��weight ****************************
		�b�L���a�観�Iconfusing, �ҥH�b�o�仡���@�U
		�ѩ��ڤW�ݤ@�ӯx�} 1  2  3 ���ɭ�
							 4  5  6
							 7  8  9
		�H�G���}�C[x][y]�s����, x�������O�ѤW��U([x][0]={1 4 7}), y�����O�ѥ���k([0][y]={1 2 3})
		���O�o�˸�u��@�ɪ��y�Щw�q�O�A�˪�
		�M�Ӧb�g�{�����ɭԤ��Q�ި���h
		�ҥH�ڭ̤����ˤ����D, 
		�b���ݭn��ı�Ʈ�, ����٬O�� x->��@���k y->��@�W�U
		���O�ѩ�function�O�n��ı�ƪ��L�X��
		�ҥH�b y�b���ɭ� �L�����O left �� right ��edge
		       x�b���ɭ�         �O top  �� down 
	***************************************************************/
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
			//printf("(%3d) ", nodeList[x][y].grayPxl);
			printf("(%3.0f) ", nodeList[x][y].agtCost);
		}

		printf("\n");
	}
}
void SGNode::showSGNodeListEdgeAndGroup(SGNode **nodeList, int w, int h){
	/********************** �L�Xnode��group id �� edge ��weight ****************************/
	for(int x=0; x<w ; x++){
		if(x > 0){
			printf("     ");
			for(int y = 0 ; y < h ; y++){
				if(nodeList[x][y].edges[ SGNode::SGEDGE_LEFT ] != NULL)
					printf("-%3.0f-             ", nodeList[x][y].edges[ SGNode::SGEDGE_LEFT ]->w );
				else
					printf("                  ");
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
			printf("(%1d,%1d) ", x, y);
			//printf("(%3d) ", nodeList[x][y].deg);
			//printf("(%3d) ", nodeList[x][y].group->connCount);
			//printf("(%3d) ", nodeList[x][y].group->getId());

			if(y < h){
				if(nodeList[x][y].edges[ SGNode::SGEDGE_DOWN ] != NULL)
					printf("-%3.0f- ", nodeList[x][y].edges[ SGNode::SGEDGE_DOWN ]->w );
				else
					printf("      ");
			}
		}

		if(x < w){
			printf("\n");
			printf("     ");
			for(int y = 0 ; y < h ; y++){
				if(nodeList[x][y].edges[ SGNode::SGEDGE_RIGHT ] != NULL)
					printf("-%3.0f-             ", nodeList[x][y].edges[ SGNode::SGEDGE_RIGHT ]->w );
				else
					printf("                  ");
			}
		}

		printf("\n");
	}
}
//
void SGNode::setPixel(uint32_t pixel){
	this->grayPxl = pixel;
}
void SGNode::addNewEdge(SGEdge *newEdge, int direction){
	if(this->edges[ direction ] == NULL)
		this->deg++;
	this->edges[ direction ] = newEdge;
}
void SGNode::assignSPGRootGroup(SPGroup *newGroup){
	this->group->next = newGroup;
}
int SGNode::getOppositeDirectionFlag(int direction){
		switch (direction)
		{
			case SGEDGE_UP:
				return SGEDGE_DOWN;
			case SGEDGE_DOWN:
				return SGEDGE_UP;
			case SGEDGE_LEFT:
				return SGEDGE_RIGHT;
			case SGEDGE_RIGHT:
				return SGEDGE_LEFT;
			default:
				return -1;
		}
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
	for(int i = 0 ; i < len ; i++){
		costList[i] = NULL;
	}
	return costList;
}
void SGECostNode::freeEmptySGECostList(SGECostNode **costList, int len){
	free(costList);
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

/*********************************************
	  Kruskal's Algorithm Implementation
*********************************************/
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
inline void connectEdgeBtwTwoSGNode(SGNode &node1, SGNode &node2, int edgeType, int cost){
	SGEdge *newEdge = SGEdge::createNewSGEdge(cost);

	if( edgeType == SGNode::EDGETYPE_HORIZONTAL){      // node1 �� node2
		node1.addNewEdge(newEdge, SGNode::SGEDGE_RIGHT);
		node2.addNewEdge(newEdge, SGNode::SGEDGE_LEFT);
	}else{                                        		  
		node1.addNewEdge(newEdge, SGNode::SGEDGE_DOWN);// node1
													   //   �U
		node2.addNewEdge(newEdge, SGNode::SGEDGE_UP);  // node2
	}
}
bool connectEdgeOfTwoSGNode(SGNode &node1, SGNode &node2, int edgeType, int cost){
	SPGroup *g1 = node1.group;
	SPGroup *g2 = node2.group;

	int flag = 0; //0->both are null, 1->g1 not null, 2->both not null, 3->g1 is null but g2 is not 
	if(g1 != NULL){
		flag++;
	}
	if(g2 != NULL){
		if(flag == 0)//g1 is null
			flag = 3;
		else{
			flag = 2;
		}
	}

	if(flag == 2){//both are in groups
		if( g1->getId() != g2->getId() ){
			connectEdgeBtwTwoSGNode(node1, node2, edgeType, cost);
			SPGroup::combineTwoSPGroup(g1, g2);
		}else{//can't be connected, it will cause a cycle in the graph
			return false;
		}
	}else if(flag == 3){//g1 = null
		node1.group = g2->getRootGroup();
		connectEdgeBtwTwoSGNode(node1, node2, edgeType, cost);
	}else if(flag == 1){//g2 = null
		node2.group = g1->getRootGroup();
		connectEdgeBtwTwoSGNode(node1, node2, edgeType, cost);
	}else{//both null
		SPGroup *newGroup = SPGroup::createNewSPGroup();
		node1.group = newGroup;
		node2.group = newGroup;
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
SGNode *getRootOfMST(SGNode **nodeList, int w, int h){
	//�ѩ�Ȯ��٬ݤ��Xroot���P�|������t�O(�t�׸򵲪G���G�������v��), �]�������Ϥ������@�I�@��root
	return &nodeList[w/2][h/2];
}
void buildKruskalMST(SGNode **nodeList, int **mat, uint32_t w, uint32_t h){
	//�ϥ�8bit�Ƕ����� intensityLimit �N�O 256 ��
	SGECostNode **costList = SGECostNode::initSGECostList( IntensityLimit );

	for(int x = 0 ; x < w ; x++){
		for(int y = 0 ; y < h ; y++){
			nodeList[x][y].setPixel( mat[x][y] );

			if( x > 0 ){//�B�z����edge
				int x1 = x - 1;
				SGECostNode *newNode = SGECostNode::creatSGECostNode(x1, y, x, y);
				int diff = abs(mat[x1][y] - mat[x][y]);
				appendToSGECostList(costList, newNode, diff);
			}

			if( y > 0 ){//�B�z�W��edge
				int y1 = y - 1;
				SGECostNode *newNode = SGECostNode::creatSGECostNode(x, y1, x, y);
				int diff = abs(mat[x][y1] - mat[x][y]);
				appendToSGECostList(costList, newNode, diff);
			}
		}
	}

	SGECostNode::showSGECostList(costList, IntensityLimit);
	addMSTEdgeToNodeList(nodeList, costList, IntensityLimit, w, h);
	SGECostNode::freeEmptySGECostList(costList, IntensityLimit);
}

/*******************************************************
		Non local cost aggregation Implementation
*******************************************************/

CostAggregator::CostAggregator(SGNode **nList){
	this->nodeList = nList;
}
void CostAggregator::upwardAggregation(int x, int y, int parentDirection){
	//�����U�n�D�Ҧ��l�`�I��s��agtCost
	this->nodeList[x][y].agtCost  = this->nodeList[x][y].grayPxl;
	for(int i=0 ; i<SGNode::EDGELIMIT ; i++){
		if(i != parentDirection && this->nodeList[x][y].edges[i] != NULL){
			// debug
			if(UPAGT_TraceEachStep){
				printf("Cost Aggragating �y��[%2d][%2d] ", x, y);
				if(i == SGNode::SGEDGE_UP)
					printf("Go UP   ");
				else if(i == SGNode::SGEDGE_DOWN)
					printf("Go DOWN ");
				else if(i == SGNode::SGEDGE_LEFT)
					printf("Go LEFT ");
				else if(i == SGNode::SGEDGE_RIGHT)
					printf("Go RIGHT");
			}
			//

			int rx, ry;
			this->getPointedXY(x, y, rx, ry, i);

			if(UPAGT_TraceEachStep){// debug
				printf("-> [%2d][%2d]\n", rx, ry);
				system("PAUSE");
			}
			//

			upwardAggregation(rx, ry, SGNode::getOppositeDirectionFlag(i));
			this->nodeList[x][y].agtCost += this->nodeList[rx][ry].agtCost;
		}
	}
}
void CostAggregator::downwardAggregation(int x, int y, int parentDirection){

}
void CostAggregator::getPointedXY(int x, int y, int &resultX, int &resultY, int direction){
	switch (direction)
		{
			case SGNode::SGEDGE_UP:
				resultX = x;
				resultY = y-1;
				break;
			case SGNode::SGEDGE_DOWN:
				resultX = x;
				resultY = y+1;
				break;
			case SGNode::SGEDGE_LEFT:
				resultX = x-1;
				resultY = y;
				break;
			case SGNode::SGEDGE_RIGHT:
				resultX = x+1;
				resultY = y;
				break;
			default:
				break;
		}
}
