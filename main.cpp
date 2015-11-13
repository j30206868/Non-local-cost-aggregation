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

const char* LeftIMGName  = "face/face2.png"; 
const char* RightIMGName = "face/face1.png";

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

int main()
{
	//build MST
	cv::Mat left = cv::imread(LeftIMGName, CV_LOAD_IMAGE_COLOR);
	cv::Mat right = cv::imread(RightIMGName, CV_LOAD_IMAGE_COLOR);

	cv::Mat left_gray;
	cv::Mat right_gray;

	cv::cvtColor(left, left_gray, CV_BGR2GRAY);
	cv::cvtColor(right, right_gray, CV_BGR2GRAY);

	int w = left.cols;
	int h = left.rows;

	uchar ***left_color_arr  = cvmat_to_3d_arr(left , left.rows , left.cols , 3);
	uchar ***right_color_arr = cvmat_to_3d_arr(right, right.rows, right.cols, 3);
	uchar **left_gray_arr    = cvmat_to_2d_arr(left_gray , left_gray.rows , left_gray.cols );
	uchar **right_gray_arr   = cvmat_to_2d_arr(right_gray, right_gray.rows, right_gray.cols);

	cwz_img *left_cwz_img = cvmat_colorimg_to_cwzimg(left_color_arr, h, w);
	cwz_img *right_cwz_img = cvmat_colorimg_to_cwzimg(right_color_arr, h, w);
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
	cl_context context = clCreateContextFromType(prop, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, NULL);
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

	cl_command_queue queue = clCreateCommandQueue(context, devices[0], 0, 0);
	if(queue == 0) {
		std::cerr << "Can't create command queue\n";
		clReleaseContext(context);
		return 0;
	}

	const int DATA_SIZE = w * h;

	cl_mem cl_l_b = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(uchar) * left_cwz_img->node_c, &left_cwz_img->b[0], NULL);
	cl_mem cl_l_g = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(uchar) * left_cwz_img->node_c, &left_cwz_img->g[0], NULL);
	cl_mem cl_l_r = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(uchar) * left_cwz_img->node_c, &left_cwz_img->r[0], NULL);
	cl_mem cl_l_gradient = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * left_cwz_img->node_c, &left_cwz_img->gradient[0], NULL);

	cl_mem cl_r_b = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(uchar) * right_cwz_img->node_c, &right_cwz_img->b[0], NULL);
	cl_mem cl_r_g = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(uchar) * right_cwz_img->node_c, &right_cwz_img->g[0], NULL);
	cl_mem cl_r_r = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(uchar) * right_cwz_img->node_c, &right_cwz_img->r[0], NULL);
	cl_mem cl_r_gradient = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * right_cwz_img->node_c, &right_cwz_img->gradient[0], NULL);

	cl_mem cl_match_result = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * match_result_len, NULL, NULL);

	if(cl_l_b == 0 || cl_l_g == 0 || cl_l_r == 0 || cl_l_gradient == 0 ||
	   cl_r_b == 0 || cl_r_g == 0 || cl_r_r == 0 || cl_r_gradient == 0 ||
	   cl_match_result == 0) {
		std::cerr << "Can't create OpenCL buffer\n";
		clReleaseMemObject(cl_l_b);
		clReleaseMemObject(cl_l_g);
		clReleaseMemObject(cl_l_r);
		clReleaseMemObject(cl_l_gradient);
		clReleaseMemObject(cl_r_b);
		clReleaseMemObject(cl_r_g);
		clReleaseMemObject(cl_r_r);
		clReleaseMemObject(cl_r_gradient);
		clReleaseMemObject(cl_match_result);
		clReleaseCommandQueue(queue);
		clReleaseContext(context);
		return 0;
	}

	cl_program program = load_program(context, "test.cl");
	if(program == 0) {
		std::cerr << "Can't load or build program\n";
		clReleaseMemObject(cl_l_b);
		clReleaseMemObject(cl_l_g);
		clReleaseMemObject(cl_l_r);
		clReleaseMemObject(cl_l_gradient);
		clReleaseMemObject(cl_r_b);
		clReleaseMemObject(cl_r_g);
		clReleaseMemObject(cl_r_r);
		clReleaseMemObject(cl_r_gradient);
		clReleaseMemObject(cl_match_result);
		clReleaseCommandQueue(queue);
		clReleaseContext(context);
		return 0;
	}

	cl_kernel matcher = clCreateKernel(program, "matching_cost", 0);
	if(matcher == 0) {
		std::cerr << "Can't load kernel\n";
		clReleaseProgram(program);
		clReleaseMemObject(cl_l_b);
		clReleaseMemObject(cl_l_g);
		clReleaseMemObject(cl_l_r);
		clReleaseMemObject(cl_l_gradient);
		clReleaseMemObject(cl_r_b);
		clReleaseMemObject(cl_r_g);
		clReleaseMemObject(cl_r_r);
		clReleaseMemObject(cl_r_gradient);
		clReleaseMemObject(cl_match_result);
		clReleaseCommandQueue(queue);
		clReleaseContext(context);
		return 0;
	}

	clSetKernelArg(matcher, 0, sizeof(cl_mem), &cl_l_b);
	clSetKernelArg(matcher, 1, sizeof(cl_mem), &cl_l_g);
	clSetKernelArg(matcher, 2, sizeof(cl_mem), &cl_l_r);
	clSetKernelArg(matcher, 3, sizeof(cl_mem), &cl_l_gradient);
	clSetKernelArg(matcher, 4, sizeof(cl_mem), &cl_r_b);
	clSetKernelArg(matcher, 5, sizeof(cl_mem), &cl_r_g);
	clSetKernelArg(matcher, 6, sizeof(cl_mem), &cl_r_r);
	clSetKernelArg(matcher, 7, sizeof(cl_mem), &cl_r_gradient);
	clSetKernelArg(matcher, 8, sizeof(cl_mem), &cl_match_result);

	size_t work_size = DATA_SIZE;
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
	clReleaseMemObject(cl_l_b);
	clReleaseMemObject(cl_l_g);
	clReleaseMemObject(cl_l_r);
	clReleaseMemObject(cl_l_gradient);
	clReleaseMemObject(cl_r_b);
	clReleaseMemObject(cl_r_g);
	clReleaseMemObject(cl_r_r);
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

	//Keep doing MST
	SGECostNode **cList = buildCostListFromCV(nodeList, left_gray_arr, w, h);
	addMSTEdgeToNodeList(nodeList, cList, IntensityLimit, w, h);
	//Aggregation
	ca->upwardAggregation(w/2, h/2, -1);

	ca->downwardAggregation(w/2, h/2, -1);

	ca->pickBestDepthForNodeList();

	//ca->showDisparityMap();
	cv::Mat dMap(h, w, CV_8U);
	for(int y=0 ; y<h ; y++) for(int x=0 ; x<w ; x++)
	{
		dMap.at<uchar>(y,x) = nodeList[y][x].dispairty * (double) IntensityLimit / (double)disparityLevel;
	}
	cv::imwrite("dMap.bmp", dMap);

	system("PAUSE");

	return 0;
}