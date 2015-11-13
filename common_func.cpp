#include "common_func.h"

cwz_img *cvmat_colorimg_to_cwzimg(uchar ***color_arr, int h, int w){
	int node_c = h * w;
	
	cwz_img *cwzimg = new cwz_img(node_c);
	
	int node_idx = 0;
	for(int y=0; y<h ; y++)
	{
		for(int x=0; x<w ; x++)
		{
			cwzimg->b[node_idx] = color_arr[y][x][0];
			cwzimg->g[node_idx] = color_arr[y][x][1];
			cwzimg->r[node_idx] = color_arr[y][x][2];

			node_idx++;
		}
	}
	return cwzimg;
}
cwz_img *cvmat_colorimg_to_cwzimg(cv::Mat img, int h, int w){
	int node_c = h * w;
	int w_ = w*3;
	
	cwz_img *cwzimg = new cwz_img(node_c);
	
	float gray,gray_minus,gray_plus;
	int node_idx = 0;
	for(int y=0; y<h ; y++)
	{
		cwzimg->b[node_idx] = img.at<uchar>(y, 0);
		cwzimg->g[node_idx] = img.at<uchar>(y, 1);
		cwzimg->r[node_idx] = img.at<uchar>(y, 2);

		//deal with gradient
		gray_minus=rgb_2_gray( &img.at<uchar>(y, 0) );
		gray=gray_plus=rgb_2_gray( &img.at<uchar>(y, 3) );
		cwzimg->gradient[node_idx]=gray_plus-gray_minus+127.5;
	
		node_idx++;

		for(int x=3; x<(w_-3) ; x+=3)
		{
			cwzimg->b[node_idx] = img.at<uchar>(y, x);
			cwzimg->g[node_idx] = img.at<uchar>(y, x+1);
			cwzimg->r[node_idx] = img.at<uchar>(y, x+2);
		
			//deal with gradient
			gray_plus=rgb_2_gray( &img.at<uchar>(y, x+3) );
			cwzimg->gradient[node_idx]=0.5*(gray_plus-gray_minus)+127.5;
			gray_minus=gray;
			gray=gray_plus;
			
			node_idx++;
		}
		//last pixel
		cwzimg->b[node_idx] = img.at<uchar>(y, w_-3);
		cwzimg->g[node_idx] = img.at<uchar>(y, w_-2);
		cwzimg->r[node_idx] = img.at<uchar>(y, w_-1);
		//deal with gradient
		cwzimg->gradient[node_idx] = gray_plus-gray_minus+127.5;

		node_idx++;
	}
	return cwzimg;
}
uchar ***cvmat_to_3d_arr(cv::Mat img, int h, int w, int b){
	uchar ***arr = new_3d_arr<uchar>(h, w, b);
	for(int y=0; y<h ; y++)
	{
		int x_ = 0;
		for(int x=0; x<w ; x++)
		{
			for(int p=0; p<b ; p++)
			{
				arr[y][x][p] = img.at<uchar>(y, x_+p);
			}
			x_+=3;
		}
	}
	return arr;
}
uchar **cvmat_to_2d_arr(cv::Mat img, int h, int w){
	uchar **arr = new_2d_arr<uchar>(h, w);
	for(int y=0; y<h ; y++)
	for(int x=0; x<w ; x++)
	{
		arr[y][x] = img.at<uchar>(y, x);
	}
	return arr;
}