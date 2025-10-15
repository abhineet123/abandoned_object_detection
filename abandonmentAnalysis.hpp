#ifndef _ABANDONMENT_
#define _ABANDONMENT_

#define EDGE_ENERGY 0
#define REGION_GROWING 1
#define INWARD_GROWING 0
#define OUTWARD_GROWING 1
#define UPWARD_GROWING 2
#define DOWNWARD_GROWING 3
#define REGION_COUNT 4
#define USE_HORZ 1
#define USE_VERT 1

#define SHOW_CANNY 0

#define DISABLE_ABND 0
#define ABND_GRADIENT_EDGE_DETECTION 1
#define ABND_CANNY_EDGE_DETECTION 2
#define ABND_REGION_GROWING 3

#define REGION_SIMILARITY 0
#define PIXEL_SIMILARITY 1
#define PIXEL_COMPARISON 2

#define MIN_INTENSITY 200
#define SHOW_STATUS_ABND 0

#define BACKGROUND_COLOR 0
#define FOREGROUND_COLOR 255

#define MAX_NAME 100

#include "BlobResult.h"
#include "Image.hpp"
#include "runningStat.hpp"
#include "foregroundProcessing.hpp"
#include "combineImages.hpp"

#include <vector>

using namespace std;

struct abandonment_struct{

	int abandonment_method;
	int similarity_method;
	int edge_diff_threshold_percent;
	int region_growing_threshold_percent;
	int region_diff_threshold_percent;

	int detect_still_person;
	int max_avg_diff_10;	

	double edge_diff_threshold;
	double region_growing_threshold;
	double region_diff_threshold;
	double max_avg_diff;
	int max_squared_diff;
	int erosion_width;

	int canny_low_thr;
	int canny_ratio;
	int canny_high_thr;
	int show_current;

	int gradient_min_thr;
};

struct horz_extent {
	int y;
	int min_x;
	int max_x;
};

struct vert_extent {
	int x;
	int min_y;
	int max_y;
};

struct rect_struct{
	int line_thickness;
	int line_type;
	int shift;
};

struct seed_point{
	int x;
	int y;
	int vert_growing;
	int horz_growing;

	seed_point(int init_x,int init_y){
		x=init_x;
		y=init_y;
		vert_growing=0;
		horz_growing=0;
	}
};


class AbandonmentAnalysis {

public:
	AbandonmentAnalysis(CvSize image_size,abandonment_struct *abandonment_params_init);
	~AbandonmentAnalysis();

	void initStateVariables(CvSize image_size);
	void clearStateVariables();

	inline void getThresholdedGradientImages(img_grad* frg_grad,img_grad *bkg_grad);
	void  getCombinedThresholdedGradientImage(IplImage *grad_combined, IplImage *grad_x,IplImage *grad_y);
	int getEdgePixelCount(CvSeq *point_set,IplImage *img);
	inline void clearContourImage();	
	inline int  getTotalPointCount();
	void showGradientImagesForAbandonedBlobs(CBlobResult &blobs,ForegroundProc *frg_obj,IplImage *temp_rgb_img,IplImage *temp_gray_img);

	void getSortedContourPoints(CvSeq *contour_points);

	bool (AbandonmentAnalysis::*evaluateSimilarity)(RgbImage &img,CvPoint pt1,int point_set_id);
	bool evaluatePixellSimilarity(RgbImage &img,CvPoint pt1,int point_set_id);
	bool comparePixellSimilarity(RgbImage &img,CvPoint pt1,int point_set_id);
	bool evaluateRegionSimilarity(RgbImage &img,CvPoint pt1,int point_set_id);

	void drawOriginalContour(CBlobResult *blobs,CvScalar color);
	void drawErodedContour(CBlobResult *blobs,CvScalar color);
	void testStateVariables(CvSeq *contour_points);
	void drawContour(int show_all,CvScalar color);
	void drawPoints(int show_all,CvScalar color,int point_set_id);
	void testRegionGrowing(RgbImage &img,CvSize image_size,CBlobResult *blobs);	

	void getErodedContour();
	void getVerticalErodedContour();
	void getHorizontalErodedContour();

	void getContourExtents();
	void getVerticalContourExtents(int no_of_points);
	void getHorizontalContourExtents(int no_of_points);

	void initializeSeedPoints(RgbImage &img);
	void initializeVerticalSeedPoints(RgbImage &img);
	void initializeHorizontalSeedPoints(RgbImage &img);

	void applyRegionGrowing(RgbImage &img,CvSize image_size);
	void applyVerticalRegionGrowing(RgbImage &img,CvSize image_size);
	void applyHorizontalRegionGrowing(RgbImage &img,CvSize image_size);

	int evaluateRegionGrowingPixelCount(CBlob *blob,RgbImage &frg_img,RgbImage &bkg_img,CvSize image_size);
	int evaluateGradientEdgePixelCount(CBlob *blob,img_grad* frg_grad,img_grad *bkg_grad);
	int evaluateCannyEdgePixelCount(CBlob *blob);	

	bool detectRemovedObjectsUsingRegionGrowing(CBlobResult *tracked_blobs,RgbImage &frg_img,RgbImage &bkg_img,CvSize image_size);
	bool detectRemovedObjectsUsingGradientEdgeDetection(CBlobResult *tracked_blobs,img_grad* frg_grad,img_grad *bkg_grad);
	bool detectRemovedObjectsUsingCannyEdgeDetection(CBlobResult *tracked_blobs,IplImage *frg_img,IplImage *bkg_img);

	void initParams(abandonment_struct *abandonment_params_init);
	static void updateParams(int);
	static void updateAbandonmentMethod(int);
	static void updateSimilarityMethod(int);
	static void printParams();
	void initWindow(char *window_name);

	inline bool getAdjacentPoint(CvPoint pt1,CvPoint *pt2,int point_set_id);

	void getBoundedBox(CvPoint *min_point,CvPoint *max_point);
	void getBoundingBox(CvPoint *min_point,CvPoint *max_point);
	void getErodedBox(CvPoint *min_point,CvPoint *max_point,IplImage *img);
	void getDilatedBox(CvPoint *min_point,CvPoint *max_point,IplImage *img);
	void showBoxes(CBlobResult *blobs,int line_thickness,int line_type, int shift);

	void drawBlobBoundary(CBlob *current_blob,CvScalar color);
	void drawErodedContour2(CBlobResult *blobs);

	void clearSeedPoints();
	void getBlobSeedPoints(CBlob *blob);
	void getGrowingDirection(seed_point *root_point);

	bool evaluatePointSimilarity(IplImage *img,CvPoint *pt1,CvPoint *pt2);
	void performRegionGrowing(IplImage *img);

	void displayRegionPoints(IplImage *img);

	static void showAbandonmentHelpWindow(int mouse_event,int x,int y,int flags,void* param);

	//void getRegionMeanY(BwImage &frg_gray);

	CvSeq *sorted_by_x;
	CvSeq *sorted_by_y;
	CvMemStorage *storage_x;
	CvMemStorage *storage_y;

	vector<CvPoint*> inward_growing_points;
	vector<CvPoint*> outward_growing_points;
	vector<CvPoint*> upward_growing_points;
	vector<CvPoint*> downward_growing_points;

	int horz_seed_count;
	int vert_seed_count;	

	vector<Pixel_t*> point_set_mean;
	vector<Pixel_t*> point_set_std;

	vector<horz_extent*> contour_horz_extent;
	vector<horz_extent*> eroded_contour_horz_extent;  
	vector<vert_extent*> contour_vert_extent;
	vector<vert_extent*> eroded_contour_vert_extent;

	RunningStatVector *in_obj,*out_obj;	
	RunningStatVector *up_obj,*down_obj;	

	RgbImage contour_image;	
	BwImage temp_gray_img;
	RgbImage temp_rgb_img;

	bool state_variables_initialized;

	static abandonment_struct *params;

	IplImage *region_growing_bkg;
	IplImage *region_growing_frg;

	IplImage *canny_bkg;
	IplImage *canny_frg;

	IplImage *frg_grad_x;
	IplImage *frg_grad_y;
	IplImage *frg_grad_combined;

	IplImage *bkg_grad_x;
	IplImage *bkg_grad_y;
	IplImage *bkg_grad_combined;

	IplImage *blob_image;
	IplImage *boundary_image;

	CBlobResult *eroded_blobs;
	vector<seed_point*> blob_seed_points;

	CvSize proc_size;
	static bool abandonment_updated;

	static char *abandonment_help_window;
	static IplImage *abandonment_help_image;
	
};


#endif


