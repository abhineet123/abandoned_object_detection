#ifndef _PRE_PROCESS_
#define _PRE_PROCESS_

#include <cv.h>
#include <highgui.h>
//#include <imgproc.h>

#include<stdio.h>
#include<conio.h>

#define MAX_INTENSITY 255
#define HIST_HEIGHT 800

#define IMG_MERGE 0
#define IMG_SPLIT 1

#define DISABLE_NOISE_REDUCTION 0
#define GAUSSIAN_FILTER 1
#define MEDIAN_FILTER 2
#define BILATERAL_FILTER 3
#define LINEAR_FILTER 4

#define DISABLE_CONTRAST_ENHANCEMENT 0
#define EQUALIZE_HISTOGRAM 1
#define NORMALIZE_HISTOGRAM 2
#define NORMALIZE_IMAGE 3
#define FILTER_IMAGE 4

#define USE_DIRECT_FILTERING 0
#define SHOW_PROC_IMG 0

#define MAX_NAME 100

using namespace std;
using namespace cv;

struct pre_process_struct{	

	int noise_reduction_method;
	int contrast_enhancement_method; 
	int show_histogram;

	int kernel_width_id;
	int kernel_height_id;

	int kernel_width;
	int kernel_height;

	int use_square_kernel;

	int cutoff_percent;
	double cutoff_fraction;
};

class PreProcessing{


public:
	PreProcessing(CvSize img_size,pre_process_struct *params_init);
	~PreProcessing();

	void initStateVariables(CvSize img_size);
	void clearStateVariables();

	void printKernel(CvMat *kernel);
	void splitOrMerge(IplImage *img_rgb,int op_flag);
	void equalizeHistogramRGB(IplImage *img);
	void normalizeHistogramRGB(IplImage *img);
	void filterImageRGB(IplImage *img);

	void getCutoffPixelValues(unsigned char &min_cutoff,unsigned char &max_cutoff);
	void removeCutoffIntensities(IplImage *img,unsigned char min_intensity,unsigned char max_intensity);
	void normalizeImageGS(IplImage *img);
	void normalizeImageRGB(IplImage *img);	

	void performContrastEnhancement(IplImage *img);
	void performNoiseReduction(IplImage *img);

	static void printParames();
	static void updateParams(int);
	static void updateNoiseReductionMethod(int);
	static void updateContrastEnhancementMethod(int);

	/*
	void printParames();
	static void updateParams(int val,void *param);
	static void updateNoiseReductionMethod(int val,void *param);
	static void updateContrastEnhancementMethod(int val,void *param);
	*/

	void initParams(pre_process_struct *params_init);
	void initWindow(char *window_name);

	int getIntensityCount(IplImage *img,CvSize image_size);

	void normalizeIntensityCount(int total_count,int max_count);
	void drawCountGraph();
	void getImageHistogram(IplImage *img,CvSize img_size,char *window_name);

	static void showPreProcessHelpWindow(int mouse_event,int x,int y,int flags,void* param);

	IplImage *img_red;
	IplImage *img_green;
	IplImage *img_blue;
	IplImage *img_hist;
	IplImage *img_gray;

	IplImage *temp_filter_img;

	CvSize stored_img_size;

	static pre_process_struct *params;
	static bool noise_reduction_updated;
	static bool contrast_enhancement_updated;

	CvMat *contrast_enhancement_kernel;

	bool state_variables_initialized;

	int total_pixel_count;
	int *intensity_count;

	static char *pre_process_help_window;
	static IplImage *pre_process_help_image;

};

#endif