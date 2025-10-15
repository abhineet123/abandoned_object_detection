#ifndef _COMBINE_IMG_
#define _COMBINE_IMG_

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#define HORIZONTAL_JOIN 0
#define VERTICAL_JOIN 1
#define INF 99999


struct intensity_struct{

	int max_intensity;
	int min_intensity;
};
IplImage* initializeCombinedImage(CvSize img1_size,CvSize img2_size,int nchannels,int combine_method);
void combineImages(IplImage* img1,IplImage* img2,IplImage* combined_img,int combine_method);
void covertGrayscaleToRGB(IplImage *img_gray,IplImage *img_rgb);
void getIntensityRange(IplImage *img_gray,CvSize image_size,int &min_val,int &max_val);

#endif
