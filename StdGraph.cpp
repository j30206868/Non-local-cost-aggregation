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
//	Debug使用 (可將graph印出)
//
void SGNode::showSGNodeList(SGNode **nodeList, int w, int h){
	/********************** 印出node的pixel value 跟 edge 的weight ****************************
		在印的地方有點confusing, 所以在這邊說明一下
		由於實際上看一個矩陣 1  2  3 的時候
							 4  5  6
							 7  8  9
		以二維陣列[x][y]存取時, x指的其實是由上到下([x][0]={1 4 7}), y指的是由左到右([0][y]={1 2 3})
		但是這樣跟真實世界的座標定義是顛倒的
		然而在寫程式的時候不想管那麼多
		所以我們仍假裝不知道, 
		在不需要視覺化時, 單純還是把 x->當作左右 y->當作上下
		但是由於此function是要視覺化的印出來
		所以在 y軸的時候 印的其實是 left 跟 right 的edge
		       x軸的時候         是 top  跟 down 
	***************************************************************/
	for(int x=0; x<w ; x++){
		if(x > 0){
			printf("     ");
			for(int y = 0 ; y < h ; y++){
				if(nodeList[x][y].edges[ SGNode::SGEDGE_LEFT ] != NULL)
					printf("-%.3f-       ", nodeList[x][y].edges[ SGNode::SGEDGE_LEFT ]->w );
				else
					printf("            ");
			}
			printf("\n");
		}

		printf("[%2d] ", x);
		for(int y=0 ; y<h ; y++){
			if(y > 0){
				if(nodeList[x][y].edges[ SGNode::SGEDGE_UP ] != NULL)
					printf("-%.3f- ", nodeList[x][y].edges[ SGNode::SGEDGE_UP ]->w );
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
	/********************** 印出node的group id 跟 edge 的weight ****************************/
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
void appendToSGECostList(SGECostNode **costList, SGECostNode **tailList, SGECostNode *newNode, uint32_t idx){
	if( costList[idx] == NULL ){//head is NULL
		costList[idx] = newNode;
		tailList[idx] = newNode;
	}else{
		tailList[idx]->next = newNode;
		tailList[idx] = tailList[idx]->next;
	}
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
bool connectEdgeOfTwoSGNode(SGNode &node1, SGNode &node2, int edgeType, int cost){
	SPGroup *g1 = node1.group;
	SPGroup *g2 = node2.group;

	int flag = 0; //0->both are null, 1->g1 not null, 2->both not null, 3->g1 is null but g2 is not 
	if(g1 != NULL){
		flag++;
	}
	if(g2 != NULL){
		if(flag == 0){//g1 is null
			node1.group = g2->getRootGroup();
			connectEdgeBtwTwoSGNode(node1, node2, edgeType, cost);
			return true;
		}else{//both are in groups
			if( g1->getId() != g2->getId() ){
				connectEdgeBtwTwoSGNode(node1, node2, edgeType, cost);
				SPGroup::combineTwoSPGroup(g1, g2);
				return true;
			}else{//can't be connected, it will cause a cycle in the graph
				return false;
			}
		}
	}

	if(flag == 1){//g2 = null
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
			int edgeType;
			if(costNode->x1 == costNode->x2)
				edgeType = SGNode::EDGETYPE_VERTICLE;
			else
				edgeType = SGNode::EDGETYPE_HORIZONTAL;

			connectEdgeOfTwoSGNode(nodeList[costNode->x1][costNode->y1], nodeList[costNode->x2][costNode->y2], edgeType, i);

			//debug
			//SGNode::showSGNodeListEdgeAndGroup(nodeList, w, h);
			//system("PAUSE");

			//free this node after insertion
			nextNode = costNode->next;
			//free(costNode);
			costNode = nextNode;
		}
	}
}
SGNode *getRootOfMST(SGNode **nodeList, int w, int h){
	//由於暫時還看不出root不同會有什麼差別(速度跟結果似乎都不受影像), 因此直接圖中間的一點作為root
	return &nodeList[w/2][h/2];
}
//使用8bit灰階的話 intensityLimit 就是 256 色
SGECostNode **costList = SGECostNode::initSGECostList( IntensityLimit );
SGECostNode **costTailList = SGECostNode::initSGECostList( IntensityLimit );
SGECostNode ** buildCostList(SGNode **nodeList, int **mat, uint32_t w, uint32_t h){
	for(int x = 0 ; x < w ; x++){
		for(int y = 0 ; y < h ; y++){
			nodeList[x][y].setPixel( mat[x][y] );

			if( x > 0 ){//處理左方edge
				int x1 = x - 1;
				SGECostNode *newNode = SGECostNode::creatSGECostNode(x1, y, x, y);
				int diff = abs(mat[x1][y] - mat[x][y]);
				appendToSGECostList(costList, costTailList, newNode, diff);
			}

			if( y > 0 ){//處理上方edge
				int y1 = y - 1;
				SGECostNode *newNode = SGECostNode::creatSGECostNode(x, y1, x, y);
				int diff = abs(mat[x][y1] - mat[x][y]);
				appendToSGECostList(costList, costTailList, newNode, diff);
			}
		}
	}

	return costList;

	//SGECostNode::showSGECostList(costList, IntensityLimit);
	//addMSTEdgeToNodeList(nodeList, costList, IntensityLimit, w, h);
	//SGECostNode::freeEmptySGECostList(costList, IntensityLimit);
	//SGECostNode::freeEmptySGECostList(costTailList, IntensityLimit);
}

/*******************************************************
		Non local cost aggregation Implementation
*******************************************************/
CostAggregator::CostAggregator(SGNode **nList){
	this->nodeList = nList;
}
void CostAggregator::upwardAggregation(int x, int y, int parentDirection){
	//先往下要求所有子節點更新其agtCost
	this->nodeList[x][y].agtCost  = this->nodeList[x][y].grayPxl;
	for(int i=0 ; i<SGNode::EDGELIMIT ; i++){
		if(i != parentDirection && this->nodeList[x][y].edges[i] != NULL){
			// debug
			/*if(UPAGT_TraceEachStep){
				printf("Cost Aggragating 座標[%2d][%2d] ", x, y);
				if(i == SGNode::SGEDGE_UP)
					printf("Go UP   ");
				else if(i == SGNode::SGEDGE_DOWN)
					printf("Go DOWN ");
				else if(i == SGNode::SGEDGE_LEFT)
					printf("Go LEFT ");
				else if(i == SGNode::SGEDGE_RIGHT)
					printf("Go RIGHT");
			}*/
			//

			int rx, ry;
			this->getPointedXY(x, y, rx, ry, i);

			/*if(UPAGT_TraceEachStep){// debug
				printf("-> [%2d][%2d]\n", rx, ry);
				system("PAUSE");
			}*/
			//
			//更新與子節點之間的weight
			this->nodeList[x][y].edges[i]->w = exp(this->nodeList[x][y].edges[i]->w * -1 / sigma);
			//
			upwardAggregation(rx, ry, SGNode::getOppositeDirectionFlag(i));
			this->nodeList[x][y].agtCost += this->nodeList[rx][ry].agtCost * this->nodeList[x][y].edges[i]->w;
		}
	}
}
void CostAggregator::downwardAggregation(int x, int y, int parentDirection){
	//往下要求所有子節點更新其agtCost
	if(parentDirection != -1){
		float weight = this->nodeList[x][y].edges[parentDirection]->w;
		int px, py;
		this->getPointedXY(x, y, px, py, parentDirection);
		this->nodeList[x][y].agtCost = weight                         *
									   this->nodeList[px][py].agtCost +
									   (1 - weight*weight)            *
									   this->nodeList[x][y].agtCost;	   
	}
	for(int i=0 ; i<SGNode::EDGELIMIT ; i++){
		if(i != parentDirection && this->nodeList[x][y].edges[i] != NULL){
			// debug
			/*if(DOWNAGT_TraceEachStep){
				printf("Cost Aggragating 座標[%2d][%2d] ", x, y);
				if(i == SGNode::SGEDGE_UP)
					printf("Go UP   ");
				else if(i == SGNode::SGEDGE_DOWN)
					printf("Go DOWN ");
				else if(i == SGNode::SGEDGE_LEFT)
					printf("Go LEFT ");
				else if(i == SGNode::SGEDGE_RIGHT)
					printf("Go RIGHT");
			}*/
			//

			int rx, ry;
			this->getPointedXY(x, y, rx, ry, i);

			/*if(DOWNAGT_TraceEachStep){// debug
				printf("-> [%2d][%2d]\n", rx, ry);
				system("PAUSE");
			}*/
			//
			downwardAggregation(rx, ry, SGNode::getOppositeDirectionFlag(i));
		}
	}
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


/**********************************************************
					combined function
**********************************************************/
SGNode **getNonLocalCostAgtResult(std::string fname){
	SGNode **nodeList;

	cv::Mat image;
    image = cv::imread(fname, CV_LOAD_IMAGE_GRAYSCALE);   // Read the file

    if(! image.data )                              // Check for invalid input
    {
        std::cout <<  "Could not open or find the image" << std::endl ;
        return NULL;
    }

	/*int w = 

	int **mat = new int*[w]; 
	for(int i=0 ; i<w ; i++){
		mat[i] = new int[h];
		//mat[i] = image.row(j);
		for(int j=0; j<h ; j++){
			uchar& uxy = image.at<uchar>(j, i);
			mat[i][j] = (int)uxy;
		}
	}*/
	return NULL;
}