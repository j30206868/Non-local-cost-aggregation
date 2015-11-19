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

__kernel void MedianFilterBitonic(const __global uint* pSrc, __global uint* pDst)
{
	const int x = get_global_id(0);
	const int y = get_global_id(1);
	
	const int width = get_global_size(0);
	const int iOffset = y * width;
	const int iPrev = iOffset - width;
	const int iNext = iOffset + width;

	uint uiRGBA[9];
			
	//get pixels within aperture
	uiRGBA[0] = pSrc[iPrev + x - 1];
	uiRGBA[1] = pSrc[iPrev + x];
	uiRGBA[2] = pSrc[iPrev + x + 1];

	uiRGBA[3] = pSrc[iOffset + x - 1];
	uiRGBA[4] = pSrc[iOffset + x];
	uiRGBA[5] = pSrc[iOffset + x + 1];

	uiRGBA[6] = pSrc[iNext + x - 1];
	uiRGBA[7] = pSrc[iNext + x];
	uiRGBA[8] = pSrc[iNext + x + 1];

	uint uiResult = 0;
	uint uiMask = 0xFF;

	for(int ch = 0; ch < 3; ch++)
	{

		//extract next color channel
		uint r0,r1,r2,r3,r4,r5,r6,r7,r8;
		r0=uiRGBA[0]& uiMask;
		r1=uiRGBA[1]& uiMask;
		r2=uiRGBA[2]& uiMask;
		r3=uiRGBA[3]& uiMask;
		r4=uiRGBA[4]& uiMask;
		r5=uiRGBA[5]& uiMask;
		r6=uiRGBA[6]& uiMask;
		r7=uiRGBA[7]& uiMask;
		r8=uiRGBA[8]& uiMask;
		
		//perform partial bitonic sort to find current channel median
		uint uiMin = min(r0, r1);
		uint uiMax = max(r0, r1);
		r0 = uiMin;
		r1 = uiMax;

		uiMin = min(r3, r2);
		uiMax = max(r3, r2);
		r3 = uiMin;
		r2 = uiMax;

		uiMin = min(r2, r0);
		uiMax = max(r2, r0);
		r2 = uiMin;
		r0 = uiMax;

		uiMin = min(r3, r1);
		uiMax = max(r3, r1);
		r3 = uiMin;
		r1 = uiMax;

		uiMin = min(r1, r0);
		uiMax = max(r1, r0);
		r1 = uiMin;
		r0 = uiMax;

		uiMin = min(r3, r2);
		uiMax = max(r3, r2);
		r3 = uiMin;
		r2 = uiMax;

		uiMin = min(r5, r4);
		uiMax = max(r5, r4);
		r5 = uiMin;
		r4 = uiMax;

		uiMin = min(r7, r8);
		uiMax = max(r7, r8);
		r7 = uiMin;
		r8 = uiMax;

		uiMin = min(r6, r8);
		uiMax = max(r6, r8);
		r6 = uiMin;
		r8 = uiMax;

		uiMin = min(r6, r7);
		uiMax = max(r6, r7);
		r6 = uiMin;
		r7 = uiMax;

		uiMin = min(r4, r8);
		uiMax = max(r4, r8);
		r4 = uiMin;
		r8 = uiMax;

		uiMin = min(r4, r6);
		uiMax = max(r4, r6);
		r4 = uiMin;
		r6 = uiMax;

		uiMin = min(r5, r7);
		uiMax = max(r5, r7);
		r5 = uiMin;
		r7 = uiMax;

		uiMin = min(r4, r5);
		uiMax = max(r4, r5);
		r4 = uiMin;
		r5 = uiMax;

		uiMin = min(r6, r7);
		uiMax = max(r6, r7);
		r6 = uiMin;
		r7 = uiMax;

		uiMin = min(r0, r8);
		uiMax = max(r0, r8);
		r0 = uiMin;
		r8 = uiMax;

		r4 = max(r0, r4);
		r5 = max(r1, r5);

		r6 = max(r2, r6);
		r7 = max(r3, r7);

		r4 = min(r4, r6);
		r5 = min(r5, r7);

		//store found median into result
		uiResult |= min(r4, r5);

		//update channel mask
		uiMask <<= 8;
	}

	//store result into memory
	pDst[iOffset + x] = uiResult;
}

__kernel void MedianFilterGrayScale(const __global uchar* pSrc, __global uchar* pDst)
{
	const int x = get_global_id(0);
	const int y = get_global_id(1);
	
	const int width = get_global_size(0);
	const int iOffset = y * width;
	const int iPrev = iOffset - width;
	const int iNext = iOffset + width;

	//get pixels within aperture
	uchar r[9];
	r[0] = pSrc[iPrev + x - 1];
	r[1] = pSrc[iPrev + x];
	r[2] = pSrc[iPrev + x + 1];

	r[3] = pSrc[iOffset + x - 1];
	r[4] = pSrc[iOffset + x];
	r[5] = pSrc[iOffset + x + 1];

	r[6] = pSrc[iNext + x - 1];
	r[7] = pSrc[iNext + x];
	r[8] = pSrc[iNext + x + 1];
	
	uchar tmp;
	for(int i=0 ; i< 5 ; i++){
		for(int j=i+1 ; j<9 ; j++){
			if(r[i] > r[j]){
				tmp = r[i];
				r[i] = r[j];
				r[j] = tmp;
			}
		}
	}
	/*pDst[iOffset + x] = r[5];*/
}