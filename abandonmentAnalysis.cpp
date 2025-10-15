#include "abandonmentAnalysis.hpp"

abandonment_struct* AbandonmentAnalysis::params;
bool AbandonmentAnalysis::abandonment_updated;

char* AbandonmentAnalysis::abandonment_help_window;
IplImage* AbandonmentAnalysis::abandonment_help_image;


AbandonmentAnalysis::AbandonmentAnalysis(CvSize image_size,abandonment_struct *abandonment_params_init){

	storage_x = cvCreateMemStorage();
	storage_y = cvCreateMemStorage();

	abandonment_help_window=new char[MAX_NAME];
	sprintf(abandonment_help_window,"Abandonment Analysis Help");	

	abandonment_help_image=cvLoadImage("help/abandonment_help.jpg");

	in_obj=new RunningStatVector;
	out_obj=new RunningStatVector;

	up_obj=new RunningStatVector;
	down_obj=new RunningStatVector;

	state_variables_initialized=false;
	initStateVariables(image_size);

	/*sorted_by_x=cvCreateSeq(CV_SEQ_ELTYPE_POINT,sizeof(CvSeq),sizeof(CvPoint),storage_x);
	sorted_by_y=cvCreateSeq(CV_SEQ_ELTYPE_POINT,sizeof(CvSeq),sizeof(CvPoint),storage_y);*/

	sorted_by_x=NULL;
	sorted_by_y=NULL;

	eroded_blobs=NULL;

	point_set_mean.resize(REGION_COUNT);
	point_set_std.resize(REGION_COUNT);

	params=new abandonment_struct;
	initParams(abandonment_params_init);
	updateAbandonmentMethod(0);
	updateSimilarityMethod(0);
	abandonment_updated=false;
	updateParams(0);

}

AbandonmentAnalysis::~AbandonmentAnalysis(){

	clearStateVariables();

	delete(in_obj);
	delete(out_obj);
	delete(up_obj);
	delete(down_obj);
	delete(params);

	delete(abandonment_help_window);
}

void AbandonmentAnalysis::initStateVariables(CvSize image_size){

	clearStateVariables();

	contour_image=cvCreateImage(image_size, IPL_DEPTH_8U, 3);	
	cvSet(contour_image.Ptr(),CV_RGB(255,255,255));

	temp_rgb_img=cvCreateImage(image_size, IPL_DEPTH_8U, 3);
	temp_gray_img=cvCreateImage(image_size, IPL_DEPTH_8U, 1);

	region_growing_bkg=cvCreateImage(image_size, IPL_DEPTH_8U, 1);
	region_growing_frg=cvCreateImage(image_size, IPL_DEPTH_8U, 1);

	canny_bkg=cvCreateImage(image_size, IPL_DEPTH_8U, 1);
	canny_frg=cvCreateImage(image_size, IPL_DEPTH_8U, 1);

	frg_grad_x=cvCreateImage(image_size, IPL_DEPTH_8U, 1);
	frg_grad_y=cvCreateImage(image_size, IPL_DEPTH_8U, 1);
	frg_grad_combined=cvCreateImage(image_size, IPL_DEPTH_8U, 1);

	bkg_grad_x=cvCreateImage(image_size, IPL_DEPTH_8U, 1);
	bkg_grad_y=cvCreateImage(image_size, IPL_DEPTH_8U, 1);
	bkg_grad_combined=cvCreateImage(image_size, IPL_DEPTH_8U, 1);

	blob_image=cvCreateImage(image_size, IPL_DEPTH_8U, 1);
	boundary_image=cvCreateImage(image_size, IPL_DEPTH_8U,3);

	proc_size=image_size;

	state_variables_initialized=true;
	abandonment_updated=false;
}

void AbandonmentAnalysis::clearStateVariables(){

	if(!state_variables_initialized)
		return;

	contour_image.freeImage();	

	temp_rgb_img.freeImage();
	temp_gray_img.freeImage();

	cvReleaseImage(&canny_bkg);
	cvReleaseImage(&canny_frg);

	cvReleaseImage(&frg_grad_x);
	cvReleaseImage(&frg_grad_y);
	cvReleaseImage(&frg_grad_combined);

	cvReleaseImage(&bkg_grad_x);
	cvReleaseImage(&bkg_grad_y);
	cvReleaseImage(&bkg_grad_combined);

	cvReleaseImage(&blob_image);
	cvReleaseImage(&boundary_image);

	state_variables_initialized=false;
}
//--------------------------------------- Region Growing Functions---------------------------------------------//

int comparePointsByX(const void* a, const void* b, void* userdata ){
	CvPoint *pt1=(CvPoint*)a;
	CvPoint *pt2=(CvPoint*)b;

	if(pt1->x>pt2->x)
		return 1;

	if(pt1->x<pt2->x)
		return -1;

	return 0;
}

int comparePointsByY(const void* a, const void* b, void* userdata ){
	CvPoint *pt1=(CvPoint*)a;
	CvPoint *pt2=(CvPoint*)b;

	if(pt1->y>pt2->y)
		return 1;

	if(pt1->y<pt2->y)
		return -1;

	return 0;
}

void AbandonmentAnalysis::getBoundedBox(CvPoint *min_point,CvPoint *max_point){
	int horz_count=contour_horz_extent.size();
	int max_min_x=-INF,min_max_x=INF;

	for(int i=0;i<horz_count;i++){
		if(max_min_x<contour_horz_extent[i]->min_x)
			max_min_x=contour_horz_extent[i]->min_x;

		if(min_max_x>contour_horz_extent[i]->max_x)
			min_max_x=contour_horz_extent[i]->max_x;
	}

	int vert_count=contour_vert_extent.size();
	int max_min_y=-INF,min_max_y=INF;

	for(int i=0;i<vert_count;i++){
		if(max_min_y<contour_vert_extent[i]->min_y)
			max_min_y=contour_vert_extent[i]->min_y;

		if(min_max_y>contour_vert_extent[i]->max_y)
			min_max_y=contour_vert_extent[i]->max_y;
	}

	min_point->y=max_min_y;
	min_point->x=max_min_x;

	max_point->y=min_max_y;
	max_point->x=min_max_x;
}

void AbandonmentAnalysis::getBoundingBox(CvPoint *min_point,CvPoint *max_point){
	int horz_count=contour_horz_extent.size();
	int min_min_x=INF,max_max_x=-INF;

	for(int i=0;i<horz_count;i++){
		if(min_min_x>contour_horz_extent[i]->min_x)
			min_min_x=contour_horz_extent[i]->min_x;

		if(max_max_x<contour_horz_extent[i]->max_x)
			max_max_x=contour_horz_extent[i]->max_x;
	}

	int vert_count=contour_vert_extent.size();
	int min_min_y=INF,max_max_y=-INF;

	for(int i=0;i<vert_count;i++){
		if(min_min_y>contour_vert_extent[i]->min_y)
			min_min_y=contour_vert_extent[i]->min_y;

		if(max_max_y<contour_vert_extent[i]->max_y)
			max_max_y=contour_vert_extent[i]->max_y;
	}

	min_point->y=min_min_y;
	min_point->x=min_min_x;

	max_point->y=max_max_y;
	max_point->x=max_max_x;
}

void AbandonmentAnalysis::getDilatedBox(CvPoint *min_point,CvPoint *max_point,IplImage *img){

	min_point->x-=params->erosion_width;
	min_point->y-=params->erosion_width;

	max_point->x+=params->erosion_width;
	max_point->y+=params->erosion_width;

	if(max_point->x>=img->width)
		max_point->x=img->width-1;
	if(max_point->y>=img->height)
		max_point->y=img->height-1;

	if(min_point->x<0)
		min_point->x=0;
	if(min_point->y<0)
		min_point->y=0;

	if(min_point->x>max_point->x)
		min_point->x=max_point->x;
	if(min_point->y>max_point->y)
		min_point->y=max_point->y;
}

void AbandonmentAnalysis::getErodedBox(CvPoint *min_point,CvPoint *max_point,IplImage *img){

	min_point->x+=params->erosion_width;
	min_point->y+=params->erosion_width;

	max_point->x-=params->erosion_width;
	max_point->y-=params->erosion_width;

	if(min_point->x>=img->width)
		min_point->x=img->width-1;
	if(min_point->y>=img->height)
		min_point->y=img->height-1;

	if(max_point->x<0)
		max_point->x=0;
	if(max_point->y<0)
		max_point->y=0;

	if(min_point->x>max_point->x)
		min_point->x=max_point->x;
	if(min_point->y>max_point->y)
		min_point->y=max_point->y;
}

void AbandonmentAnalysis::showBoxes(CBlobResult *blobs,int line_thickness,int line_type, int shift){
	
	CvScalar bounded_col=CV_RGB(255,0,0);
	CvScalar bounding_col=CV_RGB(0,0,255);
	CvScalar blob_col=CV_RGB(0,255,0);
	CvScalar background_col=CV_RGB(255,255,255);

	cvSet(temp_rgb_img.Ptr(),background_col);

	
	for(int i=0;i<blobs->GetNumBlobs();i++){

		CBlob *blob=blobs->GetBlob(i);

		blob->FillBlob(temp_rgb_img.Ptr(),CV_RGB(0,255,0));

		CBlobContour *blob_contour=blob->GetExternalContour();
		CvSeq *point_set=blob_contour->GetContourPoints();
		getSortedContourPoints(point_set);
		//cout<<"calling getContourExtents\n";
		getContourExtents();

		CvPoint bounded_min,bounded_max;
		getBoundedBox(&bounded_min,&bounded_max);
		//getErodedBox(&bounded_min,&bounded_max,temp_rgb_img.Ptr());

		CvPoint bounding_min,bounding_max;
		getBoundingBox(&bounding_min,&bounding_max);
		getDilatedBox(&bounding_min,&bounding_max,temp_rgb_img.Ptr());

		cvRectangle(temp_rgb_img.Ptr(), bounded_min, bounded_max,bounded_col,line_thickness,line_type,shift);
		//cvRectangle(temp_rgb_img.Ptr(), bounding_min, bounding_max,bounding_col,line_thickness,line_type,shift);
	}	
}

void AbandonmentAnalysis::getSortedContourPoints(CvSeq *contour_points){

	if(sorted_by_x)
		cvClearSeq(sorted_by_x);
	if(sorted_by_y)
		cvClearSeq(sorted_by_y);

	sorted_by_x = cvCloneSeq(contour_points, storage_x);
	sorted_by_y = cvCloneSeq(contour_points, storage_y);

	cvSeqSort(sorted_by_x,comparePointsByX,0);
	cvSeqSort(sorted_by_y,comparePointsByY,0);
}

int  AbandonmentAnalysis::getTotalPointCount(){

	int total_point_count=0;
	total_point_count+=inward_growing_points.size();
	total_point_count+=outward_growing_points.size();

	total_point_count+=downward_growing_points.size();
	total_point_count+=upward_growing_points.size();

	return total_point_count;
}
void AbandonmentAnalysis::displayRegionPoints(IplImage *img){
	
	int point_count=blob_seed_points.size();
	for(int i=0;i<point_count;i++){
		seed_point *current_point=blob_seed_points[i];
		int current_pos=current_point->y*img->widthStep+current_point->x*img->nChannels;
		img->imageData[current_pos]=(unsigned char)FOREGROUND_COLOR;
	}
}


// return 1 for removed, 0 for abandoned and -1 for neither (state change)
int AbandonmentAnalysis::evaluateRegionGrowingPixelCount(CBlob *blob,RgbImage &frg_img,RgbImage &bkg_img,CvSize image_size){

	if(SHOW_STATUS_ABND)
		cout<<"Starting evaluateRegionGrowingPixelCount \n";
	int use_old_method=0;
	int bkg_count=0,frg_count=0;
	if(use_old_method){

		CBlobContour *blob_contour=blob->GetExternalContour();
		CvSeq *point_set=blob_contour->GetContourPoints();

		getSortedContourPoints(point_set);	
		getContourExtents();
		getErodedContour();

		initializeSeedPoints(bkg_img);
		//drawContour(0,CV_RGB(0,255,0));
		applyRegionGrowing(bkg_img,image_size);		
		//clearContourImage();
		//drawContour(CV_RGB(0,255,0));

		bkg_count=getTotalPointCount();

		//clearContourImage();	
		initializeSeedPoints(frg_img);	
		drawContour(1,CV_RGB(255,0,0));

		applyRegionGrowing(frg_img,image_size);	
		drawContour(0,CV_RGB(0,255,0));

		frg_count=getTotalPointCount();

	}else{

		int initial_count,final_count;

		getBlobSeedPoints(blob);
		initial_count=blob_seed_points.size();
		performRegionGrowing(frg_img.Ptr());
		displayRegionPoints(region_growing_bkg);
		final_count=blob_seed_points.size();
		//frg_count=final_count-initial_count;
		frg_count=final_count;

		getBlobSeedPoints(blob);
		initial_count=blob_seed_points.size();
		performRegionGrowing(bkg_img.Ptr());
		displayRegionPoints(region_growing_frg);
		final_count=blob_seed_points.size();
		//bkg_count=final_count-initial_count;
		bkg_count=final_count;

		cout<<"frg_count="<<frg_count<<"\t";
		cout<<"bkg_count="<<bkg_count<<"\n";
		/*cvShowImage("bkg_img",bkg_img.Ptr());
		cvShowImage("frg_img",frg_img.Ptr());
		cvWaitKey(0);
		cvDestroyWindow("bkg_img");
		cvDestroyWindow("frg_img");*/
		//_getch();
	}

	if(bkg_count<frg_count){
		//if background has less pixels in the grown region than the current frame, the object is removed.
		if(bkg_count==0)
			return 1;
		double fractional_diff=(double)(frg_count-bkg_count)/(double)bkg_count;			
		if(fractional_diff>=params->region_diff_threshold)
			return 1;
		//if background and the the current frame have similar number of pixels in the grown region, the object represents state change.
		return -1;
	}else if(bkg_count>frg_count){

		if(frg_count==0)
			return 1;
		//if background has more pixels in the grown region than the current frame, the object is abandoned.
		double fractional_diff=(double)(bkg_count-frg_count)/(double)frg_count;	
		if(fractional_diff>=params->region_diff_threshold)
			return 0;
	}
	//if background and the the current frame have similar number of pixels in the grown region, the object represents state change.

	if(SHOW_STATUS_ABND)
		cout<<"Done evaluateRegionGrowingPixelCount \n";
	return -1;
}

bool AbandonmentAnalysis::detectRemovedObjectsUsingRegionGrowing(CBlobResult *tracked_blobs,RgbImage &frg_img,RgbImage &bkg_img,CvSize image_size){

	if(SHOW_STATUS_ABND)
		cout<<"Starting detectRemovedObjectsUsingRegionGrowing \n";	
	
	if((frg_img.width()!=bkg_img.width())||(frg_img.height()!=bkg_img.height())||(frg_img.nChannels()!=bkg_img.nChannels())){
		cout<<"\nError in detectRemovedObjectsUsingCannyEdgeDetection.\n";

		cout<<"The properties of current and background images do not match.\n";
		cout<<"Width: Current: "<<frg_img.width()<<"\tBackground: "<<bkg_img.width()<<"\n";
		cout<<"Height: Current: "<<frg_img.height()<<"\tBackground: "<<bkg_img.height()<<"\n";
		cout<<"No. of channels: Current: "<<frg_img.nChannels()<<"\tBackground: "<<bkg_img.nChannels()<<"\n";
		_getch();
		return false;
	}	

	if((frg_img.width()!=proc_size.width)||(frg_img.height()!=proc_size.height)){
		cout<<"\nError in detectRemovedObjectsUsingRegionGrowing.\n";

		cout<<"The current image size does not match with the expected size.\n";
		cout<<"Width: Current: "<<frg_img.width()<<"\t Expected: "<<proc_size.width<<"\n";
		cout<<"Height: Current: "<<frg_img.height()<<"\t Expected: "<<proc_size.height<<"\n";		
		_getch();
		return false;
	}	

	cvSet(contour_image.Ptr(),CV_RGB(255,255,255));
	cvSetZero(region_growing_bkg);
	cvSetZero(region_growing_frg);

	bool found_state_change=false;
	int tracked_blob_count=tracked_blobs->GetNumBlobs();

	for(int i=0;i<tracked_blob_count;i++){
		CBlob *current_blob=tracked_blobs->GetBlob(i);

		if((current_blob->is_occluded)||(!current_blob->is_candidate))
			continue;

		current_blob->is_candidate=false;

		if((params->detect_still_person)&&(current_blob->diff_moving_avg>params->max_avg_diff)){
			current_blob->is_removed=false;
			current_blob->is_abandoned=false;
			current_blob->is_state_change=false;
			current_blob->is_still_person=true;
			continue;
		}

		int obj_status=evaluateRegionGrowingPixelCount(current_blob,frg_img,bkg_img,image_size);
		if(obj_status==1){
			current_blob->is_abandoned=false;
			current_blob->is_removed=true;
		}else if(obj_status==0){
			current_blob->is_abandoned=true;
			current_blob->is_removed=false;
		}else if(obj_status==-1){
			current_blob->is_abandoned=false;
			current_blob->is_removed=false;
			current_blob->is_state_change=true;
			found_state_change=true;
		}
		
	}
	if(SHOW_STATUS_ABND)
		cout<<"Done detectRemovedObjectsUsingRegionGrowing \n";
	return found_state_change;
}

//----------------------------------------------------------------------------------------------------------------------//


//--------------------------------------- Edge Detection Functions---------------------------------------------//

int AbandonmentAnalysis::getEdgePixelCount(CvSeq *point_set,IplImage *img){
	if(SHOW_STATUS_ABND)
		cout<<"Starting detectRemovedObjectsUsingRegionGrowing \n";
	
	int edge_pixel_count=0;	
	int contour_length=point_set->total;		

	for(int j=0;j<contour_length;j++){
		CvPoint *contour_point=(CvPoint*)cvGetSeqElem(point_set,j);
		int pixel_location=contour_point->y*img->widthStep+contour_point->x;
		unsigned char current_val=(unsigned char)img->imageData[pixel_location];
		if(current_val==FOREGROUND_COLOR)
			edge_pixel_count++;
	}
	return edge_pixel_count;

	if(SHOW_STATUS_ABND)
		cout<<"Done detectRemovedObjectsUsingRegionGrowing \n";
}


//---------------------------------------Canny Edge Detection--------------------------------------------------------//

//returns 1 if object is removed, 0 if it is abandoned and -1 if it is neither (state change)
int AbandonmentAnalysis::evaluateCannyEdgePixelCount(CBlob *blob){

	if(SHOW_STATUS_ABND)
		cout<<"Starting evaluateCannyEdgePixelCount \n";	

	CBlobContour *blob_contour=blob->GetExternalContour();
	CvSeq *point_set=blob_contour->GetContourPoints();		

	int show_blob=0;

	int frg_edge_pixel_count=getEdgePixelCount(point_set,canny_frg);
	if(show_blob){
		cvShowImage( "canny_frg_original",canny_frg);	
		cvShowImage( "canny_frg_thresholded",temp_gray_img.Ptr());	
	}

	int bkg_edge_pixel_count=getEdgePixelCount(point_set,canny_bkg);
	if(show_blob){
		cvShowImage( "canny_bkg_original",canny_bkg);	
		cvShowImage( "canny_bkg_thresholded",temp_gray_img.Ptr());	
	}	

	if(show_blob){
		cout<<"frg_edge_pixel_count="<<frg_edge_pixel_count<<"\t";
		cout<<"bkg_edge_pixel_count="<<bkg_edge_pixel_count<<"\n";
	}

	if(bkg_edge_pixel_count>frg_edge_pixel_count){
		if(frg_edge_pixel_count==0){
			//cout<<"\nObtained zero edge pixels in the foreground image\n";
			//_getch();
			return 1;
		}
		//cout<<"\nFound more or equal edge pixels in the background image.\n";
		//_getch();
		double fractional_diff=(double)(abs(bkg_edge_pixel_count-frg_edge_pixel_count))/(double)frg_edge_pixel_count;
		if(fractional_diff>=params->edge_diff_threshold)
			return 1;

	}else if(bkg_edge_pixel_count<frg_edge_pixel_count){
		if(bkg_edge_pixel_count==0){
			//cout<<"\nObtained zero edge pixels in the foreground image\n";
			//_getch();
			return 0;
		}

		double fractional_diff=(double)(abs(frg_edge_pixel_count-bkg_edge_pixel_count))/(double)bkg_edge_pixel_count;
		if(fractional_diff>=params->edge_diff_threshold)
			return 0;
	}

	return -1;

	if(SHOW_STATUS_ABND)
		cout<<"Done evaluateCannyEdgePixelCount \n";
}

bool AbandonmentAnalysis::detectRemovedObjectsUsingCannyEdgeDetection(CBlobResult *tracked_blobs,IplImage *frg_img,IplImage *bkg_img){	

	if(SHOW_STATUS_ABND)
		cout<<"Starting detectRemovedObjectsUsingCannyEdgeDetection \n";

	if((frg_img->width!=bkg_img->width)||(frg_img->height!=bkg_img->height)||(frg_img->nChannels!=bkg_img->nChannels)){
		cout<<"\nError in detectRemovedObjectsUsingCannyEdgeDetection.\n";

		cout<<"The properties of current and background images do not match.\n";
		cout<<"Width: Current: "<<frg_img->width<<"\tBackground: "<<bkg_img->width<<"\n";
		cout<<"Height: Current: "<<frg_img->height<<"\tBackground: "<<bkg_img->height<<"\n";
		cout<<"No. of channels: Current: "<<frg_img->nChannels<<"\tBackground: "<<bkg_img->nChannels<<"\n";
		_getch();
		 return false;
	}

	if((frg_img->width!=canny_frg->width)||(frg_img->height!=canny_frg->height)||(frg_img->nChannels!=canny_frg->nChannels)){
		cout<<"\nError in detectRemovedObjectsUsingCannyEdgeDetection.\n";

		cout<<"The properties of input images do not match those of the the stored ones.\n";
		cout<<"Width: Actual: "<<frg_img->width<<"\tExpected: "<<canny_frg->width<<"\n";
		cout<<"Height: Actual: "<<frg_img->height<<"\tExpected: "<<canny_frg->height<<"\n";
		cout<<"No. of channels: Actual: "<<frg_img->nChannels<<"\tExpected: "<<canny_frg->nChannels<<"\n";
		_getch();
		 return false;
	}

	cvCanny(frg_img,canny_frg,params->canny_low_thr,params->canny_high_thr);
	if(SHOW_CANNY)
		cvShowImage("Current Canny",canny_frg);
	cvCanny(bkg_img,canny_bkg,params->canny_low_thr,params->canny_high_thr);
	if(SHOW_CANNY)
		cvShowImage("Background Canny",canny_bkg);

	bool found_state_change=false;
	int tracked_blob_count=tracked_blobs->GetNumBlobs();

	for(int i=0;i<tracked_blob_count;i++){
		CBlob *current_blob=tracked_blobs->GetBlob(i);
		
		if((current_blob->is_occluded)||(!current_blob->is_candidate))
			continue;

		current_blob->is_candidate=false;

		if((params->detect_still_person)&&(current_blob->diff_moving_avg>params->max_avg_diff)){
			current_blob->is_removed=false;
			current_blob->is_abandoned=false;
			current_blob->is_state_change=false;
			current_blob->is_still_person=true;			
			cout<<"Blob "<<i<<" detected as a still person with diff_moving_avg="<<current_blob->diff_moving_avg<<"\n";
			continue;
		}		

		int obj_status=evaluateCannyEdgePixelCount(current_blob);
		if(obj_status==1){
			current_blob->is_abandoned=false;
			current_blob->is_removed=true;
			cout<<"Blob "<<i<<" detected as removed.\n";
		}else if(obj_status==0){
			current_blob->is_abandoned=true;
			current_blob->is_removed=false;
			cout<<"Blob "<<i<<" detected as abandoned.\n";
		}else if(obj_status==-1){
			current_blob->is_abandoned=false;
			current_blob->is_removed=false;
			current_blob->is_state_change=true;		
			found_state_change=true;
			cout<<"Blob "<<i<<" detected as state change.\n";
		}		
	}

	if(SHOW_STATUS_ABND)
		cout<<"Done detectRemovedObjectsUsingCannyEdgeDetection \n";
	return found_state_change;
}

//---------------------------------------Gradient Edge Detection--------------------------------------------------------//

//returns 1 if object is removed, 0 if it is abandoned and -1 if it is neither (state change)
void  AbandonmentAnalysis::getCombinedThresholdedGradientImage(IplImage *grad_combined, IplImage *grad_x,IplImage *grad_y){

	if((grad_x->width!=grad_y->width)||(grad_x->height!=grad_y->height)||(grad_x->nChannels!=grad_y->nChannels)){
		cout<<"\nError in getCombinedThresholdedGradientImage.\n";

		cout<<"The properties of current and background images do not match.\n";
		cout<<"Width: grad_x: "<<grad_x->width<<"\t grad_y: "<<grad_y->width<<"\n";
		cout<<"Height: grad_x: "<<grad_x->height<<"\t grad_y: "<<grad_y->height<<"\n";
		cout<<"No. of channels: grad_x: "<<grad_x->nChannels<<"\t grad_y: "<<grad_y->nChannels<<"\n";
		_getch();
		return;
	}

	for(int r=0;r<grad_x->height;r++){
		int col_pos=r*grad_x->widthStep;
		for(int c=0;c<grad_x->width;c++){
			unsigned char val_x=(unsigned char)grad_x->imageData[col_pos+c];
			unsigned char val_y=(unsigned char)grad_y->imageData[col_pos+c];

			if((val_x==FOREGROUND_COLOR)||(val_y==FOREGROUND_COLOR))
				grad_combined->imageData[col_pos+c]=(unsigned char)FOREGROUND_COLOR;
			else
				grad_combined->imageData[col_pos+c]=0;			
		}
	}
}

void AbandonmentAnalysis::getThresholdedGradientImages(img_grad* frg_grad,img_grad *bkg_grad){

	if(SHOW_STATUS_ABND)
		cout<<"Starting getThresholdedGradientImages \n";	
	
		cvThreshold(frg_grad->grad_x.Ptr(),frg_grad_x,params->gradient_min_thr,255,CV_THRESH_BINARY );
		cvThreshold(frg_grad->grad_y.Ptr(),frg_grad_y,params->gradient_min_thr,255,CV_THRESH_BINARY );

		cvThreshold(bkg_grad->grad_x.Ptr(),bkg_grad_x,params->gradient_min_thr,255,CV_THRESH_BINARY );
		cvThreshold(bkg_grad->grad_y.Ptr(),bkg_grad_y,params->gradient_min_thr,255,CV_THRESH_BINARY );
	

	/*if(!show_blob)
		return;	*/

	//CvRect bounding_box=blob->GetBoundingBox();
	//CvPoint min_point,max_point;
	//min_point.x=bounding_box.x;
	//min_point.y=bounding_box.y;		
	//max_point.x=min_point.x+bounding_box.width;
	//max_point.y=min_point.y+bounding_box.height;

	//int line_thickness=1;
	//int line_type=8;
	//int shift=0;
	//CvScalar blob_col=CV_RGB(255,0,0);

	//temp_rgb_img.Clear();
	//covertGrayscaleToRGB(temp_gray_img.Ptr(),temp_rgb_img.Ptr());
	//cvRectangle(temp_rgb_img.Ptr(),min_point,max_point,blob_col,line_thickness,line_type,shift);

	if(SHOW_STATUS_ABND)
		cout<<"Done getThresholdedGradientImages \n";
}

int AbandonmentAnalysis::evaluateGradientEdgePixelCount(CBlob *blob,img_grad* frg_grad,img_grad *bkg_grad){

	if(SHOW_STATUS_ABND)
		cout<<"Starting evaluateGradientEdgePixelCount \n";		
	
	CBlobContour *blob_contour=blob->GetExternalContour();
	CvSeq *point_set=blob_contour->GetContourPoints();	

	int show_blob=0;	
	
	int frg_edge_pixel_count=0;
	frg_edge_pixel_count+=getEdgePixelCount(point_set,frg_grad_x);	
	frg_edge_pixel_count+=getEdgePixelCount(point_set,frg_grad_y);
	

	int bkg_edge_pixel_count=0;
	bkg_edge_pixel_count+=getEdgePixelCount(point_set,bkg_grad_x);
	bkg_edge_pixel_count+=getEdgePixelCount(point_set,bkg_grad_y);


	if(show_blob){
		cout<<"frg_edge_pixel_count="<<frg_edge_pixel_count<<"\t";
		cout<<"bkg_edge_pixel_count="<<bkg_edge_pixel_count<<"\n";
	}

	if(bkg_edge_pixel_count>frg_edge_pixel_count){
		if(frg_edge_pixel_count==0){
			//cout<<"\nObtained zero edge pixels in the foreground image\n";
			//_getch();
			return 1;
		}
		//cout<<"\nFound more or equal edge pixels in the background image.\n";
		//_getch();
		double fractional_diff=(double)(abs(bkg_edge_pixel_count-frg_edge_pixel_count))/(double)frg_edge_pixel_count;
		if(fractional_diff>=params->edge_diff_threshold)
			return 1;

	}else if(bkg_edge_pixel_count<frg_edge_pixel_count){
		if(bkg_edge_pixel_count==0){
			//cout<<"\nObtained zero edge pixels in the foreground image\n";
			//_getch();
			return 0;
		}

		double fractional_diff=(double)(abs(frg_edge_pixel_count-bkg_edge_pixel_count))/(double)bkg_edge_pixel_count;
		if(fractional_diff>=params->edge_diff_threshold)
			return 0;
	}

	if(SHOW_STATUS_ABND)
		cout<<"Done evaluateGradientEdgePixelCount \n";

	return -1;
}

bool AbandonmentAnalysis::detectRemovedObjectsUsingGradientEdgeDetection(CBlobResult *tracked_blobs,img_grad* frg_grad,img_grad *bkg_grad){	

	if(SHOW_STATUS_ABND)
		cout<<"Starting detectRemovedObjectsUsingGradientEdgeDetection \n";

	getThresholdedGradientImages(frg_grad,bkg_grad);
	getCombinedThresholdedGradientImage(bkg_grad_combined,bkg_grad_x,bkg_grad_y);
	getCombinedThresholdedGradientImage(frg_grad_combined,frg_grad_x,frg_grad_y);
	
	bool found_state_change=false;
	int tracked_blob_count=tracked_blobs->GetNumBlobs();

	for(int i=0;i<tracked_blob_count;i++){
		CBlob *current_blob=tracked_blobs->GetBlob(i);

		if((current_blob->is_occluded)||(!current_blob->is_candidate))
			continue;

		current_blob->is_candidate=false;

		//Check for still person
		if((params->detect_still_person)&&(current_blob->diff_moving_avg>params->max_avg_diff)){
			current_blob->is_removed=false;
			current_blob->is_abandoned=false;
			current_blob->is_state_change=false;
			current_blob->is_still_person=true;			
			continue;
		}

		int obj_status=evaluateGradientEdgePixelCount(current_blob,frg_grad,bkg_grad);
		if(obj_status==1){
			current_blob->is_abandoned=false;
			current_blob->is_removed=true;
		}else if(obj_status==0){
			current_blob->is_abandoned=true;
			current_blob->is_removed=false;
		}else if(obj_status==-1){
			current_blob->is_abandoned=false;
			current_blob->is_removed=false;
			current_blob->is_state_change=true;
			found_state_change=true;
		}		
	}

	if(SHOW_STATUS_ABND)
		cout<<"Done detectRemovedObjectsUsingGradientEdgeDetection \n";
	return found_state_change;
}

//----------------------------------------------------------------------------------------------------------------------//

void AbandonmentAnalysis::showGradientImagesForAbandonedBlobs(CBlobResult &blobs,ForegroundProc *frg_obj,IplImage *temp_rgb_img,IplImage *temp_gray_img){

	int blob_count=blobs.GetNumBlobs();
	CBlob *current_blob;
	CvRect bounding_box;
	CvPoint min_point,max_point;
	CvScalar blob_col=CV_RGB(255,0,0);
	int min_intensity=100;

	//parameters
	int line_thickness=1;
	int line_type=8;
	int shift=0;

	IplImage *frg_grad_x=frg_obj->frg_grad->grad_x.Ptr();
	IplImage *frg_grad_y=frg_obj->frg_grad->grad_y.Ptr();
	IplImage *bkg_grad_x=frg_obj->bkg_grad->grad_x.Ptr();
	IplImage *bkg_grad_y=frg_obj->bkg_grad->grad_y.Ptr();	

	for(int i=0;i<blob_count;i++){
		current_blob=blobs.GetBlob(i);

		/*if(!current_blob->is_abandoned)
		continue;*/

		bounding_box=current_blob->GetBoundingBox();
		min_point.x=bounding_box.x;
		min_point.y=bounding_box.y;		
		max_point.x=min_point.x+bounding_box.width;
		max_point.y=min_point.y+bounding_box.height;

		cvThreshold( frg_grad_x,temp_gray_img, min_intensity,255, CV_THRESH_BINARY );
		covertGrayscaleToRGB(temp_gray_img,temp_rgb_img);
		cvRectangle(temp_rgb_img,min_point,max_point,blob_col,line_thickness,line_type,shift);
		cvShowImage( "frg_grad_x",temp_rgb_img);

		cvThreshold( frg_grad_y,temp_gray_img, min_intensity,255, CV_THRESH_BINARY );
		covertGrayscaleToRGB(temp_gray_img,temp_rgb_img);
		cvRectangle(temp_rgb_img,min_point,max_point,blob_col,line_thickness,line_type,shift);
		cvShowImage( "frg_grad_y",temp_rgb_img);

		cvThreshold( bkg_grad_x,temp_gray_img, min_intensity,255, CV_THRESH_BINARY );
		covertGrayscaleToRGB(temp_gray_img,temp_rgb_img);
		cvRectangle(temp_rgb_img,min_point,max_point,blob_col,line_thickness,line_type,shift);
		cvShowImage( "bkg_grad_x",temp_rgb_img);

		cvThreshold( bkg_grad_y,temp_gray_img, min_intensity,255, CV_THRESH_BINARY );
		covertGrayscaleToRGB(temp_gray_img,temp_rgb_img);
		cvRectangle(temp_rgb_img,min_point,max_point,blob_col,line_thickness,line_type,shift);
		cvShowImage( "bkg_grad_y",temp_rgb_img);

		cvWaitKey(0);
	}
}

void AbandonmentAnalysis::printParams(){

	cout<<"\n";

	cout<<"Abandonment analysis parameters updated:\n";

	cout<<"edge_diff_threshold="<<params->edge_diff_threshold<<"\t";
	cout<<"region_growing_threshold="<<params->region_growing_threshold<<"\t";
	cout<<"region_diff_threshold="<<params->region_diff_threshold<<"\t";
	cout<<"max_squared_diff="<<params->max_squared_diff<<"\t";
	cout<<"detect_still_person="<<params->detect_still_person<<"\t";
	cout<<"max_avg_diff="<<params->max_avg_diff<<"\t";
	cout<<"canny_low_thr="<<params->canny_low_thr<<"\t";
	cout<<"canny_high_thr="<<params->canny_high_thr<<"\t";
	cout<<"erosion_width="<<params->erosion_width<<"\n";
	cout<<"gradient_min_thr="<<params->gradient_min_thr<<"\n";

	cout<<"\n";
	//_getch();
}

void AbandonmentAnalysis::initParams(abandonment_struct *abandonment_params_init) {

	if(SHOW_STATUS_ABND)
		cout<<"Starting AbandonmentAnalysis::initParams \n";

	params->abandonment_method=abandonment_params_init->abandonment_method;	
	params->similarity_method=abandonment_params_init->similarity_method;	
	params->erosion_width=abandonment_params_init->erosion_width;

	params->edge_diff_threshold_percent=abandonment_params_init->edge_diff_threshold_percent;
	params->region_growing_threshold_percent=abandonment_params_init->region_growing_threshold_percent;
	params->region_diff_threshold_percent=abandonment_params_init->region_diff_threshold_percent;

	params->detect_still_person=abandonment_params_init->detect_still_person;
	params->max_avg_diff_10=abandonment_params_init->max_avg_diff_10;
	params->max_squared_diff=abandonment_params_init->max_squared_diff;

	params->canny_low_thr=abandonment_params_init->canny_low_thr;
	params->canny_ratio=abandonment_params_init->canny_ratio;

	params->gradient_min_thr=abandonment_params_init->gradient_min_thr;

	if(SHOW_STATUS_ABND)
		cout<<"Done  AbandonmentAnalysis::initParams \n";
}

void AbandonmentAnalysis::updateParams(int) {	

	if(SHOW_STATUS_ABND)
		cout<<"Starting AbandonmentAnalysis::updateParams \n";
	
	params->edge_diff_threshold=(double)params->edge_diff_threshold_percent/100.0;
	params->region_growing_threshold=(double)params->region_growing_threshold_percent/100.0;
	params->region_diff_threshold=(double)params->region_diff_threshold_percent/100.0;		
	params->max_avg_diff=(double)(params->max_avg_diff_10)/10.0;

	params->canny_high_thr=params->canny_low_thr*params->canny_ratio;

	printParams();

	if(SHOW_STATUS_ABND)
		cout<<"Done AbandonmentAnalysis::updateParams \n";
}

void AbandonmentAnalysis::updateAbandonmentMethod(int) {	

	cout<<"\n";
	if(params->abandonment_method==ABND_REGION_GROWING){
		cout<<"Using region growing for abandonment analysis.\n";
	}else if(params->abandonment_method==ABND_GRADIENT_EDGE_DETECTION){
		cout<<"Using gradient edge detection for abandonment analysis.\n";
	}else if(params->abandonment_method==ABND_CANNY_EDGE_DETECTION){
		cout<<"Using Canny edge detection for abandonment analysis.\n";
	}else{
		cout<<"Abandonment analysis disabled.\n";
	}
	cout<<"\n";
	abandonment_updated=true;
}

void AbandonmentAnalysis::updateSimilarityMethod(int) {	

	cout<<"\n";
	if(params->abandonment_method!=ABND_REGION_GROWING)
		return;

	if(params->similarity_method==REGION_SIMILARITY){
		cout<<"Using comparison with the existing region as region growing similarity metric.\n";
	}else if(params->similarity_method==PIXEL_SIMILARITY){
		cout<<"Using comparison with the adjacent pixel as region growing similarity metric.\n";
	}else if(params->similarity_method==PIXEL_COMPARISON){
		cout<<"Using comparison with adjacent pixels on both sides as region growing similarity metric.\n";	
	}
	cout<<"\n";
	abandonment_updated=true;
}

void AbandonmentAnalysis::showAbandonmentHelpWindow(int mouse_event,int x,int y,int flags,void* param){

	if(mouse_event==CV_EVENT_LBUTTONDOWN){
		cvNamedWindow(abandonment_help_window,1);
		cvShowImage(abandonment_help_window,abandonment_help_image);
	}else if(mouse_event==CV_EVENT_RBUTTONDOWN){
		cvDestroyWindow(abandonment_help_window);
	}
}

void AbandonmentAnalysis::initWindow(char *window_name){

	if(SHOW_STATUS_ABND)
		cout<<"Starting AbandonmentAnalysis::initWindow \n";
	
	cvNamedWindow( window_name,1);

	cvCreateTrackbar("abandonment",window_name,&(params->abandonment_method),3,updateAbandonmentMethod);

	if(params->abandonment_method==ABND_REGION_GROWING){
		cvCreateTrackbar("similarity",window_name,&(params->similarity_method),3,updateSimilarityMethod);
	}
	if(params->abandonment_method==ABND_GRADIENT_EDGE_DETECTION){
		cvCreateTrackbar("gradMin",window_name,&(params->gradient_min_thr),255,NULL);
	}

	if(params->abandonment_method==ABND_CANNY_EDGE_DETECTION){
		cvCreateTrackbar("cannyRatio",window_name,&(params->canny_ratio),10,updateParams);
		cvCreateTrackbar("cannyLow",window_name,&(params->canny_low_thr),500,updateParams);
	}

	if((params->abandonment_method==ABND_CANNY_EDGE_DETECTION)||(params->abandonment_method==ABND_GRADIENT_EDGE_DETECTION))
		cvCreateTrackbar("edgeDiff%",window_name,&(params->edge_diff_threshold_percent),500,updateParams);

	if(params->abandonment_method==ABND_REGION_GROWING){

		cvCreateTrackbar("erosionWidth",window_name,&(params->erosion_width),100,updateParams);
		cvCreateTrackbar("regionDiff%",window_name,&(params->region_diff_threshold_percent),500,updateParams);

		if(params->similarity_method==REGION_SIMILARITY)
			cvCreateTrackbar("regionGrow%",window_name,&(params->region_growing_threshold_percent),500,updateParams);	

		if(params->similarity_method==PIXEL_SIMILARITY)
			cvCreateTrackbar("pixelDiff",window_name,&(params->max_squared_diff),500,updateParams);
	}
    cvCreateTrackbar("stillPerson",window_name,&(params->detect_still_person),1,updateParams);
	cvCreateTrackbar("maxAvg10",window_name,&(params->max_avg_diff_10),500,updateParams);

	cvSetMouseCallback(window_name,showAbandonmentHelpWindow,(void*)this);

	if(SHOW_STATUS_ABND)
		cout<<"Done AbandonmentAnalysis::initWindow \n";
}









