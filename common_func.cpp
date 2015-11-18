#include "common_func.h"
//讀檔案存成disparity map
void readDisparityFromFile(std::string fname, int h, int w, cv::Mat &dMap){
	std::ifstream fin;
	std::string line = "";

	fin.open(fname, std::ios::in);
	std::string delimiter = ",";
	int delLen = delimiter.length();
	int posi;
	int diffCount = 0;
	for(int y=0; y<h; y++){
		getline(fin, line);			
		for(int x=0; x<w; x++){
			posi = line.find(",");

			dMap.at<uchar>(y,x) = std::stod( line.substr(0, posi) );

			line.erase(0, posi + delLen);
		}
	}
}

//讀檔案存成match cost
float *readMatchCostFromFile(std::string fname, int h, int w, int max_disparity, float *my_match_cost){
	std::ifstream fin;
	std::string line = "";

	fin.open(fname, std::ios::in);

	float *match_cost = new float[w*h*max_disparity];
	std::string delimiter = ",";
	int delLen = delimiter.length();
	int idx=0;
	int posi;
	int diffCount = 0;
	for(int y=0; y<h; y++){
		for(int x=0; x<w; x++){
			getline(fin, line);			
			for(int d=0 ; d<max_disparity-1 ; d++){
				posi = line.find(",");
				match_cost[idx] = std::stof( line.substr(0, posi) );

				float diff = my_match_cost[idx] - match_cost[idx];
				if( diff > 0.01 || diff < -0.01){
					diffCount++;
					printf("diff(%f) diffCount:%d\n", diff, diffCount);
				}

				line.erase(0, posi + delLen);
				idx++;
			}
			match_cost[idx] = std::stof( line );
			idx++;
		}
		printf("y:%d\n", y);
	}

	return match_cost;
}

//讀line轉成blocks處理
int closestDelimiterPosi(std::string str, std::string *delimiters, int delCount, int &delLength){
	int minIdx = str.length();
	bool isFound = false;
	int posi;
	for(int i=0 ; i<delCount ; i++){
		if( (posi = str.find(delimiters[i])) != std::string::npos)
		{// delimiter is found
			if( posi < minIdx ){
				minIdx = posi;
				delLength = delimiters[i].length();
			}
			isFound = true;
		}
	}

	if(isFound)
		return minIdx;
	else
		return -1;
}
std::string *splitInstructions(std::string str, std::string *delimiters, int delCount, int &length){
	std::string buffer[10];
	int idx = 0;
	
	int posi=0;
	std::string tmp = "";

	int delimiterLen = 0; // match 到的delimiter字串長度
	while( (posi = closestDelimiterPosi(str, delimiters, delCount, delimiterLen)) != -1 )
	{
		tmp = str.substr(0, posi);
		//cout <<" " << tmp << " ";
		buffer[idx] = tmp;
		//cout <<" " << result[idx] << " ";
		str.erase(0, posi + delimiterLen);
		if(tmp.length() >= 1)
		{//要有東西才算一個
			idx++;
		}
	}
	
	tmp = str.substr(0, str.length());
	buffer[idx] = tmp;
	//cout <<" "<< result[idx] << " ";
	if(tmp.length() >= 1)
	{//要有東西才算一個
		idx++;
	}

	length = idx;
	std::string *result = new std::string[length];
	for(int i=0 ; i<length ;i++)
	{
		result[i] = buffer[i];
	}

	return result;
}
float *splitDataContent(std::string str, std::string delimiter, int &length){
	float buffer[200];
	int idx = 0;
	
	int posi=0;
	std::string tmp = "";

	int delimiterLen = 0; // match 到的delimiter字串長度
	while( (posi = str.find(delimiter)) != std::string::npos )
	{
		tmp = str.substr(0, posi);
		//cout <<" " << tmp << " ";
		buffer[idx] = std::stod(tmp);
		//cout <<" " << result[idx] << " ";
		str.erase(0, posi + delimiter.length());
		if(tmp.length() >= 1)
		{//要有東西才算一個
			idx++;
		}
	}

	tmp = str.substr(0, str.length());
	if(tmp.length() >= 1){
		buffer[idx] = std::stod(tmp);
		//cout <<" "<< result[idx] << " ";
		if(tmp.length() >= 1)
		{//要有東西才算一個
			idx++;
		}
	}

	length = idx;
	float *result = new float[length];
	for(int i=0 ; i<length ;i++)
	{
		result[i] = buffer[i];
	}

	return result;
}

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