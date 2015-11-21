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

int *c3_mat_to_1d_int_arr(cv::Mat img, int h, int w){
	int *arr = new int [h*w];
	int idx = 0;
	for(int y=0; y<h ; y++)
	for(int x=0; x<w*3 ; x+=3)
	{
		arr[idx] =   img.at<uchar>(y, x  )       |
				    (img.at<uchar>(y, x+1) << 8) |
					(img.at<uchar>(y, x+2) << 16);
		idx++;
	}
	
	return arr;
}

uchar **int_2d_arr_to_gray_arr(int **color_arr, int h, int w){
	uchar **arr = new_2d_arr<uchar>(h, w);
	uchar *color = new uchar[3];
	int mask_b = 0xFF;
	int mask_g = mask_b << 8;
	int mask_r = mask_g << 8;
	for(int y=0; y<h ; y++)
	for(int x=0; x<w ; x++)
	{
		color[2] = (color_arr[y][x]&mask_b);
		color[1] = (color_arr[y][x]&mask_g >> 8);
		color[0] = (color_arr[y][x]&mask_r >> 16);
		arr[y][x] = rgb_2_gray( color );
	}
	delete[] color;
	return arr;
}
uchar *int_1d_arr_to_gray_arr(int *color_arr, int node_c){
	uchar *arr = new uchar[node_c];
	uchar *color = new uchar[3];
	int mask_b = 0xFF;
	int mask_g = mask_b << 8;
	int mask_r = mask_g << 8;
	for(int i=0; i<node_c ; i++)
	{
		color[2] = (color_arr[i]&mask_b);
		color[1] = (color_arr[i]&mask_g >> 8);
		color[0] = (color_arr[i]&mask_r >> 16);
		arr[i] = rgb_2_gray( color );
	}
	delete[] color;
	return arr;
}