#include "StdGraph.h"
#include <ctime>

using namespace cv;
using namespace std;

int const w = 450;
int const h = 375;

int main(){
	clock_t start, stop;
	Mat image;
    image = imread("dolls1.png", CV_LOAD_IMAGE_COLOR);   // Read the file

    if(! image.data )                              // Check for invalid input
    {
        cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

	SGNode **nodeList;

	/*int **mat = new int*[w]; 
	mat[0] = new int[h]; 
	mat[1] = new int[h];
	mat[2] = new int[h];
	mat[0][0] = 7;		mat[0][1] = 12;			mat[0][2] = 1;			mat[0][3] = 0;
	mat[1][0] = 70;		mat[1][1] = 10;			mat[1][2] = 255;		mat[1][3] = 254;
	mat[2][0] = 50;		mat[2][1] = 255;		mat[2][2] = 239;		mat[2][3] = 100;*/
	
	int **mat = new int*[w]; 
	for(int i=0 ; i<w ; i++){
		mat[i] = new int[h];
		//mat[i] = image.row(j);
		for(int j=0; j<h ; j++){
			uchar& uxy = image.at<uchar>(j, i);
			mat[i][j] = (int)uxy;
		}
	}

	start = clock(); //開始時間
	nodeList = SGNode::createSGNodeList(w, h);
	stop = clock();
	double T_create_node_list = double(stop - start);

	start = clock(); //開始時間
	SGECostNode **cList = buildCostList((SGNode **)nodeList, (int **)mat, w, h);
	stop = clock();
	double T_build_cost_list = double(stop - start);
	
	start = clock(); //開始時間
	addMSTEdgeToNodeList(nodeList, cList, IntensityLimit, w, h);
	stop = clock();
	double T_do_MST = double(stop - start);

	/*printf("時間花費 CreateNodeList: %f s\n", T_create_node_list / CLOCKS_PER_SEC );
	printf("時間花費 建costList    : %f s\n", T_build_cost_list / CLOCKS_PER_SEC );
	printf("時間花費 T_do_MST      : %f s\n", T_do_MST / CLOCKS_PER_SEC );
	system("PAUSE");*/

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

	printf("Image Size(%3d, %3d)\n", w, h);
	printf("Group Count            : %d\n", SPGroup::groupC);
	printf("時間花費 CreateNodeList: %f s\n", T_create_node_list / CLOCKS_PER_SEC );
	printf("時間花費 建costList    : %f s\n", T_build_cost_list / CLOCKS_PER_SEC  );
	printf("時間花費 T_do_MST      : %f s\n", T_do_MST / CLOCKS_PER_SEC           );
	printf("時間花費 UP AGT        : %f s\n", T_up_agt / CLOCKS_PER_SEC           );
	printf("時間花費 DOWN AGT      : %f s\n", T_down_agt / CLOCKS_PER_SEC         );

	printf("\nAfter downward aggregating Graph:\n");
	//SGNode::showSGNodeList(nodeList, w, h);

	system("PAUSE");
	return 0;
}