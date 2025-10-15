#ifndef _BLOB_TRACK_
#define _BLOB_TRACK_

#include<vector>
#include<stdio.h>
#include<conio.h>
#include <sstream>
#include <string>

#include "BlobResult.h"
#include "GrimsonGMM.hpp"

#define FOREGROUND_COLOR 255
#define BLOB_AREA 0
#define BOUNDING_BOX_AREA 1

#define DIFF_RGB 0
#define DIFF_GRAY 1

#define NO_GRAD 0
#define GRAD_X 1
#define GRAD_Y 2

#define OBJ_ALL 0
#define OBJ_STATIC 1
#define OBJ_ABANDONED 2
#define OBJ_REMOVED 3

#define AREA_CURRENT 0
#define AREA_RUNNING_AVG 1
#define AREA_MOVING_AVG 2

#define SHOW_TRACKING_STATUS 0

#define MAX_NAME 100


using namespace std;

typedef std::vector<int> int_vector;
typedef std::vector<double> double_mat1d;
typedef std::vector<double_mat1d> double_mat2d;

struct match_struct{

	int occ_threshold_percent;
	int size_threshold_percent;
	int dist_threshold_percent;
	int appearance_threshold_percent;
	int moving_avg_alpha_percent;

	double occ_threshold;
	double size_threshold;
	double dist_threshold;
	double appearance_threshold;
	double moving_avg_alpha;
	double max_blob_dist;

	double dist_threshold_sqr;

	int blob_distance_measure;
	int blob_area_measure;
	int blob_area_type;

	int use_gradient_diff;
};

struct track_struct{

	int max_mean_diff_10;
	double max_mean_diff;

	int min_hit_count_for_occ; 
	int min_hit_count_for_abandoned;
	int min_hit_count_for_static;
	int max_miss_count;
	int max_occ_count;

	int max_removed_count;
	int max_abandoned_count;

	int static_factor_occ;
	int static_factor_miss;
};

class BlobTracking{
public:

	BlobTracking(CvSize image_size,match_struct *matching_params_init,track_struct *tracking_params_init);	
	~BlobTracking();

	void initStateVariables(CvSize image_size);
	void clearStateVariables();

	inline bool matchBlobs(CBlob *blob1,CBlob *blob2,double blob_dist);	
	inline bool matchBlobSizes(CBlob *blob1,CBlob *blob2);
	inline bool matchBlobLocations(int blob1_index,int blob2_index);
	inline bool matchBlobAppearances(CBlob *blob1,CBlob *blob2);

	inline void initializeBlob(CBlob *blob);
	inline void initializeBlobProperties(CBlob *current_blob,IplImage *frg_image,IplImage *frg_grad_x,IplImage *frg_grad_y);
	inline void clearBlob(CBlob *current_blob);	
	void refreshBlobImages(CBlob *current_blob,IplImage *frg_image,IplImage *frg_grad_x,IplImage *frg_grad_y);
	void refreshTrackedBlobsImages(IplImage *frg_image,IplImage *frg_grad_x,IplImage *frg_grad_y);
	
	void updateTrackedBlobs(IplImage *frg_image,CBlobResult *frg_blobs,int frg_blob_count,BwImage &frg_mask,IplImage *frg_grad_x=NULL,IplImage *frg_grad_y=NULL);
	bool processTrackedBlobs(IplImage *frg_image,CBlobResult *frg_blobs,int frg_blob_count,IplImage *frg_grad_x=NULL,IplImage *frg_grad_y=NULL);	
	void updateBlobImage(IplImage *frg_image,int line_thickness,int line_type, int shift);

	inline void clearBlobDistances();

	bool isOccluded(CBlob *blob,BwImage &frg_mask);	

	void updateTrackedStaticMask();			

	void updateBlobDistances(CBlobResult *frg_blobs);
	void updateBlobAreas(CBlobResult *frg_blobs);
	inline void updateExistingBlobAreas();
	inline void updateExistingBlobDiff();

	void getCorrespondingBlobs(CBlobResult *current_blobs);
	void addBlobsForTracking(CBlobResult *current_blobs,int_vector indexes);	

	void fillBoundingBox(BwImage &img,CBlob *blob);

	void showCorrespondenceImage(CBlobResult *frg_blobs,int line_thickness,int line_type, int shift);	
	void showBlobs(IplImage *img,CBlobResult *blobs,CvScalar blob_color);
	void printBlobDistances();
	void displayBlobsAndDistances(RgbImage &frg_img,CBlobResult *frg_blobs,CvScalar frg_color,CvScalar tracked_color);

	void getBlobMeanStd(CBlobResult *frg_blobs,int frg_blob_count,RgbImage &frg_image);	

	void printBlobAreas(CBlobResult *blobs,int no_of_blobs);

	void updateMovingAverageOfDifference(CBlob *blob,IplImage *frg_image);
	void updateMovingAverageOfDifference(CBlob *blob,IplImage *frg_grad_x,IplImage *frg_grad_y);
	void updateMovingAverageOfAreas(CBlob *blob1,CBlob *blob2);

	double getMeanDifference(CBlob *blob,IplImage *frg_image,int diff_img);

	void updateBoundingBoxMask(CBlobResult *frg_blobs,int frg_blob_count);

	void eliminateRemovedBlobs();
	void eliminateAbandonedBlobs();
	void eliminateStaticBlobs();
	void eliminateStillPersonBlobs();

	static void updateParamsMatch(int);
	static void updateParamsTrack(int);
	static void updateStaticFactor(int);
	static void updateAreaMethod(int);
	static void updateDistanceMethod(int);
	static void updateDiffMethod(int);

	static void showTrackHelpWindow(int mouse_event,int x,int y,int flags,void* param);
	static void showMatchHelpWindow(int mouse_event,int x,int y,int flags,void* param);

	static void printTrackingParameters();
	static void printMatchingParameters();

	void initParamsMatch(match_struct *matching_params_init);
	void initParamsTrack(track_struct *tracking_params_init);

	void initWindowTrack(char *window_name);
	void initWindowMatch(char *window_name);

	void showBlob(IplImage *img,CBlob *blob,CvScalar &blob_col);
	int showBoundingBoxes(IplImage *img,int object_type);
	int getSelectionImage(IplImage *current_frame,IplImage *selection_image,int object_type,int resize_image);

	inline bool isContainedInsideBlob(CvPoint point,CBlob *blob);
    CBlob* getBlobContainingPoint(CvPoint blob_point,int object_type);
	
private:

	vector<vector<double>> blob_distance_mat;	
	int_vector min_distance_index;	
	CvScalar abandoned_col,static_col,tracked_col,occluded_col,missed_col,removed_col,still_col;

public:

	CBlobResult *tracked_blobs;
		
	BwImage tracked_mask;
	BwImage static_mask;

	BwImage current_bounding_box_mask;
	RgbImage correspondence_image;	
	
	static match_struct *matching_params;
	static track_struct *tracking_params;
	static double max_dist_sqr;

	RgbImage diagnostic_image_frg;
	RgbImage diagnostic_image_frg_big;
	RgbImage diagnostic_image_tracked;
	RgbImage diagnostic_image_tracked_big;
	IplImage *temp_img_gray;
	IplImage *tracked_blobs_image;
	int tracked_blob_count;

	static bool area_method_updated;
	static bool diff_method_updated;

	bool state_variables_initialized;

	IplImage *temp_selection_image;

	static char *track_help_window;
	static char *match_help_window;

	static IplImage *track_help_image;
	static IplImage *match_help_image;





};

#endif
