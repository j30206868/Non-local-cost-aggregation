#include "cwz_non_local_cost.h"

uint32_t SPGroup::groupC = 0;
float CostAggregator::max_gradient_cost;
float CostAggregator::max_color_cost;
float CostAggregator::color_ratio;
float CostAggregator::gradient_ratio;
const float CostAggregator::sigma = 0.1;

/*******************************************************
		Non local cost aggregation Implementation
*******************************************************/
CostAggregator::CostAggregator(SGNode **nList, int _w, int _h){
	this->nodeList = nList;
	this->w = _w;
	this->h = _h;
	this->MSTWeight = 0;
	for(int i=0 ; i<IntensityLimit ; i++){
		wHistogram[i] = exp(-double(i) / (CostAggregator::sigma * (IntensityLimit - 1)));
	}

	CostAggregator::max_gradient_cost = 2.0;
	CostAggregator::color_ratio = 0.11;
	CostAggregator::gradient_ratio = 1 - CostAggregator::color_ratio;
	//CostAggregator::max_color_cost = 7.0;
	CostAggregator::max_color_cost = 21.0;//由於 color cost不除3 比較快 所以使用21
	CostAggregator::color_ratio = 0.036667;//這邊提前除三, 使cost除3的部分可以被省略
	
}
void CostAggregator::upwardAggregation(int x, int y, int parentDirection){
	//先往下要求所有子節點更新其agtCost

	for(int i=0 ; i<SGNode::EDGELIMIT ; i++){
		if(i != parentDirection && this->nodeList[y][x].edges[i] != NULL){
			int rx, ry;
			SGNode::getPointedXY(x, y, rx, ry, i);
			//
			//更新與子節點之間的weight
			this->MSTWeight += this->nodeList[y][x].edges[i]->w ;

			upwardAggregation(rx, ry, SGNode::getOppositeDirectionFlag(i));

			for(int d = 0 ; d < disparityLevel ; d++)
				this->nodeList[y][x].agtCost[d] += ( this->nodeList[ry][rx].agtCost[d] * wHistogram[ this->nodeList[y][x].edges[i]->w ] );
		}
	}
}
void CostAggregator::downwardAggregation(int x, int y, int parentDirection){
	//往下要求所有子節點更新其agtCost
	if(parentDirection != -1){
		float weight = wHistogram[ this->nodeList[y][x].edges[parentDirection]->w ];
		int px, py;
		SGNode::getPointedXY(x, y, px, py, parentDirection);
		float weightSq = weight * weight;
		for(int d = 0 ; d < disparityLevel ; d++){
			this->nodeList[y][x].agtCost[d] = weight                            *
										      this->nodeList[py][px].agtCost[d] +
										      (1.0 - weightSq)                  *
										      this->nodeList[y][x].agtCost[d];	
		}
	}
	for(int i=0 ; i<SGNode::EDGELIMIT ; i++){
		if(i != parentDirection && this->nodeList[y][x].edges[i] != NULL){
			int rx, ry;
			SGNode::getPointedXY(x, y, rx, ry, i);
			downwardAggregation(rx, ry, SGNode::getOppositeDirectionFlag(i));
		}
	}
}
void CostAggregator::compute_gradient(float**gradient, uchar **gray_image)
{
	float gray,gray_minus,gray_plus;
	for(int y=0;y<this->h;y++)
	{
		gray_minus=gray_image[y][0];
		gray=gray_plus=gray_image[y][1];
		gradient[y][0]=gray_plus-gray_minus+127.5;

		for(int x=1;x<this->w-1;x++)
		{
			gray_plus=gray_image[y][x+1];
			gradient[y][x]=0.5*(gray_plus-gray_minus)+127.5;

			gray_minus=gray;
			gray=gray_plus;
		}
		
		gradient[y][this->w-1]=gray_plus-gray_minus+127.5;
	}
}
void CostAggregator::compute_match_cost(uchar *** left, uchar *** right, uchar ** left_gray, uchar ** right_gray)
{
	double gradient_t, match_cost_t;

	time_t start = clock();
	float **left_gradient = new_2d_arr<float>(this->h, this->w);
	compute_gradient(left_gradient, left_gray);
	float **right_gradient = new_2d_arr<float>(this->h, this->w);
	compute_gradient(right_gradient, right_gray);
	gradient_t = double(clock() - start)/ CLOCKS_PER_SEC;

	start = clock(); 
	for(int d=0 ; d<disparityLevel ; d++)
	for(int y=0 ; y<this->h ; y++)
	for(int x=0 ; x<this->w-d ; x++)
	{
			//color cost
		int rx = x+d;
		float color_cost = abs(left[y][x][0] - right[y][rx][0]);
		color_cost      += abs(left[y][x][1] - right[y][rx][1]);
		color_cost      += abs(left[y][x][2] - right[y][rx][2]);
		//color_cost      = std::min(color_cost/(float)3, max_color_cost);
		color_cost      = std::min(color_cost, max_color_cost);
		//gradient cost
		float gradient_cost = std::min(abs(left_gradient[y][x] - right_gradient[y][rx]), max_gradient_cost);

		this->nodeList[y][x].agtCost[d] = color_cost*color_ratio + gradient_cost*gradient_ratio;
	}
	match_cost_t = double(clock() - start)/ CLOCKS_PER_SEC;

	printf("compute_match_cost() gradient_t  : %f\n", gradient_t);
	printf("compute_match_cost() match_cost_t: %f\n", match_cost_t);

	free_2d_arr<float>(left_gradient, this->h, this->w);
	free_2d_arr<float>(right_gradient, this->h, this->w);
}
void CostAggregator::pickBestDepthForNodeList(){
	for(int y=0 ; y<h ; y++) for(int x=0 ; x<w ; x++)
	{
		nodeList[y][x].dispairty = 0;
		for(int d=1 ; d<disparityLevel ; d++){
			if(nodeList[y][x].agtCost[d] < nodeList[y][x].agtCost[ nodeList[y][x].dispairty ]){
				nodeList[y][x].dispairty = d;
			}
		}
	}
}
void CostAggregator::showDisparityMap(){
	cv::Mat dMap(h, w, CV_8U);
	for(int y=0 ; y<h ; y++) for(int x=0 ; x<w ; x++)
	{
		dMap.at<uchar>(y,x) = nodeList[y][x].dispairty * (double) IntensityLimit / (double)disparityLevel;
	}
	cv::imshow("dMap", dMap);
	cvWaitKey(0);
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
			SPGroup *g1Root = g1->getRootGroup();
			SPGroup *g2Root = g2->getRootGroup();
			if( g1Root->getId() != g2Root->getId() ){
				connectEdgeBtwTwoSGNode(node1, node2, edgeType, cost);
				SPGroup::combineTwoSPGroup(g1Root, g2Root);
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
			if(costNode->x1 == costNode->x2)//動y is verticle
				edgeType = SGNode::EDGETYPE_VERTICLE;
			else
				edgeType = SGNode::EDGETYPE_HORIZONTAL;

			connectEdgeOfTwoSGNode(nodeList[costNode->y1][costNode->x1], nodeList[costNode->y2][costNode->x2], edgeType, i);

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
SGECostNode ** buildCostListFromCV(SGNode **nodeList, uchar **left_gray, uint32_t w, uint32_t h){
	SGECostNode **costList = SGECostNode::initSGECostList( IntensityLimit );
	SGECostNode **costTailList = SGECostNode::initSGECostList( IntensityLimit );

	for(int y = 0 ; y < h ; y++){
		for(int x = 0 ; x < w ; x++){
			if( y > 0 ){//處理上方edge
				int y1 = y - 1;
				SGECostNode *newNode = SGECostNode::creatSGECostNode(x, y1, x, y);
				int diff = abs(left_gray[y1][x] - left_gray[y][x]);
				appendToSGECostList(costList, costTailList, newNode, diff);
			}

			if( x > 0 ){//處理左方edge
				int x1 = x - 1;
				SGECostNode *newNode = SGECostNode::creatSGECostNode(x1, y, x, y);
				int diff = abs(left_gray[y][x1] - left_gray[y][x]);
				appendToSGECostList(costList, costTailList, newNode, diff);
			}
			//printf("\n");
		}
	}

	return costList;
}


void SGNode::addNewEdge(SGEdge *newEdge, int direction){
	if(this->edges[ direction ] == NULL)
		this->deg++;
	this->edges[ direction ] = newEdge;
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
SGNode **SGNode::createSGNodeList(int w, int h){
	SGNode **nodeList = new SGNode*[h];
	for(int i=0 ; i < h ; i++){
		nodeList[i] = new SGNode[w];
	}
	return nodeList;
}
void SGNode::getPointedXY(int x, int y, int &resultX, int &resultY, int direction){
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

SPGroup *SPGroup::createNewSPGroup(){
	return new SPGroup();
}
int SPGroup::getId(){
	return getRootGroup()->id;
}
SPGroup *SPGroup::getRootGroup(){
	SPGroup *g = this;
	while(g->next != NULL){
		g = g->next;
	}
	return g;
}
inline void SPGroup::combineTwoSPGroup(SPGroup *g1Root, SPGroup *g2Root){
	SPGroup *root;
	if(g1Root->connCount >= g2Root->connCount){
		root = g1Root;
		g2Root->next = root;
	}else{
		root = g2Root;
		g1Root->next = root;
	}
	root->connCount++;
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
