#include "StdGraph.h"
#include <ctime>

int const w = 250;
int const h = 250;

void main(){
	clock_t start, stop;

	SGNode **nodeList;
	
	unsigned seed;
    seed = (unsigned)time(NULL); // 取得時間序列
    srand(seed); // 以時間序列當亂數種子

	/*int **mat = new int*[w]; 
	mat[0] = new int[h]; 
	mat[1] = new int[h];
	mat[2] = new int[h];
	mat[0][0] = 7;		mat[0][1] = 12;			mat[0][2] = 1;			mat[0][3] = 0;
	mat[1][0] = 70;		mat[1][1] = 10;			mat[1][2] = 255;		mat[1][3] = 254;
	mat[2][0] = 50;		mat[2][1] = 255;		mat[2][2] = 239;		mat[2][3] = 100;*/
	
	printf("%d\n", RAND_MAX);

	int **mat = new int*[w]; 
	for(int i=0 ; i<w ; i++){
		mat[i] = new int[h];
		for(int j=0; j<h ; j++){
			mat[i][j] = rand() % IntensityLimit;
		}
	}
	start = clock(); //開始時間
	nodeList = SGNode::createSGNodeList(w, h);
	stop = clock();
	double T_create_node_list = double(stop - start);

	start = clock(); //開始時間
	buildKruskalMST((SGNode **)nodeList, (int **)mat, w, h);
	stop = clock();
	double T_do_mst = double(stop - start);
	 
	
	printf("\nGroup info:\n");
	//SGNode::showSGNodeListEdgeAndGroup(nodeList, w, h);

	SGNode *root = getRootOfMST(nodeList, w, h);

	CostAggregator *ca = new CostAggregator(nodeList);
	start = clock(); //開始時間
	ca->upwardAggregation(w/2, h/2, -1);
	stop = clock();
	double T_up_agt = double(stop - start);
	 

	printf("Aggregated Graph:\n");
	//SGNode::showSGNodeList(nodeList, w, h);

	start = clock(); //開始時間
	ca->downwardAggregation(w/2, h/2, -1);
	stop = clock();
	double T_down_agt = double(stop - start);

	printf("Group Count: %d\n", SPGroup::groupC);
	printf("時間花費 CreateNodeList: %f s\n", T_create_node_list / CLOCKS_PER_SEC );
	printf("時間花費 MST           : %f s\n", T_do_mst / CLOCKS_PER_SEC );
	printf("時間花費 UP AGT        : %f s\n", T_up_agt / CLOCKS_PER_SEC );
	printf("時間花費 DOWN AGT      : %f s\n", T_down_agt / CLOCKS_PER_SEC );

	printf("\nAfter downward aggregating Graph:\n");
	//SGNode::showSGNodeList(nodeList, w, h);

	system("PAUSE");
}