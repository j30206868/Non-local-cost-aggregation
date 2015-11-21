#ifndef CWZ_COMMON_FUNC_H
#define CWZ_COMMON_FUNC_H

#include <opencv2\opencv.hpp>

#include <fstream>

struct cl_match_elem{
	int *rgb;
	float *gradient;
	int node_c;
	cl_match_elem(int _node_c, int *_rgb, float *_gradient){
		this->node_c = _node_c;
		this->rgb      = _rgb;
		this->gradient = _gradient;
	}
	cl_match_elem(int _node_c){
		this->node_c = _node_c;
		this->rgb      = new int[node_c];
		this->gradient = new float[node_c];
	}
};

template <class T>T *new_1d_arr(int len, T init_value){
	T *arr = new T[len];
	for(int i=0; i<len; i++){
		arr[i] = init_value;
	}
	return arr;
}

template <class T>T **new_2d_arr(int rows, int cols, T init_value){
	T **arr = new T*[rows];
	for(int i=0; i<rows; i++){
		arr[i] = new T[cols];
		for(int j=0; j<cols; j++){
			arr[i][j] = init_value;
		}
	}
	return arr;
}
template <class T>T **new_2d_arr(int rows, int cols){
	T **arr = new T*[rows];
	for(int i=0; i<rows; i++){
		arr[i] = new T[cols];
	}
	return arr;
}

template <class T>T ***new_3d_arr(int h, int w, int b){
	T ***arr = new T**[h];
	for(int i=0; i<h; i++){
		arr[i] = new T*[w];
		for(int j=0 ; j < w ; j++){
			arr[i][j] = new T[b];
		}
	}
	return arr;
}

template <class T>
void free_2d_arr(T **arr, int rows, int cols)
{
	for(int i=0; i<rows; i++)
	{
		delete[] arr[i];
	}
	delete[] arr;
}
template <class T>
void free_3d_arr(T ***arr, int h, int w, int b)
{
	for(int i=0; i<h ; i++)
	{
		for(int j=0; j<w; j++)
		{
			delete[] arr[i][j];
		}
		delete[]  arr[i];
	}
	delete[] arr;
}


//將format string切成陣列
int closestDelimiterPosi(std::string str, std::string *delimiters, int delCount, int &delLength);
std::string *splitInstructions(std::string str, std::string *delimiters, int delCount, int &length);
float *splitDataContent(std::string str, std::string delimiter, int &length);

//讀檔案存成match cost
float *readMatchCostFromFile(std::string fname, int h, int w, int max_disparity, float *my_match_cost);
void readDisparityFromFile(std::string fname, int h, int w, cv::Mat &dMap);

int *c3_mat_to_1d_int_arr(cv::Mat img, int h, int w);
template<class T>
T **map_1d_arr_to_2d_arr(T *arr, int h, int w){
	T **arr_2d = new T*[h];
	for(int y=0 ; y<h ; y++){
		int offset = y * w;
		arr_2d[y] = &arr[offset];
	}
	return arr_2d;
}

uchar **int_2d_arr_to_gray_arr(int **color_arr, int h, int w);
uchar *int_1d_arr_to_gray_arr(int *color_arr, int node_c);
inline uchar rgb_2_gray(uchar*in){return(uchar(0.299*in[0]+0.587*in[1]+0.114*in[2]+0.5));}

//check if type equals
template<typename T, typename U>
struct is_same
{
    static const bool value = false;
};

template<typename T>
struct is_same<T, T>
{
    static const bool value = true;
};

template<typename T, typename U>
bool eqTypes() { return is_same<T, U>::value; }

#endif //CWZ_COMMON_FUNC_H