#ifndef _BLOB_FILTER_
#define _BLOB_FILTER_

#include <cv.h>
#include<stdio.h>
#include<conio.h>
#include<vector>
#include <highgui.h>

#include "BlobResult.h"

#define CANCEL_OP 0
#define ADD_OBJ 1
#define REMOVE_OBJ 2
#define DO_NOTHING 3
#define OP_COMPLETE 4

#define DIST_EUCLIDEAN 0
#define DIST_BOUNDING_BOX 1

#define SHOW_FILTER_STATUS 0
#define MAX_NAME 100

//#include "Image.hpp"

using namespace std;
using namespace cv;

struct filter_struct{

	int enable_filtering;

	int size_thr_percent;
	int dist_thr_percent;
	int appearance_thr_percent;

	double size_thr;
	double dist_thr;
	double appearance_thr;
	int dist_method;

	double dist_thr_sqr;
	double appearance_thr_sqr;

	int match_location;
	int match_size;
	int match_appearance;
};


struct obj_outline_struct{
	int line_thickness;
	int line_type;
	int shift;

	CvScalar obj_col;
};

struct obj_struct{
	CvPoint min_point;
	CvPoint max_point;

	IplImage *obj_img;
	IplImage *obj_img_resized;

	CvPoint centroid;
	int obj_area;	
};

class BlobFilter{

public:

	BlobFilter(CvSize image_size,filter_struct *filter_params_init);
	~BlobFilter();

	void initStateVariables(CvSize image_size);
	void clearStateVariables();
	void freeImages();
	void freeObjects();

	int getObject(IplImage *temp_selection_image);
	inline void copyPixel(IplImage *src,IplImage *dst,int r,int c);
	void clearSelectionImage(IplImage *current_frame,obj_struct *obj);
	void removeObjectsFromEnd(IplImage *current_frame,int removed_obj_count);
	void addCandidateRemovedObjects(IplImage *current_frame);	
	inline void addObjectDataFromEnd(int obj_count);
	inline void showObjectsFromEnd(int obj_count);

	inline int matchObjectLocation(obj_struct *obj1);
	inline int matchObjectSize(obj_struct *obj1);
	inline int matchObjectAppearance(obj_struct *obj1,double resize_factor);
	int findMatchingeObject(double resize_factor);
	double getSquaredDistance(obj_struct *obj1,obj_struct *obj2);
	double findMinimumBoundingBoxDistance(CvPoint centroid,CvPoint min_point,CvPoint max_point);
	double getSquaredEuclideanDistance(CvPoint pt1,CvPoint pt2);

	double  getMeanSquaredPixelDifference(IplImage *img1,IplImage *img2,CvPoint min_point,CvPoint max_point,int pixel_count);

	inline int isSamePoint(CvPoint pt1,CvPoint pt2);

	static void printParams();
	static void updateParams(int);
	static void updateFiltering(int);
	static void updateMatch(int);
	void initWindow(char *window_name);

	void initParams(filter_struct *filter_params_init);
	static void getClickedPoint(int mouse_event,int x,int y,int flags,void* param);

	void initializeTestObject(CBlob *init_blob,IplImage *init_img);
	void resizeCandidateObjectImages(CvSize img_size);
	bool filterRemovedOjects(CBlobResult *blobs,IplImage *bkg_img,double resize_factor);

	int getObjectContainingPoint(CvPoint obj_point);
	void removeCandidateObject(int obj_id);

	inline void backupAndResizeObject(obj_struct *obj,double resize_factor);
	inline void restoreBackupObject(obj_struct *obj);
	inline void refreshSelectionImage(IplImage *current_frame);

	static void showBlobFilteringHelpWindow(int mouse_event,int x,int y,int flags,void* param);

	vector<obj_struct*> candidate_removed_objects;
	vector<IplImage*> candidate_removed_obj_images;
	
	IplImage *temp_selection_image;
	IplImage *hover_image;
	int clicked_point_count;
	CvPoint mouse_click_point;
	bool point_selected;
	bool left_button_clicked;
	bool right_button_clicked;		
	CvPoint mouse_hover_point;
	bool mouse_hover_event;

	static bool match_updated;


	obj_outline_struct *outline_params;
	static filter_struct *filter_params;

	CvSize original_size;

	bool state_variables_initialized;
	char *selection_window;

	obj_struct *test_obj;
	obj_struct *new_obj;
	obj_struct *backup_obj;

	static char *blob_filtering_help_window;
	static IplImage *blob_filtering_help_image;
};

#endif