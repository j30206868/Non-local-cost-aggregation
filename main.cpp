#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>

#include <time.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include <opencv2\opencv.hpp>
#include "cwz_non_local_cost.h"
#include "cl_data_type.h"

const char* LeftIMGName  = "dolls/dolls1.png"; 
const char* RightIMGName = "dolls/dolls2.png";

cl_program load_program(cl_context context, const char* filename)
{
	std::ifstream in(filename, std::ios_base::binary);
	if(!in.good()) {
		return 0;
	}

	// get file length
	in.seekg(0, std::ios_base::end);
	size_t length = in.tellg();
	in.seekg(0, std::ios_base::beg);

	// read program source
	std::vector<char> data(length + 1);
	in.read(&data[0], length);
	data[length] = 0;

	// create and build program 
	const char* source = &data[0];
	cl_program program = clCreateProgramWithSource(context, 1, &source, 0, 0);
	if(program == 0) {
		return 0;
	}

	if(clBuildProgram(program, 0, 0, 0, 0, 0) != CL_SUCCESS) {
		return 0;
	}

	return program;
}

void compute_gradient(float*gradient, uchar **gray_image, int h, int w)
{
	float gray,gray_minus,gray_plus;
	int node_idx = 0;
	for(int y=0;y<h;y++)
	{
		gray_minus=gray_image[y][0];
		gray=gray_plus=gray_image[y][1];
		gradient[node_idx]=gray_plus-gray_minus+127.5;

		node_idx++;

		for(int x=1;x<w-1;x++)
		{
			gray_plus=gray_image[y][x+1];
			gradient[node_idx]=0.5*(gray_plus-gray_minus)+127.5;

			gray_minus=gray;
			gray=gray_plus;
			node_idx++;
		}
		
		gradient[node_idx]=gray_plus-gray_minus+127.5;
		node_idx++;
	}
}

cl_device_id setup_opencl(cl_context &context, cl_int &err){
	cl_uint num;
	err = clGetPlatformIDs(0, 0, &num);
	if(err != CL_SUCCESS) {
		std::cerr << "Unable to get platforms\n";
		return 0;
	}

	std::vector<cl_platform_id> platforms(num);
	err = clGetPlatformIDs(num, &platforms[0], &num);
	if(err != CL_SUCCESS) {
		std::cerr << "Unable to get platform ID\n";
		return 0;
	}

	cl_context_properties prop[] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platforms[0]), 0 };
	context = clCreateContextFromType(prop, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, NULL);
	if(context == 0) {
		std::cerr << "Can't create OpenCL context\n";
		return 0;
	}

	size_t cb;
	clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &cb);
	std::vector<cl_device_id> devices(cb / sizeof(cl_device_id));
	clGetContextInfo(context, CL_CONTEXT_DEVICES, cb, &devices[0], 0);

	clGetDeviceInfo(devices[0], CL_DEVICE_NAME, 0, NULL, &cb);
	std::string devname;
	devname.resize(cb);
	clGetDeviceInfo(devices[0], CL_DEVICE_NAME, cb, &devname[0], 0);
	std::cout << "Device: " << devname.c_str() << "\n";
	return devices[0];
}

int main()
{
	//cv::Mat ppmimg = cv::imread("hand.ppm");
	//cv::imwrite("hand_mst_no_ctmf.bmp", ppmimg);

	//build MST
	//cv::Mat left = cv::imread(LeftIMGName, CV_LOAD_IMAGE_COLOR);
	//cv::Mat right = cv::imread(RightIMGName, CV_LOAD_IMAGE_COLOR);

	cv::FileStorage fs("imageLR.xml", cv::FileStorage::READ);
    if( fs.isOpened() == false)
    {
        printf( "No More....Quitting...!" );
        return 0;
    }

    cv::Mat matL , matR; //= Mat(480, 640, CV_16UC1);
    fs["left"] >> matL; 
	fs["right"] >> matR;                
    fs.release();

	cv::Mat left = cv::Mat(480, 640, CV_8UC3);
	cv::Mat right = cv::Mat(480, 640, CV_8UC3);

	for(int y=0; y<left.rows ; y++){
		int x_ = 0;
		for(int x=0; x<left.cols ; x++)
		{
			uchar lvalue = matL.at<unsigned short>(y, x) / 4;
			left.at<uchar>(y, x_  ) = lvalue;
			left.at<uchar>(y, x_+1) = lvalue;
			left.at<uchar>(y, x_+2) = lvalue;

			uchar rvalue = matR.at<unsigned short>(y, x) / 4;
			right.at<uchar>(y, x_  ) = rvalue;
			right.at<uchar>(y, x_+1) = rvalue;
			right.at<uchar>(y, x_+2) = rvalue;

			x_+=3;
		}
	}

	/************************************/
	time_t img_init_s = clock();

	int w = left.cols;
	int h = left.rows;

	int **left_color_arr = c3_mat_to_2d_int_arr(left, left.rows, left.cols);
	int **right_color_arr = c3_mat_to_2d_int_arr(right, right.rows, right.cols);

	uchar **left_gray_arr  = int_2d_arr_to_gray_arr(left_color_arr, h, w);
	uchar **right_gray_arr = int_2d_arr_to_gray_arr(right_color_arr, h, w);

	cl_match_elem *left_cwz_img  = cvmat_colorimg_to_match_elem(left_color_arr, h, w);
	cl_match_elem *right_cwz_img = cvmat_colorimg_to_match_elem(right_color_arr, h, w);
	printf("陣列init花費時間: %fs\n", double(clock() - img_init_s) / CLOCKS_PER_SEC);
	
	compute_gradient(left_cwz_img->gradient , left_gray_arr, h, w);
	compute_gradient(right_cwz_img->gradient, right_gray_arr, h, w);

	SGNode **nodeList = SGNode::createSGNodeList(w, h);

	CostAggregator *ca = new CostAggregator(nodeList, w, h);

	int match_result_len = h * w * disparityLevel;
	float *matching_result = new float[match_result_len];

	/*******************************************************
							 OpenCL
	*******************************************************/
	cl_int err;
	cl_context context;
	cl_device_id device = setup_opencl(context, err);

	cl_program program = load_program(context, "test.cl");
	if(program == 0) { std::cerr << "Can't load or build program\n"; clReleaseContext(context); return 0; }

	cl_kernel matcher = clCreateKernel(program, "matching_cost", 0);
	if(matcher == 0) { std::cerr << "Can't load kernel\n"; clReleaseProgram(program); clReleaseContext(context); return 0; }

	match_info *info = (match_info *) malloc(sizeof(info));
	info->img_width = w; info->max_d = disparityLevel;

	time_t step_up_kernel_s = clock();

	cl_mem cl_l_rgb = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * left_cwz_img->node_c, &left_cwz_img->rgb[0], NULL);
	cl_mem cl_l_gradient = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * left_cwz_img->node_c, &left_cwz_img->gradient[0], NULL);

	cl_mem cl_r_rgb = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * right_cwz_img->node_c, &right_cwz_img->rgb[0], NULL);
	cl_mem cl_r_gradient = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * right_cwz_img->node_c, &right_cwz_img->gradient[0], NULL);

	cl_mem cl_match_result = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * match_result_len, NULL, NULL);

	cl_mem cl_match_info = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(match_info), info, NULL);

	if(cl_l_rgb == 0 || cl_l_gradient == 0 ||
	   cl_r_rgb == 0 || cl_r_gradient == 0 ||
	   cl_match_result == 0) {
		std::cerr << "Can't create OpenCL buffer\n";
		clReleaseProgram(program);
		clReleaseKernel(matcher);
		clReleaseMemObject(cl_l_rgb);
		clReleaseMemObject(cl_l_gradient);
		clReleaseMemObject(cl_r_rgb);
		clReleaseMemObject(cl_r_gradient);
		clReleaseMemObject(cl_match_result);
		clReleaseContext(context);
		return 0;
	}

	clSetKernelArg(matcher, 0, sizeof(cl_mem), &cl_l_rgb);
	clSetKernelArg(matcher, 1, sizeof(cl_mem), &cl_l_gradient);
	clSetKernelArg(matcher, 2, sizeof(cl_mem), &cl_r_rgb);
	clSetKernelArg(matcher, 3, sizeof(cl_mem), &cl_r_gradient);
	clSetKernelArg(matcher, 4, sizeof(cl_mem), &cl_match_result);
	clSetKernelArg(matcher, 5, sizeof(cl_mem), &cl_match_info);
	
	cl_command_queue queue = clCreateCommandQueue(context, device, 0, 0);
	if(queue == 0) {
		std::cerr << "Can't create command queue\n";
		clReleaseKernel(matcher);
		clReleaseProgram(program);
		clReleaseMemObject(cl_l_rgb);
		clReleaseMemObject(cl_l_gradient);
		clReleaseMemObject(cl_r_rgb);
		clReleaseMemObject(cl_r_gradient);
		clReleaseMemObject(cl_match_result);
		clReleaseContext(context);
		return 0;
	}
	printf("Setup kernel Time: %.6fs\n", double(clock() - step_up_kernel_s) / CLOCKS_PER_SEC);

	size_t work_size = w * h;
	clock_t tOfCLStart = clock();
    /* Do your stuff here */
	err = clEnqueueNDRangeKernel(queue, matcher, 1, 0, &work_size, 0, 0, 0, 0);

	if(err == CL_SUCCESS) {
		err = clEnqueueReadBuffer(queue, cl_match_result, CL_TRUE, 0, sizeof(float) * match_result_len, &matching_result[0], 0, 0, 0);
	}
	printf("CL Time taken: %.6fs\n", (double)(clock() - tOfCLStart)/CLOCKS_PER_SEC);

	if(err == CL_SUCCESS) {

	}
	else {
		std::cerr << "Can't run kernel or read back data\n";	
	}

	clReleaseKernel(matcher);
	clReleaseProgram(program);
	clReleaseMemObject(cl_l_rgb);
	clReleaseMemObject(cl_l_gradient);
	clReleaseMemObject(cl_r_rgb);
	clReleaseMemObject(cl_r_gradient);
	clReleaseMemObject(cl_match_result);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	int mr_idx = 0;
	for(int y=0 ; y<h ; y++)
	for(int x=0 ; x<w ; x++)
	for(int d=0 ; d<disparityLevel ; d++)
	{
		nodeList[y][x].agtCost[d] = matching_result[mr_idx];
		mr_idx++;
	}

	//已經驗正matching cost的結果是一樣的
	//matching_result = readMatchCostFromFile("cost.txt", h, w, disparityLevel, matching_result);

	//Keep doing MST
	SGECostNode **cList = buildCostListFromCV(nodeList, left_gray_arr, w, h);
	addMSTEdgeToNodeList(nodeList, cList, IntensityLimit, w, h);
	//Aggregation
	ca->upwardAggregation(w/2, h/2, -1);
	printf("count:%d total weight:%d\n", ca->totalWeight, ca->MSTWeight);

	ca->downwardAggregation(w/2, h/2, -1);

	ca->pickBestDepthForNodeList();

	//ca->showDisparityMap();
	cv::Mat dMap(h, w, CV_8U);
	for(int y=0 ; y<h ; y++) for(int x=0 ; x<w ; x++)
	{
		dMap.at<uchar>(y,x) = nodeList[y][x].dispairty * (double) IntensityLimit / (double)disparityLevel;
	}

	//readDisparityFromFile("disparity.txt", h, w, dMap);

	cv::imwrite("dMap.bmp", dMap);
	cv::namedWindow("testw", CV_NORMAL);
	cv::imshow("testw",dMap);
	cv::waitKey(0);

	system("PAUSE");

	return 0;
}