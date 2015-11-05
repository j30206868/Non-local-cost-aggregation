#ifndef STANDARDGRAPH_H
#define STANDARDGRAPH_H

// for 4 conntected standard graph

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#define IntensityLimit 256

struct SPGroup{
	uint32_t id;
	SPGroup *next;
	uint32_t connCount; // connection counter

	SPGroup() : id(0), next(NULL), connCount(0) {
		SPGroup::groupC++;
		this->id = SPGroup::groupC;
	};
	int getId();
	SPGroup *getRootGroup();

	static uint32_t groupC;
	
	static SPGroup *createNewSPGroup();
	static inline void combineTwoSPGroup(SPGroup *g1, SPGroup *g2);
};

struct SGEdge{
	float w;
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
	uint32_t grayPxl;// grayscale pixel
	float agtCost;
	SPGroup *group;

	static int const EDGELIMIT = 4;
	SGNode() : edges(NULL), deg(0), grayPxl(0), agtCost(0), group(NULL){
		this->edges = new SGEdge*[EDGELIMIT];
		this->edges[SGEDGE_UP]	  = NULL;
		this->edges[SGEDGE_LEFT]  = NULL;
		this->edges[SGEDGE_DOWN]  = NULL;
		this->edges[SGEDGE_RIGHT] = NULL;
	};
	void setPixel(uint32_t pixel);
	void addNewEdge(SGEdge *newEdge, int direction);
	void assignSPGRootGroup(SPGroup *newGroup);

	//edge index(direction) flag
	static int const SGEDGE_UP    = 0;
	static int const SGEDGE_LEFT  = 1;
	static int const SGEDGE_DOWN  = 2;
	static int const SGEDGE_RIGHT = 3;

	static int getOppositeDirectionFlag(int direction);

	//edge type
	static int const EDGETYPE_VERTICLE   = 0;
	static int const EDGETYPE_HORIZONTAL = 1;

	static SGNode **createSGNodeList(int w, int h);
	static void showSGNodeListEdgeAndGroup(SGNode **nodeList, int w, int h);
	static void showSGNodeList(SGNode **nodeList, int w, int h);
};

/*********************************************
	  Kruskal's Algorithm Implementation
*********************************************/

void buildKruskalMST(SGNode **nodeList, int **mat, uint32_t w, uint32_t h);
void appendToSGECostList(SGECostNode **costList, SGECostNode *costNode, uint32_t idx); //index 就是 edge 的 weight (也就是vertex pixel相減的差值)
void addMSTEdgeToNodeList(SGNode **nodeList, SGECostNode **costList, int costListLen, int w, int h);
bool connectEdgeOfTwoSGNode(SGNode &node1, SGNode &node2, int edgeType, int cost);
SGNode *getRootOfMST(SGNode **nodeList, int w, int h);

/*******************************************************
		Non local cost aggregation Implementation
*******************************************************/
class CostAggregator{
	
public:
		CostAggregator(SGNode **nList);
		void upwardAggregation(int x, int y, int parentDirection);
		void downwardAggregation(int x, int y, int parentDirection);
private:
	static bool const UPAGT_TraceEachStep = false;
	SGNode **nodeList;
	void getPointedXY(int x, int y, int &resultX, int &resultY, int direction);
};


#endif // !STANDARDGRAPH_H


