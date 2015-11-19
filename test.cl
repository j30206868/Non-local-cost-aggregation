//__constant float color_ratio       = 0.11;
__constant float color_ratio       = 0.11;
__constant float gradient_ratio    = 0.89;
__constant float max_color_cost    = 7.0;
__constant float max_gradient_cost = 2.0;

__constant int mask_b = 0xFF;
__constant int mask_g = 0xFF00;
__constant int mask_r = 0xFF0000;

typedef struct {
	int max_d;
	int img_width;
} match_info;

__kernel void matching_cost(__global const int* l_rgb, __global const float *l_gradient, 
							__global const int* r_rgb, __global const float *r_gradient, 
							__global float* result, __global match_info *info)
{
	// 450 -> width of image
	// 60  -> max_disparity

	const int idx = get_global_id(0);
	const int x = idx % info->img_width;
	const int resultIdx = idx * info->max_d;
	const int sD = fmin((float)x, info->max_d-1);
	float color_cost;
	float gradient_cost;
	int ridx;
	int d;
	for(d = sD ; d >=0 ; d--){
		ridx = idx-d;
		color_cost = abs(  ((l_rgb[idx]&mask_b) - (r_rgb[ridx]&mask_b))      ) +
					 abs( (((l_rgb[idx]&mask_g) - (r_rgb[ridx]&mask_g)) >> 8)) +
					 abs( (((l_rgb[idx]&mask_r) - (r_rgb[ridx]&mask_r)) >> 16));

		color_cost = fmin(color_cost/3.0, max_color_cost);

		gradient_cost = fmin( fabs(l_gradient[idx] - r_gradient[ridx]), max_gradient_cost);

		result[resultIdx+d] = color_cost*color_ratio + gradient_cost*gradient_ratio;
	}

	ridx = idx-x;
	for(d = info->max_d-1 ; d > sD ; d--){
		color_cost = abs(  ((l_rgb[idx]&mask_b) - (r_rgb[ridx]&mask_b))      ) +
					 abs( (((l_rgb[idx]&mask_g) - (r_rgb[ridx]&mask_g)) >> 8)) +
					 abs( (((l_rgb[idx]&mask_r) - (r_rgb[ridx]&mask_r)) >> 16));

		color_cost = fmin(color_cost/3, max_color_cost);

		gradient_cost = fmin( fabs(l_gradient[idx] - r_gradient[ridx]), max_gradient_cost);

		result[resultIdx+d] = color_cost*color_ratio + gradient_cost*gradient_ratio;
	}
}

