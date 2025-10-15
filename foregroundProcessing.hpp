#ifndef _FORE_PROC_
#define _FORE_PROC_

#include "imgproc\imgproc.hpp"
#include "highgui\highgui.hpp"
#include<stdio.h>
#include<conio.h>
#include <cv.h>
#include <cxcore.h>
#include <vector>
#include <fstream>
#include "Image.hpp"
#include "highgui.h"
#include "runningStat.hpp"

#define FOREGROUND_COLOR 255
#define BACKGROUND_COLOR 0

#define NO_SHADOW_DETECTION 0
#define SIMPLE_NCC 1
#define COMPLEX_NCC 2

#define MAX_NAME 100

using namespace cv;
using namespace std;

typedef vector<double> double_vector;

struct frg_struct{

	int perform_foreground_analysis;
	int shadow_detection_method;
	int shadow_refinement;

	int complex_ncc_threshold_percent;
	int simple_ncc_threshold_percent;
	int frg_similarity_threshold_percent;	
	int min_intensity_ratio_percent;
	int intensity_ratio_std_threshold_percent;

	double frg_similarity_threshold;
	double complex_ncc_threshold;
	double simple_ncc_threshold;
	int min_frame_intensity;
	double min_intensity_ratio;
	double intensity_ratio_std_threshold;
	double intensity_ratio_var_threshold;	

	double complex_ncc_threshold_sqr;
	double simple_ncc_threshold_sqr;

	CvSize neighbourhood_size;
};

struct morph_struct{
	int performMorphologicalOp;
	int performOpening;
	int performClosing;
	int no_of_iterations;
};


struct img_grad{	
	BwImage grad_overall;
	BwImage grad_magnitude;
	BwImage grad_x, grad_y;
	BwImage abs_grad_x, abs_grad_y;
};


struct grad_params{
	int ddepth;
	double scale;
	double delta;
};

struct CvWindow{
	int min_row;
	int max_row;
	int min_col;
	int max_col;	
};



typedef vector<vector<CvWindow*>> CvWindowMat;

class ForegroundProc{	

public:

	/***************************Non Static Members*********************************************/

	ForegroundProc(CvSize image_size,frg_struct *frg_params_init,morph_struct *morph_params_init);
	~ForegroundProc();

	void initStateVariables(CvSize image_size);
	void clearStateVariables();
	void clearWindowExtentForImage(CvWindowMat &window_extents);
	CvWindow* getWindowExtentForPixel(CvSize window_size,int r,int c,CvSize image_size);
	void getWindowExtentForImage(CvWindowMat &window_extents,CvSize window_size,CvSize image_size);


	void removeFalseForeground(IplImage* bkg_img,IplImage* frg_img,BwImage& frg_mask);

	bool detectLightingChange(int r,int c);

	bool (ForegroundProc::*isShadow)(int,int);
	bool detectShadowUsingComplexNCC(int,int);
	bool detectShadowUsingSimpleNCC(int,int);		

	double getComplexNCC(int r,int c);
	bool isShadowCandidate(int r,int c);
	bool detectFalseShadow(int r,int c);

	void initParams(frg_struct *frg_params_init);
	void initMorphParams(morph_struct *morph_params_init);	

	void initWindow(char *window_name);
	void initMorphWindow(char *window_name);

	static void showForegroundProcHelpWindow(int mouse_event,int x,int y,int flags,void* param);
	static void showForegroundToggleHelpWindow(int mouse_event,int x,int y,int flags,void* param);

	CvWindowMat img_window_extents;	

	CvWindowMat shadow_window_extents;	

	BwImage bkg_gray,frg_gray;	

	img_grad *bkg_grad,*frg_grad;

	RunningStatScalar *stat_obj;	

	bool state_variables_initialized;

	/***************************Static Members*********************************************/

	static void updateParams(int x=0);
	static void updateShadowMethod(int x=0);

	static void printProcParams();	
	static void printToggleParams(int x=0);
	static void printMorphParams(int x=0);

	static frg_struct *params;	
	static bool frg_updated;
	static morph_struct *morph_params;
	static bool shadow_method_updated;

	static char *frg_proc_help_window;
	static char *frg_toggle_help_window;

	static IplImage *frg_proc_help_image;
	static IplImage *frg_toggle_help_image;
};


#endif