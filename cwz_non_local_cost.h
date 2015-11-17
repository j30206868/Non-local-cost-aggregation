#ifndef CWZ_NON_LOCAL_COST_H
#define CWZ_NON_LOCAL_COST_H

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>

//for write file
#include <fstream>
#include <iomanip>
#include <sstream>

//for combined functions
#include <opencv2\opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include "common_func.h"

#define IntensityLimit 256
#define disparityLevel 60

/******************************************************
					Data Structure
*******************************************************/

struct SPGroup{
	uint32_t id;
	SPGroup *next;
	uint32_t connCount; // connection counter

	SPGroup() : id(0), next(NULL), connCount(0) {
		SPGroup::groupC++;
		this->id = SPGroup::groupC;
		this->next = NULL;
	};
	int getId();
	SPGroup *getRootGroup();

	static uint32_t groupC;
	
	static SPGroup *createNewSPGroup();
	static inline void combineTwoSPGroup(SPGroup *g1, SPGroup *g2);
};
struct SGEdge{
	uchar w;
	static SGEdge *createNewSGEdge(int cost);
};
struct SGECostNode{
	uint16_t x1;
	uint16_t y1;
	uint16_t x2;
	uint16_t y2;
	SGECostNode *next;

	SGECostNode() : next(NULL){};

	static SGECostNode *creatSGECostNode(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
	static SGECostNode **initSGECostList(int len);
	static void freeEmptySGECostList(SGECostNode **costList, int len);
	static void showSGECostList(SGECostNode **costList, int len);
};
struct SGNode{
	SGEdge **edges;
	uint32_t deg; //edge amount
	SPGroup *group;
	int dispairty; //best disparity value can be 0 ~ max disparity(not the cost)
	float *agtCost;

	static int const EDGELIMIT = 4;
	//SGNode() : edges(NULL), deg(0), grayPxl(0), agtCost(0), group(NULL){
	SGNode() : edges(NULL), deg(0), agtCost(NULL), group(NULL){
		this->edges = new SGEdge*[EDGELIMIT];
		this->edges[SGEDGE_UP]	  = NULL;
		this->edges[SGEDGE_LEFT]  = NULL;
		this->edges[SGEDGE_DOWN]  = NULL;
		this->edges[SGEDGE_RIGHT] = NULL;
		this->agtCost             = new float[disparityLevel];
		for(int i=0 ; i<disparityLevel ; i++)
			this->agtCost[i] = 2.55;
		//改成0 會變挺慘的
	};

	void addNewEdge(SGEdge *newEdge, int direction);

	//edge index(direction) flag
	static int const SGEDGE_UP    = 0;
	static int const SGEDGE_LEFT  = 1;
	static int const SGEDGE_DOWN  = 2;
	static int const SGEDGE_RIGHT = 3;

	static int getOppositeDirectionFlag(int direction);
	static void getPointedXY(int x, int y, int &resultX, int &resultY, int direction);

	//edge type
	static int const EDGETYPE_VERTICLE   = 0;
	static int const EDGETYPE_HORIZONTAL = 1;

	static SGNode **createSGNodeList(int w, int h);
};

/*********************************************
	  Kruskal's Algorithm Implementation
*********************************************/
void appendToSGECostList(SGECostNode **costList, SGECostNode **tailList, SGECostNode *newNode, uint32_t idx); //index 就是 edge 的 weight (也就是vertex pixel相減的差值)
void addMSTEdgeToNodeList(SGNode **nodeList, SGECostNode **costList, int costListLen, int w, int h);
bool connectEdgeOfTwoSGNode(SGNode &node1, SGNode &node2, int edgeType, int cost);
SGECostNode ** buildCostListFromCV(SGNode **nodeList, uchar **left_gray, uint32_t w, uint32_t h);

/*******************************************************
		Non local cost aggregation Implementation
*******************************************************/
class CostAggregator{
	
public:
	static float max_gradient_cost;
	static float max_color_cost;
	static float color_ratio;
	static float gradient_ratio;
	
	int MSTWeight;
	float wHistogram[IntensityLimit];

	CostAggregator(SGNode **nList, int _w, int _h);
	void upwardAggregation(int x, int y, int parentDirection);
	void downwardAggregation(int x, int y, int parentDirection);
	void compute_match_cost(uchar *** left, uchar *** right, uchar ** left_gray, uchar ** right_gray);
	void compute_gradient(float**gradient, uchar **gray_image);
	void pickBestDepthForNodeList();
	void showDisparityMap();
private:
	SGNode **nodeList;
	int w, h;

	static const float sigma;

	static bool const UPAGT_TraceEachStep   = false;
	static bool const DOWNAGT_TraceEachStep = false;
};

#endif // !CWZ_NON_LOCAL_COST_H


