#ifndef CWZ_CL_CPP_FUNCTIONS_H
#define CWZ_CL_CPP_FUNCTIONS_H

#include "common_func.h"

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
int apply_cl_cost_match(cl_context &context, cl_device_id &device, cl_program &program, cl_int &err, 
						cl_match_elem *left_cwz_img, cl_match_elem *right_cwz_img, float *matching_result, int h, int w, int match_result_len){
	cl_kernel matcher = clCreateKernel(program, "matching_cost", 0);
	if(matcher == 0) { std::cerr << "Can't load matching_cost kernel\n"; return 0; }

	match_info info;
	info.img_height = h; info.img_width = w; info.max_d = disparityLevel;

	time_t step_up_kernel_s = clock();

	cl_mem cl_l_rgb = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * left_cwz_img->node_c, &left_cwz_img->rgb[0], NULL);
	cl_mem cl_l_gradient = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * left_cwz_img->node_c, &left_cwz_img->gradient[0], NULL);

	cl_mem cl_r_rgb = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * right_cwz_img->node_c, &right_cwz_img->rgb[0], NULL);
	cl_mem cl_r_gradient = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * right_cwz_img->node_c, &right_cwz_img->gradient[0], NULL);

	cl_mem cl_match_result = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * match_result_len, NULL, NULL);

	cl_mem cl_match_info = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(match_info), &info, NULL);

	if(cl_l_rgb == 0 || cl_l_gradient == 0 ||
	   cl_r_rgb == 0 || cl_r_gradient == 0 ||
	   cl_match_result == 0 || cl_match_info == 0) {
		std::cerr << "Can't create OpenCL buffer\n";
		clReleaseKernel(matcher);
		clReleaseMemObject(cl_l_rgb);
		clReleaseMemObject(cl_l_gradient);
		clReleaseMemObject(cl_r_rgb);
		clReleaseMemObject(cl_r_gradient);
		clReleaseMemObject(cl_match_result);
		clReleaseMemObject(cl_match_info);
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
		clReleaseMemObject(cl_l_rgb);
		clReleaseMemObject(cl_l_gradient);
		clReleaseMemObject(cl_r_rgb);
		clReleaseMemObject(cl_r_gradient);
		clReleaseMemObject(cl_match_result);
		clReleaseMemObject(cl_match_info);
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
	clReleaseMemObject(cl_l_rgb);
	clReleaseMemObject(cl_l_gradient);
	clReleaseMemObject(cl_r_rgb);
	clReleaseMemObject(cl_r_gradient);
	clReleaseMemObject(cl_match_result);
	clReleaseCommandQueue(queue);
	clReleaseMemObject(cl_match_info);
	
	return 1;
}

template<class T>
T *apply_cl_color_img_mdf(cl_context &context, cl_device_id &device, cl_program &program, cl_int &err,
						   T *color_1d_arr, int node_c, int h, int w, bool apply_median_filter = true)
{
	if(!apply_median_filter){
		return color_1d_arr;
	}
							   
	cl_kernel mdf_kernel;

	match_info info;
	info.img_height = h; info.img_width = w; info.max_d = disparityLevel;

	if(eqTypes<int, T>()){
		mdf_kernel = clCreateKernel(program, "MedianFilterBitonic", 0);
		if(mdf_kernel == 0) { std::cerr << "Can't load MedianFilterBitonic kernel\n"; return 0; }
	}else if(eqTypes<uchar, T>()){
		mdf_kernel = clCreateKernel(program, "MedianFilterGrayScale", 0);
		if(mdf_kernel == 0) { std::cerr << "Can't load MedianFilterGrayScale kernel\n"; return 0; }
	}else{
		return 0;
	}
	cl_mem cl_arr = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(T) * node_c, color_1d_arr, NULL);
	cl_mem cl_result = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(T) * node_c, NULL, NULL);
	cl_mem cl_match_info = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(match_info), &info, NULL);

	if(cl_arr == 0 || cl_result == 0 || cl_match_info == 0) {
		std::cerr << "Can't create OpenCL buffer for median filter\n";
		clReleaseKernel(mdf_kernel);
		clReleaseMemObject(cl_arr);
		clReleaseMemObject(cl_result);
		clReleaseMemObject(cl_match_info);
		return 0;
	}

	clSetKernelArg(mdf_kernel, 0, sizeof(cl_mem), &cl_arr);
	clSetKernelArg(mdf_kernel, 1, sizeof(cl_mem), &cl_result);
	clSetKernelArg(mdf_kernel, 2, sizeof(cl_mem), &cl_match_info);

	cl_command_queue queue = clCreateCommandQueue(context, device, 0, 0);
	if(queue == 0) {
		std::cerr << "Can't create command queue for median filter\n";
		clReleaseKernel(mdf_kernel);
		clReleaseMemObject(cl_arr);
		clReleaseMemObject(cl_result);
		clReleaseMemObject(cl_match_info);
		return 0;
	}

	T *result_arr = new T[node_c];

	size_t offset_size[2] = {0,0};
	size_t work_size[2] = {w, h};
	clock_t tOfCLStart = clock();
    /* Do your stuff here */
	err = clEnqueueNDRangeKernel(queue, mdf_kernel, 2, offset_size, work_size, 0, 0, 0, 0);

	if(err == CL_SUCCESS) {
		err = clEnqueueReadBuffer(queue, cl_result, CL_TRUE, 0, sizeof(T) * node_c, &result_arr[0], 0, 0, 0);
	}
	printf("MDF Time taken: %fs\n", (double)(clock() - tOfCLStart)/CLOCKS_PER_SEC);

	if(err != CL_SUCCESS)  {
		std::cout << getErrorString(err) << std::endl;
		std::cerr << "Can't run kernel or read back data for median filter\n";
		delete[] result_arr;
		clReleaseKernel(mdf_kernel);
		clReleaseMemObject(cl_arr);
		clReleaseMemObject(cl_result);
		clReleaseCommandQueue(queue);
		clReleaseMemObject(cl_match_info);
		return 0;
	}

	clReleaseKernel(mdf_kernel);
	clReleaseMemObject(cl_arr);
	clReleaseMemObject(cl_result);
	clReleaseCommandQueue(queue);
	clReleaseMemObject(cl_match_info);

	return result_arr;
}

#endif