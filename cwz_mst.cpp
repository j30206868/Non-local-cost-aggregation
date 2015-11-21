#include "cwz_mst.h"

float cwz_mst::sigma = 0.1;

inline int get_1d_idx_from_2d(int x, int y, int w){
	return y * w + x;
}
inline void addEdge(TEleUnit *img, int **edge_node_list, short *distance, int edge_idx, int x0, int y0, int x1, int y1, int w, int channel){
	int idx0 = edge_node_list[edge_idx][0] = get_1d_idx_from_2d(x0, y0, w);
	int idx1 = edge_node_list[edge_idx][1] = get_1d_idx_from_2d(x1, y1, w);

	if( channel == 1 ){
		distance[edge_idx] = std::abs(img[idx0] - img[idx1]);
	}else if( channel == 3 ){
		idx0 *= 3;
		idx1 *= 3;

		distance[edge_idx] = std::abs(img[idx0  ] - img[idx1  ]) +
							 std::abs(img[idx0+1] - img[idx1+1]) +
							 std::abs(img[idx0+2] - img[idx1+2]);
	}else{
		printf("cwz_mst addEdge(...): channel is not equal to 1 nor 3.\n");
		system("PAUSE");
	}
}
inline void push(int id_in, int *stack, int &end){
	end++;
	stack[end] = id_in;
}
inline int pop(int *stack, int &end){
	int result = -1;
	if(end > -1){
		result = stack[end];
		end--;
	}
	return result;
}

void cwz_mst::init(int _h, int _w, int _ch){
	this->h = _h;
	this->w = _w;
	this->node_amt = _h * _w;
	this->edge_amt = (_h-1) * _w + h * (_w-1);
	this->channel = _ch;

	this->edge_node_list = new_2d_arr<int>(this->edge_amt, 2);
	this->distance = new short[edge_amt];
	this->cost_sorted_edge_idx = new int[edge_amt];

	this->node_conn_node_list = new_2d_arr<int>(this->node_amt, 4);
	this->node_conn_weights   = new_2d_arr<int>(this->node_amt, 4);
	this->node_conn_node_num  = new_1d_arr(this->node_amt, 0);

	this->node_group = new int[this->node_amt];
	for(int i=0 ; i<this->node_amt ; i++){ this->node_group[i] = i; }

	this->id_stack = new int[this->node_amt];
	this->id_stack_e = -1;
	this->node_parent_id = new_1d_arr<int>(this->edge_amt, -1);
	//最終要使用的 tree 結構
	this->child_node_num = new_1d_arr<int>(this->node_amt, 0);
	this->child_node_list = new_2d_arr<int>( this->node_amt, 4, -1);
	this->node_idx_from_p_to_c= new_1d_arr(this->node_amt, -1);
	this->node_weight = new int[this->node_amt];

	for(int i=0 ; i<IntensityLimit ; i++){ histogram[i] = 0; }

	this->agt_result = new float[ node_amt * disparityLevel ];
	this->best_disparity = new TEleUnit[node_amt];

	for(int i=0 ; i<IntensityLimit ; i++){
		whistogram[i] = exp(-double(i) / (cwz_mst::sigma * (IntensityLimit - 1)));
	}

	this->isInit = true;
}
void cwz_mst::set_img(TEleUnit *_img){
	this->img = _img;
	this->hasImg = true;
}
void cwz_mst::build_edges(){
	int y0,y1,x0,x1;
	int edge_idx = 0;
//先加入橫邊(左右的)
	for(y0=0;y0<h;y0++)
	{
		y1=y0;
		for(int x0=0;x0<w-1;x0++)
		{
			x1=x0+1;
			addEdge(img, edge_node_list, distance,  edge_idx++, x0, y0, x1, y1, w, channel);
		}
	}
//再加入直邊(上下的)
	for(int x0=0;x0<w;x0++)
	{
		x1=x0;
		for(y0=0;y0<h-1;y0++)
		{
			y1=y0+1;
			addEdge(img, edge_node_list, distance,  edge_idx++, x0, y0, x1, y1, w, channel);
		}
	}
}
void cwz_mst::counting_sort(){
	for(int i=0 ; i<edge_amt ; i++){ histogram[ distance[i] ]++; }
	//calculate start index for each intensity level
	int his_before = histogram[0];
	int his_this;
	histogram[0] = 0;
	for(int i=1 ; i<IntensityLimit ; i++){
		his_this = histogram[i]; 
		histogram[i] = histogram[i-1] + his_before;
		his_before = his_this;
	}
	for(int i=0 ; i<edge_amt ; i++){
		int deserved_order = histogram[ distance[i] ]++;
		cost_sorted_edge_idx[ deserved_order ] = i;
	}
}
int  cwz_mst::findset(int i){
	if(node_group[i] != i){
		node_group[i] = findset(node_group[i]);
	}
	return node_group[i];
}
void cwz_mst::kruskal_mst(){
	for(int i=0 ; i<edge_amt ; i++){
		int edge_idx = cost_sorted_edge_idx[i];

		int n0 = edge_node_list[ edge_idx ][0];
		int n1 = edge_node_list[ edge_idx ][1];
		
		int p0 = findset(n0);
		int p1 = findset(n1);

		if(p0 != p1){//此兩點不在同一個集合中
			//點與點互相連結
			node_conn_node_list[n0][ node_conn_node_num[n0] ] = n1;
			node_conn_node_list[n1][ node_conn_node_num[n1] ] = n0;

			node_conn_weights[n0][ node_conn_node_num[n0] ] = distance[edge_idx];
			node_conn_weights[n1][ node_conn_node_num[n1] ] = distance[edge_idx];

			node_conn_node_num[n0]++;
			node_conn_node_num[n1]++;

			node_group[p0] = p1;
		}
	}
}
void cwz_mst::build_tree(){
	int parent_id = 0;
	int possible_child_id = -1;
	int t_c = 0;//tree node counter

	node_idx_from_p_to_c[ t_c++ ] = parent_id;
	node_weight[parent_id]    = 0;
	node_parent_id[parent_id] = 0;

	push(0, id_stack, id_stack_e);
	while( id_stack_e > -1 ){
		parent_id = pop(id_stack, id_stack_e);

		for(int edge_c=0 ; edge_c < node_conn_node_num[parent_id] ; edge_c++){
			possible_child_id = node_conn_node_list[parent_id][edge_c];
			if( node_parent_id[possible_child_id] != -1 ){
				continue;//its the parent, pass
			}
			node_idx_from_p_to_c[ t_c++ ] = possible_child_id;
			node_weight[possible_child_id] = node_conn_weights[parent_id][edge_c];
			node_parent_id[possible_child_id] = parent_id;

			child_node_list[parent_id][ child_node_num[parent_id]++ ] = possible_child_id;

			if( node_conn_node_num[possible_child_id] > 0 )
				push(possible_child_id, id_stack, id_stack_e);
		}
	}
}
//
void cwz_mst::cost_agt(float *match_cost_result){
	
	//up agt
	for(int i = node_amt-1 ; i >= 0 ; i--){
		int node_i = node_idx_from_p_to_c[i];
		int node_disparity_i = node_i * disparityLevel;

		for(int d=0 ; d<disparityLevel ; d++)
			agt_result[ node_disparity_i+d ] = match_cost_result[ node_disparity_i+d ];

		for(int child_c=0 ; child_c < child_node_num[node_i] ; child_c++){
			int child_i = child_node_list[node_i][child_c];
			int child_disparity_i = child_i * disparityLevel;

			for(int d=0 ; d<disparityLevel ; d++){
				agt_result[ node_disparity_i+d ] += agt_result[ child_disparity_i+d ] * whistogram[ node_weight[child_i] ];
			}
		}
	}
	//down agt
	for(int i=1 ; i<node_amt ; i++){
		int node_i = node_idx_from_p_to_c[i];
		int parent_i = node_parent_id[node_i];
		int node_disparity_i = node_i * disparityLevel;
		int parent_disparity_i = parent_i * disparityLevel;

		float w = whistogram[ node_weight[ node_i ] ];
		float one_m_sqw = (1.0 - w * w);

		for(int d=0 ; d<disparityLevel ; d++){
			agt_result[node_disparity_i+d] = w           * agt_result[ parent_disparity_i+d ] +
											 (one_m_sqw) * agt_result[node_disparity_i+d];
		}
	}
}
TEleUnit *cwz_mst::pick_best_dispairty(){
	for(int i=0 ; i<this->node_amt ; i++){
		int i_ = i * disparityLevel;

		float min_cost = agt_result[i_+0];
		best_disparity[i] = 0;
		for(int d=1 ; d<disparityLevel ; d++){
			if( agt_result[i_+d] < min_cost)
				best_disparity[i] = d;
		}
	}
	return best_disparity;
}
//
void cwz_mst::mst(){
	if( this->isInit && this->hasImg ){
		this->build_edges();
		this->counting_sort();
		this->kruskal_mst();
		this->build_tree();
	}
}
void cwz_mst::profile_mst(){
	printf("--	start mst profiling	--\n");
	double total = 0;
	double build_edge_s;
	double counting_sort_s;
	double kruskal_s;
	double build_tree_s;
	time_t start;
	if( this->isInit && this->hasImg ){
		start = clock();
		this->build_edges();
		build_edge_s = double(clock() - start) / CLOCKS_PER_SEC;

		start = clock();
		this->counting_sort();
		counting_sort_s = double(clock() - start) / CLOCKS_PER_SEC;

		start = clock();
		this->kruskal_mst();
		kruskal_s = double(clock() - start) / CLOCKS_PER_SEC;

		start = clock();
		this->build_tree();
		build_tree_s = double(clock() - start) / CLOCKS_PER_SEC;

		printf("build_edges  : %2.4fs\n", build_edge_s);
		printf("counting_sort: %2.4fs\n", counting_sort_s);
		printf("kruskal_mst  : %2.4fs\n", kruskal_s);
		printf("build_tree   : %2.4fs\n", build_tree_s);
		printf("---------------------\n");
		printf("   total time: %2.4fs\n", build_edge_s+counting_sort_s+kruskal_s+build_tree_s);

		int total_w = 0;
		for(int i=0 ; i<this->node_amt ; i++){
			total_w += node_weight[i];
		}
		printf("     node_amt: %d nodes\n", node_amt);
		printf(" total weight: %d\n", total_w);
	}
	printf("--	endof mst profiling	--\n");
}
//for reuse
void cwz_mst::reinit(){
	memset(this->histogram, 0, sizeof(int) * IntensityLimit);
	for(int i=0 ; i<this->node_amt ; i++){ this->node_group[i] = i; }
	memset(this->node_conn_node_num, 0, sizeof(int) * this->node_amt);
	this->id_stack_e = -1;
	memset(this->child_node_num, 0, sizeof(int) * this->node_amt); 
	memset(this->node_parent_id, -1, sizeof(int) * this->node_amt); 
}
//for testing
void cwz_mst::test_correctness(){
	if( eqTypes<TEleUnit, uchar>() )
	{//8UC1 test
		printf("\n*********************************************\n");
		printf("cwz_mst::test_correctness() 8UC1 test called:\n");
		
		int h = 3, w = 3;
		int node_c = w*h;
		uchar *arr = new uchar[node_c];
		arr[0] = 255;	arr[1] = 254;	arr[2] = 250;
		arr[3] = 171;	arr[4] =  20;	arr[5] =   0;
		arr[6] = 150;	arr[7] =  11;	arr[8] =  11;

		this->init(h, w, 1);
		this->set_img(arr);
		this->build_edges();
		this->counting_sort();
		this->kruskal_mst();
		this->build_tree();

		printf("node_idx_from_p_to_c: \n");
		for(int i=0 ; i<node_c ; i++){
			printf("	node_idx_from_p_to_c[%2d]: %2d\n", i, node_idx_from_p_to_c[i]);
		}
		printf("weights: \n");
		for(int i=0 ; i<node_c ; i++){
			printf("	node_weight[%2d]: %2d\n", i, node_weight[i]);
		}

		//reuse test
		printf("--	--	reuseness test	--	--\n");
		arr[0] = 255;	arr[1] = 254;	arr[2] =   0;
		arr[3] = 171;	arr[4] =  20;	arr[5] =   0;
		arr[6] = 150;	arr[7] =  11;	arr[8] =  11;
		this->reinit();
		this->build_edges();
		this->counting_sort();
		this->kruskal_mst();
		this->build_tree();
		printf("node_idx_from_p_to_c: \n");
		for(int i=0 ; i<node_c ; i++){
			printf("	node_idx_from_p_to_c[%2d]: %2d\n", i, node_idx_from_p_to_c[i]);
		}
		printf("weights: \n");
		for(int i=0 ; i<node_c ; i++){
			printf("	node_weight[%2d]: %2d\n", i, node_weight[i]);
		}

		//free memory

		printf("\n*********************************************\n");
	}
}

/*float *cwz_mst_filter::up_cost_agt(float *match_cost_result, int *node_parent_id, int **child_node_list, int *child_node_num, int *node_weight, int *node_idx_from_p_to_c, float *histogram, int node_amt){
	float *agt_result = new float[ node_amt * disparityLevel ];
	//up agt
	for(int i = node_amt-1 ; i >= 0 ; i--){
		int node_i = node_idx_from_p_to_c[i];
		int node_disparity_i = node_i * disparityLevel;

		for(int d=0 ; d<disparityLevel ; d++)
			agt_result[ node_disparity_i+d ] = match_cost_result[ node_disparity_i+d ];

		for(int child_c=0 ; child_c < child_node_num[node_i] ; child_c++){
			int child_i = child_node_list[node_i][child_c];
			int child_disparity_i = child_i * disparityLevel;

			for(int d=0 ; d<disparityLevel ; d++){
				agt_result[ node_disparity_i+d ] += agt_result[ child_disparity_i+d ] * histogram[ node_weight[child_i] ];
			}
		}
	}
	//down agt
	for(int i=1 ; i<node_amt ; i++){
		int node_i = node_idx_from_p_to_c[i];
		int parent_i = node_parent_id[node_i];
		int node_disparity_i = node_i * disparityLevel;
		int parent_disparity_i = parent_i * disparityLevel;

		float w = histogram[ node_weight[ node_i ] ];
		float one_m_sqw = (1.0 - w * w);

		for(int d=0 ; d<disparityLevel ; d++){
			agt_result[node_disparity_i+d] = w           * agt_result[ parent_disparity_i+d ] +
											 (one_m_sqw) * agt_result[node_disparity_i+d];
		}
	}
	return agt_result;
}*/