//__constant float color_ratio       = 0.11;
__constant float color_ratio       = 0.11;
__constant float gradient_ratio    = 0.89;
__constant float max_color_cost    = 7.0;
__constant float max_gradient_cost = 2.0;
__constant int max_disparity = 100;
__constant int width  = 640;
__constant float max_total_cost = 2.55;

__kernel void matching_cost(__global const uchar* l_b, __global const uchar* l_g,  __global const uchar *l_r, __global const float *l_gradient, 
							__global const uchar* r_b, __global const uchar* r_g,  __global const uchar *r_r, __global const float *r_gradient, 
							__global float* result)
{
	// 450 -> width of image
	// 60  -> max_disparity

	int idx = get_global_id(0);
	int x = idx % width;
	int resultIdx = idx * max_disparity;
	int maxD = fmin((float)(width - x), max_disparity);

	for(int d = 0 ; d < maxD ; d++){
		int ridx = idx+d;
		float color_cost = fabs( (float)(l_b[idx] - r_b[ridx]) ) +
						   fabs( (float)(l_g[idx] - r_g[ridx]) ) +
						   fabs( (float)(l_r[idx] - r_r[ridx]) );

		color_cost = fmin(color_cost/3, max_color_cost);

		float gradient_cost = fmin( fabs(l_gradient[idx] - r_gradient[idx]), max_gradient_cost);

		result[resultIdx+d] = color_cost*color_ratio + gradient_cost*gradient_ratio;
	}
	for(int d = maxD ; d < max_disparity ; d++){
		result[resultIdx+d] = max_total_cost;
	}
}


/*
__constant float color_ratio       = 0.036667;
__constant float gradient_ratio    = 0.89;
__constant float max_color_cost    = 21.0;
__constant float max_gradient_cost = 2.0;

__kernel void matching_cost(__global const uchar* l_b, __global const uchar* l_g,  __global const uchar *l_r, __global const float *l_gradient, 
							__global const uchar* r_b, __global const uchar* r_g,  __global const uchar *r_r, __global const float *r_gradient, 
							__global float* result)
{
	// 450 -> width of image
	// 60  -> max_disparity

	int idx = get_global_id(0);
	int x = idx % 450;
	int resultIdx = idx * 60;
	int maxD = 450 - x;
	if( maxD > 60 ){
		maxD = 60;
	}

	for(int d = 0 ; d < maxD ; d++){
		int ridx = idx+d;
		float color_cost = fabs( (float)(l_b[idx] - r_b[ridx]) ) +
						   fabs( (float)(l_g[idx] - r_g[ridx]) ) +
						   fabs( (float)(l_r[idx] - r_r[ridx]) );

		color_cost = fmin(color_cost, max_color_cost);

		float gradient_cost = fmin( fabs(l_gradient[idx] - r_gradient[idx]), max_gradient_cost);

		result[resultIdx+d] = color_cost*color_ratio + gradient_cost*gradient_ratio;
	}
	for(int d = maxD ; d < 60 ; d++){
		result[resultIdx+d] = 0;
	}
}
*/