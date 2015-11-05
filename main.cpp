#include "StdGraph.h"
#include <ctime>

int const w = 250;
int const h = 250;

void main(){
	clock_t start, stop;

	SGNode **nodeList;
	
	unsigned seed;
    seed = (unsigned)time(NULL); // ���o�ɶ��ǦC
    srand(seed); // �H�ɶ��ǦC��üƺؤl

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
	start = clock(); //�}�l�ɶ�
	nodeList = SGNode::createSGNodeList(w, h);
	stop = clock();
	double T_create_node_list = double(stop - start);

	start = clock(); //�}�l�ɶ�
	buildKruskalMST((SGNode **)nodeList, (int **)mat, w, h);
	stop = clock();
	double T_do_mst = double(stop - start);
	 
	
	printf("\nGroup info:\n");
	//SGNode::showSGNodeListEdgeAndGroup(nodeList, w, h);

	SGNode *root = getRootOfMST(nodeList, w, h);

	CostAggregator *ca = new CostAggregator(nodeList);
	start = clock(); //�}�l�ɶ�
	ca->upwardAggregation(w/2, h/2, -1);
	stop = clock();
	double T_up_agt = double(stop - start);
	 

	printf("Aggregated Graph:\n");
	//SGNode::showSGNodeList(nodeList, w, h);

	start = clock(); //�}�l�ɶ�
	ca->downwardAggregation(w/2, h/2, -1);
	stop = clock();
	double T_down_agt = double(stop - start);

	printf("Group Count: %d\n", SPGroup::groupC);
	printf("�ɶ���O CreateNodeList: %f s\n", T_create_node_list / CLOCKS_PER_SEC );
	printf("�ɶ���O MST           : %f s\n", T_do_mst / CLOCKS_PER_SEC );
	printf("�ɶ���O UP AGT        : %f s\n", T_up_agt / CLOCKS_PER_SEC );
	printf("�ɶ���O DOWN AGT      : %f s\n", T_down_agt / CLOCKS_PER_SEC );

	printf("\nAfter downward aggregating Graph:\n");
	//SGNode::showSGNodeList(nodeList, w, h);

	system("PAUSE");
}