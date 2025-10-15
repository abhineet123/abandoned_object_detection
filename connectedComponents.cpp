#include"connectedComponents.hpp"

void getConnectedComponents(IplImage* src){
	
	IplImage* dst = cvCreateImage( cvGetSize(src), 8, 3 );
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;

	cvThreshold( src, src, 1, 255, CV_THRESH_BINARY );
	cvNamedWindow( "Source", 1 );
	cvShowImage( "Source", src );

	cvFindContours( src, storage, &contour, sizeof(CvContour),CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
	cvZero( dst );

	for( ; contour != 0; contour = contour->h_next ){
		CvScalar color = CV_RGB( rand()&255, rand()&255, rand()&255 );
		/* replace CV_FILLED with 1 to see the outlines */
		cvDrawContours( dst, contour, color, color, -1, CV_FILLED, 8 );
	}

	cvNamedWindow( "Components", 1 );
	cvShowImage( "Components", dst );
	cvWaitKey(0);

}