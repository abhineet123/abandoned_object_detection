#ifndef _BLOB_DETECT_
#define _BLOB_DETECT_

#include<cv.h>
#include<highgui.h>

//#include "blobMain.h"
#include "BlobResult.h"

#define BACKGROUND_COLOR 0
#define FOREGROUND_COLOR 255

#define MAX_NAME 100

using namespace std;

struct blob_detection_struct{

	int min_blob_size;
};

class BlobDetection{

public:
	BlobDetection(CvSize image_size,blob_detection_struct *blob_detect_params_init);
	~BlobDetection();

	void initStateVariables(CvSize image_size);
	void clearStateVariables();

	void getBlobs(IplImage *mask_image,IplImage *current_frame,int show_blobs);

	//IplImage* getBlobs2(IplImage*,IplImage*);

	void initParams(blob_detection_struct *blob_detect_params_init);

	void updateParams(int);

	void initWindow(char *window_name);

	static void showBlobDetectionHelpWindow(int mouse_event,int x,int y,int flags,void* param);

	CBlobResult *blobs;
	int no_of_blobs;
	IplImage *blob_image;
	IplImage *bounding_box_image;
	IplImage *mask_image_thresholded;

	blob_detection_struct *params;

	CvSize frame_size;

	bool state_variables_initialized;

	static char *blob_detection_help_window;
	static IplImage *blob_detection_help_image;
};

#endif