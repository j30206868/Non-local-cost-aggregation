#ifndef CWZ_MST_H
#define CWZ_MST_H

#include "cwz_non_local_cost.h"

typedef uchar TEleUnit;

class cwz_mst{
public:
	static float sigma;

	cwz_mst():isInit(false), hasImg(false){}
	void init(int _h, int _w, int _ch);
	void set_img(TEleUnit *_img);
	//
	void build_edges();
	void counting_sort();
	void kruskal_mst();
	void build_tree();
	//
	void cost_agt(float *match_cost_result);
	TEleUnit* pick_best_dispairty();
	//profiling
	void mst();
	void profile_mst();
	//for reuse
	void reinit();
	//test use function
	void test_correctness();

	TEleUnit *best_disparity;
private:
	int findset(int i);

	bool isInit;
	bool hasImg;

	TEleUnit *img;
	short *distance;
	int **edge_node_list;//[2]edge兩端的node的idx
	int *cost_sorted_edge_idx;
	int *node_group;
	
	int **node_conn_node_list;//[4]所有與此node相連的node(中間存在edge的另一端點)
	int **node_conn_weights;  //[4]記錄被連接的edge的weight
	int *node_conn_node_num;  //記錄node_conn_node_list中有幾個點跟此點連接

	int histogram[IntensityLimit];

	int *id_stack;//node index
	int id_stack_e;
	int *node_parent_id;

	//最終要使用的 tree 結構
	int *child_node_num;
	int **child_node_list;
	int *node_weight;//weight on the edge with its parent ( root's weight will be -1 )
	int *node_idx_from_p_to_c;//node id from parent to children
	
	float *agt_result;
	float whistogram[IntensityLimit];

	int h, w;
	int edge_amt;
	int node_amt;
	int channel;
};

/*namespace cwz_mst_filter{
	float *up_cost_agt(float *match_cost_result, int *node_parent_id, int **child_node_list, int *child_node_num, int *node_weight, int *node_idx_from_p_to_c, float *histogram, int node_amt);
};*/

#endif