//__constant float color_ratio       = 0.11;
__constant float color_ratio       = 0.11;
__constant float gradient_ratio    = 0.89;
__constant float max_color_cost    = 7.0;
__constant float max_gradient_cost = 2.0;

typedef struct {
	int max_d;
	int img_width;
} match_info;

__kernel void matching_cost(__global const uchar* l_b, __global const uchar* l_g,  __global const uchar *l_r, __global const float *l_gradient, 
							__global const uchar* r_b, __global const uchar* r_g,  __global const uchar *r_r, __global const float *r_gradient, 
							__global float* result, __global match_info *info)
{
	// 450 -> width of image
	// 60  -> max_disparity

	const int idx = get_global_id(0);
	const int x = idx % info->img_width;
	const int resultIdx = idx * info->max_d;
	const int sD = fmin((float)x, info->max_d-1);

	for(int d = sD ; d >=0 ; d--){
		int ridx = idx-d;
		float color_cost = fabs( (float)(l_b[idx] - r_b[ridx]) ) +
						   fabs( (float)(l_g[idx] - r_g[ridx]) ) +
						   fabs( (float)(l_r[idx] - r_r[ridx]) );

		color_cost = fmin(color_cost/3.0, max_color_cost);

		float gradient_cost = fmin( fabs(l_gradient[idx] - r_gradient[ridx]), max_gradient_cost);

		result[resultIdx+d] = color_cost*color_ratio + gradient_cost*gradient_ratio;
	}
	const int ridx = idx-x;
	for(int d = info->max_d-1 ; d > sD ; d--){
		//int ridx = idx + width - x - 1;
		float color_cost = fabs( (float)(l_b[idx] - r_b[ridx]) ) +
						   fabs( (float)(l_g[idx] - r_g[ridx]) ) +
						   fabs( (float)(l_r[idx] - r_r[ridx]) );

		color_cost = fmin(color_cost/3, max_color_cost);

		float gradient_cost = fmin( fabs(l_gradient[idx] - r_gradient[ridx]), max_gradient_cost);

		result[resultIdx+d] = color_cost*color_ratio + gradient_cost*gradient_ratio;
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