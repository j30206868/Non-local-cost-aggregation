#ifndef CWZ_COMMON_FUNC_H
#define CWZ_COMMON_FUNC_H

#include <opencv2\opencv.hpp>

struct cwz_img{
	uchar *r;
	uchar *g;
	uchar *b;
	float *gradient;
	int node_c;
	
	cwz_img(int _node_c){
		this->node_c = _node_c;
		this->r        = new uchar[node_c];
		this->g        = new uchar[node_c];
		this->b        = new uchar[node_c];
		this->gradient = new float[node_c];
	}
};


inline unsigned char rgb_2_gray(unsigned char*in){
	return(unsigned char(0.299*in[0]+0.587*in[1]+0.114*in[2]+0.5));
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

uchar ***cvmat_to_3d_arr(cv::Mat img, int h, int w, int b);
uchar **cvmat_to_2d_arr(cv::Mat img, int h, int w);

cwz_img *cvmat_colorimg_to_cwzimg(uchar ***color_arr, int h, int w);
cwz_img *cvmat_colorimg_to_cwzimg(cv::Mat img, int h, int w);

#endif //CWZ_COMMON_FUNC_H