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
	static void showSGECostList(SGECostNode **costList, int len);
};

struct SGNode{
	SGEdge **edges;
	uint32_t deg; //edge amount
	uint32_t grayPxl;// grayscale pixel
	SPGroup *group;

	SGNode() : edges(NULL), deg(0), grayPxl(0), group(NULL){
		this->edges = new SGEdge*[4];
		this->edges[0] = NULL;
		this->edges[1] = NULL;
		this->edges[2] = NULL;
		this->edges[3] = NULL;
	};
	void setPixel(uint32_t pixel);
	void addNewEdge(SGEdge *newEdge, int direction);
	void assignSPGRootGroup(SPGroup *newGroup);

	static int const SGEDGE_UP    = 1;
	static int const SGEDGE_LEFT  = 2;
	static int const SGEDGE_DOWN  = 3;
	static int const SGEDGE_RIGHT = 4;

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

#endif // !STANDARDGRAPH_H


