#include "blobDetection.hpp"

//using namespace cvb;
char* BlobDetection::blob_detection_help_window;
IplImage* BlobDetection::blob_detection_help_image;

BlobDetection::BlobDetection(CvSize image_size,blob_detection_struct *blob_detect_params_init){

	blob_detection_help_window=new char[MAX_NAME];
	sprintf(blob_detection_help_window,"Blob Detection Help");	

	blob_detection_help_image=cvLoadImage("help/blob_detection_help.jpg");

	state_variables_initialized=false;
	blobs=NULL;
	initStateVariables(image_size);

	params=new blob_detection_struct;
	initParams(blob_detect_params_init);
	updateParams(0);


}
BlobDetection::~BlobDetection(){

	clearStateVariables();

	delete(blob_detection_help_window);
	delete(params);
}
void BlobDetection::initStateVariables(CvSize image_size){

	no_of_blobs=0;
	
	frame_size=image_size;	
	blob_image=cvCreateImage(image_size, IPL_DEPTH_8U, 3);
	bounding_box_image=cvCreateImage(image_size, IPL_DEPTH_8U, 3);
	mask_image_thresholded = cvCreateImage( frame_size, IPL_DEPTH_8U, 1);  
}

void BlobDetection::clearStateVariables(){

	if(blobs){
		delete(blobs);
		blobs=NULL;
	}

	if(!state_variables_initialized)
		return;
	
	cvReleaseImage(&blob_image);
	cvReleaseImage(&bounding_box_image);
	cvReleaseImage(&mask_image_thresholded);

	state_variables_initialized=false;
}

void BlobDetection::getBlobs(IplImage *mask_image,IplImage *current_frame,int show_blobs) { 

	//cout<<"Starting getBlobs \n";	

	//parameters
	int line_thickness=1;
	int line_type=8;
	int shift=0;

	cvSetZero(blob_image);
	//cvSetZero(bounding_box_image);
	cvCopy(current_frame, bounding_box_image);

	/**cvNamedWindow( "Blobs2", 1 );
	cvNamedWindow( "original", 1 );
	//cvNamedWindow( "displayedImage", 1 );
	cvNamedWindow( "unmodified", 1 );

	cvShowImage( "original", original );*/

	// threshold the input image to convert it to binary (in case it is already not)	
	cvThreshold( mask_image, mask_image_thresholded, 100,255, CV_THRESH_BINARY );

	//cvShowImage( "original_thresholded", original_thresholded );


	// find white blobs (or connected components) in the binary thresholded image
	//cout<<"About to run CBlobResult\n";
	if(blobs)
		delete(blobs);
	blobs = new CBlobResult( mask_image_thresholded, NULL, BACKGROUND_COLOR);
	//cout<<"Done running CBlobResult\n";

	// remove blobs smaller than min_blob_size
	// cout<<"About to run Filter\n";
	blobs->Filter( blobs, B_INCLUDE, CBlobGetArea(), B_GREATER, params->min_blob_size );

	//cout<<"Done running Filter\n";


	//blobs.PrintBlobs( "filteredBlobs.txt" );

	/*// get mean gray color of biggest blob
	CBlob biggestBlob;
	CBlobGetMean getMeanColor( original );
	double meanGray;

	blobs.GetNthBlob( CBlobGetArea(), 0, biggestBlob );
	meanGray = getMeanColor( biggestBlob );*/


	// display filtered blobs
	//cvMerge( original_thresholded, original_thresholded, original_thresholded, NULL, blob_image );    // 
	//cvShowImage( "blob_image", blob_image );    

	no_of_blobs=blobs->GetNumBlobs();
	//std::cout<<"No. of blobs:"<<no_of_blobs<<"\n";
	//bounding_box_mat=new CvRect(no_of_blobs);
	for (int i = 0; i <no_of_blobs ; i++ ) {
		CBlob *currentBlob = blobs->GetBlob(i);

		if(show_blobs){
			CvRect current_bounding_box=currentBlob->GetBoundingBox();
			CvPoint point_1,point_2;
			point_1.x=current_bounding_box.x;
			point_1.y=current_bounding_box.y;
			point_2.x=current_bounding_box.x+current_bounding_box.width;
			point_2.y=current_bounding_box.y+current_bounding_box.height;        
			cvRectangle(bounding_box_image, point_1, point_2,CV_RGB(0,255,0),line_thickness,line_type, shift);
		}

		currentBlob->FillBlob(blob_image,CV_RGB(0,255,0));
	} 	    
	//cout<<"Done getBlobs \n";
}

void BlobDetection::initParams(blob_detection_struct *blob_detect_params_init) {
	params->min_blob_size=blob_detect_params_init->min_blob_size;

}

void BlobDetection::updateParams(int) {	
// dummy function
}

void BlobDetection::showBlobDetectionHelpWindow(int mouse_event,int x,int y,int flags,void* param){

	if(mouse_event==CV_EVENT_LBUTTONDOWN){
		cvNamedWindow(blob_detection_help_window,1);
		cvShowImage(blob_detection_help_window,blob_detection_help_image);
	}else if(mouse_event==CV_EVENT_RBUTTONDOWN){
		cvDestroyWindow(blob_detection_help_window);
	}
}

void BlobDetection::initWindow(char *window_name){
	cvNamedWindow(window_name, 1 );
	cvCreateTrackbar("minBlobSize","Blobs",&(params->min_blob_size),200,NULL);
	cvSetMouseCallback(window_name,showBlobDetectionHelpWindow,(void*)this);
}

/*
IplImage* BlobDetection::getBlobs2(IplImage *gray,IplImage *unmodified) {
//IplImage *gray = cvCreateImage(cvGetSize(original), IPL_DEPTH_8U, 1);
//cvCvtColor(original, gray, CV_BGR2GRAY);
cvThreshold(gray, gray, 150, 255, CV_THRESH_BINARY);

IplImage *labelImg=cvCreateImage(frame_size, IPL_DEPTH_LABEL, 1);
CvBlobs blobs;
unsigned int result=cvLabel(gray, labelImg, blobs);
cvShowImage( "labelImg", labelImg );
//cvRenderBlobs(labelImg, blobs, gray, gray);
//cvShowImage( "labelImg", labelImg );
return labelImg;
}
*/
