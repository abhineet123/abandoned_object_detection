#include "blobTracking.hpp"

match_struct* BlobTracking::matching_params;
track_struct* BlobTracking::tracking_params;
double BlobTracking::max_dist_sqr;
bool BlobTracking::area_method_updated;
bool BlobTracking::diff_method_updated;

char* BlobTracking::track_help_window;
char* BlobTracking::match_help_window;

IplImage* BlobTracking::track_help_image;
IplImage* BlobTracking::match_help_image;

BlobTracking::BlobTracking(CvSize image_size,match_struct *matching_params_init,track_struct *tracking_params_init){	

	state_variables_initialized=false;	
	initStateVariables(image_size);
	//cvSet(tracked_blobs_image.Ptr(),CV_RGB(255,255,255));	

	track_help_window=new char[MAX_NAME];
	sprintf(track_help_window,"Blob Tracking Help");

	match_help_window=new char[MAX_NAME];
	sprintf(match_help_window,"Blob Matching Help");

	track_help_image=cvLoadImage("help/track_help.jpg");
	match_help_image=cvLoadImage("help/match_help.jpg");

	abandoned_col=CV_RGB(255,0,0);
	static_col=CV_RGB(255,128,0);
	tracked_col=CV_RGB(127,255,127);
	occluded_col=CV_RGB(0,0,255);
	missed_col=CV_RGB(255,255,255);
	removed_col=CV_RGB(0,0,0);
	still_col=CV_RGB(175,175,175);

	matching_params=new match_struct;
	initParamsMatch(matching_params_init);
	updateParamsMatch(0);

	tracking_params=new track_struct;		
	initParamsTrack(tracking_params_init);
	updateParamsTrack(0);	
}
BlobTracking::~BlobTracking(){

	clearStateVariables();

	delete(track_help_window);
	delete(match_help_window);

	delete(matching_params);
	delete(tracking_params);

}
void BlobTracking::initStateVariables(CvSize image_size){

	tracked_blob_count=0;

	clearStateVariables();

	tracked_blobs=new CBlobResult();

	tracked_blobs_image = cvCreateImage(image_size, IPL_DEPTH_8U, 3);
	correspondence_image = cvCreateImage(image_size, IPL_DEPTH_8U, 3);

	diagnostic_image_frg=cvCreateImage(image_size, IPL_DEPTH_8U, 3);
	diagnostic_image_frg_big=cvCreateImage(cvSize(2*image_size.width,2*image_size.height),IPL_DEPTH_8U, 3);
	diagnostic_image_tracked=cvCreateImage(image_size, IPL_DEPTH_8U, 3);
	diagnostic_image_tracked_big=cvCreateImage(cvSize(2*image_size.width,2*image_size.height),IPL_DEPTH_8U, 3);

	tracked_mask=cvCreateImage(image_size, IPL_DEPTH_8U, 1);
	static_mask=cvCreateImage(image_size, IPL_DEPTH_8U, 1);

	current_bounding_box_mask=cvCreateImage(image_size, IPL_DEPTH_8U, 1);
	temp_img_gray=cvCreateImage(image_size, IPL_DEPTH_8U, 1);

	max_dist_sqr=(double)((image_size.width*image_size.width)+(image_size.height*image_size.height));
	temp_selection_image=cvCreateImage(image_size,IPL_DEPTH_8U,3);

	area_method_updated=false;
	diff_method_updated=false;
	state_variables_initialized=true;

}

void BlobTracking::clearStateVariables(){

	if(!state_variables_initialized)
		return;

	delete(tracked_blobs);

	cvReleaseImage(&tracked_blobs_image);
	cvReleaseImage(&temp_img_gray);
	cvReleaseImage(&temp_selection_image);

	correspondence_image.freeImage();
	diagnostic_image_frg.freeImage();
	diagnostic_image_tracked.freeImage();
	diagnostic_image_tracked_big.freeImage();
	static_mask.freeImage();
	tracked_mask.freeImage();
	current_bounding_box_mask.freeImage();

	state_variables_initialized=false;
}

void BlobTracking::showBlobs(IplImage *img,CBlobResult *blobs,CvScalar blob_color){

	CBlob *current_blob;
	CvRect bounding_box;
	CvPoint min_point,max_point,mid_point;
	CvFont font;
	cvInitFont( &font, CV_FONT_HERSHEY_COMPLEX_SMALL, 0.6, 0.6, 0, 1, 8);
	int blob_count=blobs->GetNumBlobs();

	char blob_text[50];
	int line_thickness=1;
	int line_type=8;
	int shift=0;

	for(int i=0;i<blob_count;i++){

		current_blob=blobs->GetBlob(i);
		bounding_box=current_blob->GetBoundingBox();
		min_point.x=bounding_box.x;
		min_point.y=bounding_box.y;		
		max_point.x=min_point.x+bounding_box.width;
		max_point.y=min_point.y+bounding_box.height;
		mid_point.x=min_point.x+(int)(bounding_box.width/2.0);
		mid_point.y=min_point.y+(int)(bounding_box.height/2.0);	
		cvRectangle(img,min_point,max_point,blob_color,line_thickness,line_type,shift);
		sprintf(blob_text,"%d",i);
		cvPutText(img,blob_text,mid_point,&font,blob_color);
	}
}

void BlobTracking::displayBlobsAndDistances(RgbImage &frg_img,CBlobResult *frg_blobs,CvScalar frg_color,CvScalar tracked_color){

	int frg_blob_count=frg_blobs->GetNumBlobs();
	cvCopy(frg_img.Ptr(), diagnostic_image_frg.Ptr());
	cvCopy(frg_img.Ptr(), diagnostic_image_tracked.Ptr());

	showBlobs(diagnostic_image_frg.Ptr(),frg_blobs,frg_color);
	showBlobs(diagnostic_image_tracked.Ptr(),tracked_blobs,tracked_color);
	printBlobDistances();
	if(tracked_blob_count>0){
		cout<<"\nTracked blob Areas:\n";
		printBlobAreas(tracked_blobs,tracked_blob_count);
	}
	if(frg_blob_count>0){
		cout<<"\nForeground blob Areas:\n";
		printBlobAreas(frg_blobs,frg_blob_count);
	}

	cvResize(diagnostic_image_frg.Ptr(),diagnostic_image_frg_big.Ptr());
	cvResize(diagnostic_image_tracked.Ptr(),diagnostic_image_tracked_big.Ptr());

	cvShowImage( "Blobs Foreground", diagnostic_image_frg_big.Ptr());
	cvShowImage( "Blobs Tracked", diagnostic_image_tracked_big.Ptr());

	cvWaitKey(0);
}

void BlobTracking::clearBlobDistances(){	
	int mat_size=blob_distance_mat.size();
	for(int i=mat_size-1;i>=0;i--){
		blob_distance_mat[i].clear();
	}
	blob_distance_mat.clear();

	min_distance_index.clear();
}


void BlobTracking::updateBlobDistances(CBlobResult *frg_blobs){

	clearBlobDistances();

	min_distance_index.resize(tracked_blob_count);	
	blob_distance_mat.resize(tracked_blob_count);

	int current_blob_count=frg_blobs->GetNumBlobs();

	for(int i=0;i<tracked_blob_count;i++){
		blob_distance_mat[i].resize(current_blob_count);
		CBlob *current_tracked_blob=tracked_blobs->GetBlob(i);
		int min_index=0;
		for(int j=0;j<current_blob_count;j++){
			CBlob *current_frg_blob=frg_blobs->GetBlob(j);
			double current_blob_distance=current_tracked_blob->getSquaredDistance(current_frg_blob,matching_params->blob_distance_measure);
			blob_distance_mat[i][j]=current_blob_distance;
			if((j!=min_index)&&(blob_distance_mat[i][min_index]>current_blob_distance))
				min_index=j;

		}
		min_distance_index[i]=min_index;
	}
}

void BlobTracking::updateBlobAreas(CBlobResult *frg_blobs){

	int current_blob_count=frg_blobs->GetNumBlobs();	
	for(int i=0;i<current_blob_count;i++){		
		CBlob *current_blob=frg_blobs->GetBlob(i);
		if(matching_params->blob_area_measure==BLOB_AREA){
			current_blob->blob_area=current_blob->Area();
		}else if(matching_params->blob_area_measure==BOUNDING_BOX_AREA){
			CvRect bounding_box=current_blob->GetBoundingBox();
			current_blob->blob_area=(double)(bounding_box.height*bounding_box.width);
		}
	}
}

void BlobTracking::updateExistingBlobAreas(){

	for(int i=0;i<tracked_blob_count;i++){		
		CBlob *current_blob=tracked_blobs->GetBlob(i);
		if(matching_params->blob_area_measure==BLOB_AREA){
			current_blob->blob_area=current_blob->Area();
		}else if(matching_params->blob_area_measure==BOUNDING_BOX_AREA){
			CvRect bounding_box=current_blob->GetBoundingBox();
			current_blob->blob_area=(double)(bounding_box.height*bounding_box.width);
		}
		current_blob->blob_stat->clearAll();
		current_blob->area_moving_avg=current_blob->blob_area;
	}
}

void BlobTracking::updateExistingBlobDiff(){

	for(int i=0;i<tracked_blob_count;i++){		
		CBlob *current_blob=tracked_blobs->GetBlob(i);
		current_blob->diff_moving_avg=-1;
	}
}


void BlobTracking::printBlobAreas(CBlobResult *blobs,int no_of_blobs){
	CBlob *current_blob;
	for(int i=0;i<no_of_blobs;i++){

		current_blob=blobs->GetBlob(i);
		cout<<current_blob->blob_area<<"\t";	
	}
	cout<<"\n";
}

void BlobTracking::printBlobDistances(){	

	std::cout<<"\nNo. of tracked blobs:"<<tracked_blob_count<<"\n";

	for(int i=0;i<tracked_blob_count;i++){

		CBlob *blob=tracked_blobs->GetBlob(i);
		int frg_blob_count=blob_distance_mat[i].size();	
		if(i==0){
			std::cout<<"No. of foreground blobs:"<<frg_blob_count<<"\n";
		}
		for(int j=0;j<frg_blob_count;j++){
			std::cout<<blob_distance_mat[i][j]<<"\t";
		}

		if(blob->matched_blob_id>=0){
			std::cout<<"Matching Blob: "<<blob->matched_blob_id<<"\n";
		}else{
			std::cout<<"No Matching Blob\n";
		}
	}

	cout<<"max_dist_sqr="<<max_dist_sqr<<"\n";
	_getch();
}

void BlobTracking::showCorrespondenceImage(CBlobResult *frg_blobs,int line_thickness,int line_type, int shift){
	CBlob *blob1,*blob2;
	CvRect bounding_box;
	CvPoint min_point,max_point;

	cvSet(correspondence_image.Ptr(),CV_RGB(255,255,255));

	for(int i=0;i<tracked_blob_count;i++){
		blob1=tracked_blobs->GetBlob(i);
		bounding_box=blob1->GetBoundingBox();
		min_point.x=bounding_box.x;
		min_point.y=bounding_box.y;		
		max_point.x=min_point.x+bounding_box.width;
		max_point.y=min_point.y+bounding_box.height;	
		cvRectangle(correspondence_image.Ptr(),min_point,max_point,CV_RGB(0,255,0),line_thickness,line_type,shift);
	}
	cvShowImage( "Correspondence", correspondence_image.Ptr());
	cout<<"Press any key to continue....\n";
	cvWaitKey(0);	


	for(int i=0;i<tracked_blob_count;i++){

		cvSet(correspondence_image.Ptr(),CV_RGB(255,255,255));

		blob1=tracked_blobs->GetBlob(i);
		bounding_box=blob1->GetBoundingBox();
		min_point.x=bounding_box.x;
		min_point.y=bounding_box.y;		
		max_point.x=min_point.x+bounding_box.width;
		max_point.y=min_point.y+bounding_box.height;	
		if(blob1->matched_blob_id<0){
			cvRectangle(correspondence_image.Ptr(),min_point,max_point,CV_RGB(255,0,0),line_thickness,line_type,shift);
		} else{
			cvRectangle(correspondence_image.Ptr(),min_point,max_point,CV_RGB(0,255,0),line_thickness,line_type,shift);
			blob2=frg_blobs->GetBlob(blob1->matched_blob_id);
			bounding_box=blob2->GetBoundingBox();
			min_point.x=bounding_box.x;
			min_point.y=bounding_box.y;		
			max_point.x=min_point.x+bounding_box.width;
			max_point.y=min_point.y+bounding_box.height;
			cvRectangle(correspondence_image.Ptr(),min_point,max_point,CV_RGB(0,255,0),line_thickness,line_type,shift);

		}
		cvShowImage( "Correspondence", correspondence_image.Ptr());
		cout<<"Press any key to continue....\n";
		cvWaitKey(0);		
	}
}

void BlobTracking::updateBoundingBoxMask(CBlobResult *frg_blobs,int frg_blob_count){
	current_bounding_box_mask.Clear();
	CBlob *current_blob;
	for(int i=0;i<frg_blob_count;i++){		
		current_blob=frg_blobs->GetBlob(i);
		fillBoundingBox(current_bounding_box_mask,current_blob);
	}
}

//----------------------------------------main tracking and matching functions---------------------------------------------------//



void BlobTracking::updateTrackedBlobs(IplImage *frg_image,CBlobResult *frg_blobs,int frg_blob_count,BwImage &frg_mask,IplImage *frg_grad_x,IplImage *frg_grad_y){

	if(SHOW_TRACKING_STATUS)
		cout<<"Starting updateTrackedBlobs \n";	

	//cout<<"tracked_blob_count="<<tracked_blob_count<<"\n";

	updateBlobDistances(frg_blobs);	
	updateBlobAreas(frg_blobs);

	if(area_method_updated){
		area_method_updated=false;
		updateExistingBlobAreas();		
	}
	if(diff_method_updated){
		diff_method_updated=false;
		updateExistingBlobDiff();
	}
	updateBoundingBoxMask(frg_blobs,frg_blob_count);	

	for(int i=0;i<tracked_blob_count;i++){

		bool found_match=false;
		CBlob *blob1=tracked_blobs->GetBlob(i);
		//area1=blob1->Area();
		for(int j=0;j<frg_blob_count;j++){

			CBlob *blob2=frg_blobs->GetBlob(j);

			//Each new blob matches at most one of the existing tracked blobs
			if(blob2->has_been_matched)
				continue;		

			if(matchBlobs(blob1,blob2,blob_distance_mat[i][j])){

				if((frg_grad_x)&&(frg_grad_y)){
					//cout<<"\nUsing gradient images for mean difference calculation.\n";
					updateMovingAverageOfDifference(blob1,frg_grad_x,frg_grad_y);
				}else{
					updateMovingAverageOfDifference(blob1,frg_image);
				}
				updateMovingAverageOfAreas(blob1,blob2);
				blob1->blob_stat->addElement(blob2->blob_area);

				/*cout<<"\n";
				cout<<"Blob "<<i<<" Areas:\t";
				cout<<"Original: "<<blob1->blob_area<<"\t";
				cout<<"Current: "<<blob2->blob_area<<"\t";
				cout<<"Running average: "<<blob1->blob_stat->mean()<<"\t";
				cout<<"Moving average: "<<blob1->area_moving_avg<<"\t";
				cout<<"\n";
				_getch();*/

				//blob1=blob2;
				/*cout<<"\nBlob "<<i<<" :current_mean_diff="<<blob1->current_mean_diff<<"\n";
				char window_name[50];
				sprintf(window_name,"frg_image for blob %d",i);
				cvShowImage(window_name,frg_image);
				sprintf(window_name,"static_image for blob %d",i);
				cvShowImage(window_name,blob1->static_img);*/

				blob1->matched_blob_id=j;
				blob1->hit_count++;
				blob1->miss_count=0;					
				blob1->occluded_count=0;				
				blob1->is_occluded=false;

				blob2->has_been_matched=true;

				//each existing tracked blob matches with at most one new blob
				found_match=true;
				break;
			}
		}
		// if an existing blob does not match with any of the new blobs, check if it is occluded (provided that is has been in the scene long enough)
		if(!found_match){
			if((blob1->hit_count>=tracking_params->min_hit_count_for_occ)&&(isOccluded(blob1,current_bounding_box_mask))){				
				blob1->is_occluded=true;
				blob1->occluded_count++;
				blob1->hit_count++;
				blob1->miss_count=0;
			}else{
				blob1->is_occluded=false;
				blob1->matched_blob_id=-1;
				blob1->miss_count++;
			}
		}
	}
	//printBlobDistances();
	if(SHOW_TRACKING_STATUS)
		cout<<"Done updateTrackedBlobs \n";
}

bool BlobTracking::processTrackedBlobs(IplImage *frg_image,CBlobResult *frg_blobs,int frg_blob_count,IplImage *frg_grad_x,IplImage *frg_grad_y){

	if(SHOW_TRACKING_STATUS)
		cout<<"\nStarting processTrackedBlobs tracked_blob_count="<<tracked_blob_count<<"\n";

	bool found_ack=false;
	//CBlob *new_blob;	

	/*if(tracked_blob_count!=tracked_blobs.GetNumBlobs()){
	cout<<"Something weird going on in blob tracking.\n";
	cout<<"tracked_blob_count="<<tracked_blob_count<<"\t";
	cout<<"tracked_blobs size="<<tracked_blobs.GetNumBlobs()<<"\n";
	_getch();
	}*/

	for(int i=tracked_blob_count-1;i>=0;i--){

		CBlob *current_blob=tracked_blobs->GetBlob(i);

		if(current_blob->is_ack){
			clearBlob(current_blob);
			tracked_blobs->RemoveBlob(i);
			tracked_blob_count--;
			if((current_blob->is_static)||(current_blob->is_removed)||(current_blob->is_abandoned)||(current_blob->is_still_person))
				cout<<"Removing blob "<<i<<" since it has been acknowledged.\n";
			continue;
		}
		int max_occ_count=tracking_params->max_occ_count;
		int max_miss_count=tracking_params->max_miss_count;

		//objects that have been stationary in the scene for a significant duration are allowed more occlusions and misses before they are eliminated.
		if(current_blob->is_static){
			max_occ_count=max_occ_count*tracking_params->static_factor_occ;
			max_miss_count=max_miss_count*tracking_params->static_factor_miss;
		}
		if(current_blob->is_occluded){
			if(current_blob->occluded_count>max_occ_count){
				//cout<<"\nRemoved Blob "<<i<<" due to too many occlusions.\n\n";
				clearBlob(current_blob);
				tracked_blobs->RemoveBlob(i);
				tracked_blob_count--;
				if((current_blob->is_static)||(current_blob->is_removed)||(current_blob->is_abandoned)||(current_blob->is_still_person))
					cout<<"Removing blob "<<i<<" since it has been occluded too long.\n";
			}
			continue;
		}
		if((current_blob->miss_count>current_blob->hit_count)||(current_blob->miss_count>max_miss_count)){
			//cout<<"\nRemoved Blob "<<i<<" due to too many misses.\n\n";
			clearBlob(current_blob);
			tracked_blobs->RemoveBlob(i);
			tracked_blob_count--;
			continue;
			if((current_blob->is_static)||(current_blob->is_removed)||(current_blob->is_abandoned)||(current_blob->is_still_person))
				cout<<"Removing blob "<<i<<" since it has been undetected for too long.\n";
		}		

		/*if((current_blob->is_static)&&(current_blob->current_mean_diff>tracking_params->max_mean_diff)){			
		clearBlob(current_blob);
		tracked_blobs->RemoveBlob(i);
		tracked_blob_count--;			
		continue;			
		}*/

		if(current_blob->current_mean_diff>tracking_params->max_mean_diff){
			//cout<<"\nRemoved Blob "<<i<<" due to appearance mismatch.\n\n";
			clearBlob(current_blob);
			tracked_blobs->RemoveBlob(i);
			tracked_blob_count--;			
			continue;
			if((current_blob->is_static)||(current_blob->is_removed)||(current_blob->is_abandoned)||(current_blob->is_still_person))
				cout<<"Removing blob "<<i<<"because of appearance mismatch.\n";
			//_getch();
		}

		if((!current_blob->is_static)&&(current_blob->hit_count>=tracking_params->min_hit_count_for_static)){
			current_blob->is_static=true;
			//initializeBlobProperties(current_blob,frg_image,frg_grad_x,frg_grad_y);
			continue;		
		}

		if(current_blob->is_removed){
			current_blob->track_count++;
			if(current_blob->track_count>tracking_params->max_removed_count){
				current_blob->is_ack=true;		
				found_ack=true;
			}			
			continue;
		}

		if(current_blob->is_abandoned){
			current_blob->track_count++;
			if(current_blob->track_count>tracking_params->max_abandoned_count){
				current_blob->is_ack=true;	
				found_ack=true;
			}			
			continue;
		}

		if(current_blob->is_state_change){
			current_blob->is_ack=true;
			found_ack=true;
			continue;
		}

		if(current_blob->is_still_person){			
			continue;
		}

		//checking for alarm condition
		if(current_blob->hit_count>=tracking_params->min_hit_count_for_abandoned){
			/*cout<<"\nHere we are.\n";
			cout<<"min_hit_count="<<tracking_params->min_hit_count_for_abandoned<<"\t";
			cout<<"hit_count="<<current_blob->hit_count<<"\n";
			_getch();*/
			current_blob->is_candidate=true;			
		}
	}

	//adding new unmatched foreground blobs to the list of tracked blobs
	for(int i=0;i<frg_blob_count;i++){
		CBlob *current_blob=frg_blobs->GetBlob(i);
		if(!current_blob->has_been_matched){

			//new_blob=new CBlob();
			//new_blob=current_blob;

			/*cout<<"Adding new blob with default values:\n";
			cout<<"hit_count="<<current_blob->hit_count<<"\t";
			cout<<"miss_count="<<current_blob->miss_count<<"\t";
			cout<<"is_abandoned="<<current_blob->is_abandoned<<"\t";
			cout<<"is_static="<<current_blob->is_static<<"\n"; 
			cout<<"Press any key to continue.\n";
			_getch();*/

			tracked_blobs->AddBlob(current_blob);
			initializeBlob(tracked_blobs->GetBlob(tracked_blob_count));
			initializeBlobProperties(tracked_blobs->GetBlob(tracked_blob_count),frg_image,frg_grad_x,frg_grad_y);
			tracked_blob_count++;
		}
	}

	if(SHOW_TRACKING_STATUS)
		cout<<"Done processTrackedBlobs tracked_blob_count="<<tracked_blob_count<<"\n";

	return found_ack;
}

void BlobTracking::updateBlobImage(IplImage *frg_image,int line_thickness,int line_type, int shift){

	if(SHOW_TRACKING_STATUS)
		cout<<"\nStarting updateBlobImage\n";

	CvPoint min_point,max_point;	
	CvScalar blob_col=tracked_col;
	CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_COMPLEX_SMALL, 0.5, 0.5, 0, 1, 8);

	cvCopy(frg_image,tracked_blobs_image);
	//cvSet(tracked_blobs_image.Ptr(),CV_RGB(255,255,255));
	//tracked_blobs.Clear();	

	for(int i=0;i<tracked_blob_count;i++){
		//cout<<"\nUpdating blob image with blob "<<i<<"\n";
		CBlob *current_blob=tracked_blobs->GetBlob(i);		
		//cout<<"Successfully obtained blob "<<i<<"\n";

		if(current_blob->is_ack){
			//cout<<"\n\n found ack.\n\n";
			continue;
		}

		if(current_blob->is_still_person){
			//cout<<"\nBlob "<<i<<" detected as still person \t diff_moving_avg="<<current_blob->diff_moving_avg<<"\n";
			blob_col=still_col;
			continue;
		}else if(current_blob->is_state_change){			
			//cout<<"\nBlob "<<i<<" detected as state change\t diff_moving_avg="<<current_blob->diff_moving_avg<<"\n";
			blob_col=still_col;
		}else if(current_blob->is_removed){			
			//cout<<"\nBlob "<<i<<" detected as removed\t diff_moving_avg="<<current_blob->diff_moving_avg<<"\n";
			blob_col=removed_col;
		}else if(current_blob->is_abandoned){			
			blob_col=abandoned_col;
			//cout<<"\nBlob "<<i<<" detected as abandoned\t diff_moving_avg="<<current_blob->diff_moving_avg<<"\n";	
			/*cout<<"min_hit_count="<<tracking_params->min_hit_count_for_abandoned<<"\t";
			cout<<"hit_count="<<current_blob->hit_count<<"\n";
			_getch();*/
		}else if(current_blob->is_static){
			blob_col=static_col;
		}else{
			blob_col=tracked_col;
			continue;
		}

		if(current_blob->is_occluded){
			blob_col=occluded_col;	
		}else if((current_blob->matched_blob_id==-1)&&(current_blob->hit_count>1)){			
			blob_col=missed_col;			
		}	

		/*if(current_blob->matched_blob_id==-1)
		continue;*/
		//cvRectangle(tracked_blobs_image.Ptr(),min_point,max_point,blob_col,line_thickness,line_type,shift);

		if((current_blob->is_removed)||(current_blob->is_abandoned)||(current_blob->is_static)){

			CvRect bounding_box=current_blob->GetBoundingBox();
			min_point.x=bounding_box.x;
			min_point.y=bounding_box.y;		
			max_point.x=min_point.x+bounding_box.width;
			max_point.y=min_point.y+bounding_box.height;

			cvRectangle(tracked_blobs_image, min_point, max_point,blob_col,line_thickness,line_type, shift);	

			if(current_blob->is_state_change)
				cvPutText(tracked_blobs_image,"STATE CHANGE",min_point, &font, blob_col);
			else if(current_blob->is_removed)
				cvPutText(tracked_blobs_image,"REMOVED",min_point, &font, blob_col);
			else if(current_blob->is_abandoned)
				cvPutText(tracked_blobs_image,"ABANDONED",min_point, &font, blob_col);
			else if(current_blob->is_static)
				cvPutText(tracked_blobs_image,"STATIC",min_point, &font, blob_col);

			if(current_blob->is_occluded){
				cvPutText(tracked_blobs_image,"OCCLUDED",max_point, &font, blob_col);
			} else if(current_blob->matched_blob_id==-1){
				cvPutText(tracked_blobs_image,"NOT MATCHED",max_point, &font, blob_col);
			}
			/*}else if(current_blob->is_occluded){
			CvRect bounding_box=current_blob->GetBoundingBox();				
			max_point.x=min_point.x+bounding_box.width;
			max_point.y=min_point.y+bounding_box.height;
			cvRectangle(tracked_blobs_image, min_point, max_point,blob_col,line_thickness,line_type, shift);
			cvPutText(tracked_blobs_image,"OCCLUDED",max_point, &font, blob_col);*/
		} 
	}
	if(SHOW_TRACKING_STATUS)
		cout<<"\nDone updateBlobImage\n";
}
//-----------------------------------------------------------------------------------------------------------------//


//-------------------------------------------Helper functions------------------------------------------------------//

void BlobTracking::initializeBlob(CBlob *blob){

	if(SHOW_TRACKING_STATUS)
		cout<<"\nStarting initializeBlob\n";

	blob->hit_count=1;
	blob->miss_count=blob->occluded_count=0;
	blob->is_ack=false;
	blob->is_static=false;
	blob->is_abandoned=false;
	blob->is_removed=false;	
	blob->is_still_person=false;
	blob->is_occluded=false;
	blob->is_state_change=false;

	blob->static_img=NULL;
	blob->static_img_grad_x=NULL;	
	blob->static_img_grad_y=NULL;

	blob->blob_stat=NULL;

	blob->diff_moving_avg=-1;
	blob->area_moving_avg=-1;

	CvRect bounding_box=blob->GetBoundingBox();
	double diagonal_sqr=(bounding_box.width*bounding_box.width)+(bounding_box.height*bounding_box.height);
	blob->max_dist=diagonal_sqr*matching_params->dist_threshold;
	if(SHOW_TRACKING_STATUS)
		cout<<"\nDone initializeBlob\n";
}

void BlobTracking::initializeBlobProperties(CBlob *current_blob,IplImage *frg_image,IplImage *frg_grad_x,IplImage *frg_grad_y){

	if(SHOW_TRACKING_STATUS)
		cout<<"\nStarting initializeBlobProperties\n";

	if(!current_blob->blob_stat){
		//delete(current_blob->blob_stat);
		current_blob->blob_stat=new RunningStatScalar();
	}else{
		current_blob->blob_stat->clearAll();
	}
	current_blob->blob_stat->addElement(current_blob->blob_area);

	current_blob->area_moving_avg=-1;
	updateMovingAverageOfAreas(current_blob,current_blob);

	refreshBlobImages(current_blob,frg_image,frg_grad_x,frg_grad_y);

	if(SHOW_TRACKING_STATUS)
		cout<<"\nDone initializeBlobProperties\n";
}

void BlobTracking::refreshBlobImages(CBlob *current_blob,IplImage *frg_image,IplImage *frg_grad_x,IplImage *frg_grad_y){

	if(SHOW_TRACKING_STATUS)
		cout<<"\nStarting refreshBlobImages\n";	

	if((frg_grad_x)&&(frg_grad_y)){

		if(!current_blob->static_img_grad_x)
			current_blob->static_img_grad_x=cvCreateImage(cvSize(frg_grad_x->width,frg_grad_x->height),IPL_DEPTH_8U,1);

		if(!current_blob->static_img_grad_y)
			current_blob->static_img_grad_y=cvCreateImage(cvSize(frg_grad_y->width,frg_grad_y->height),IPL_DEPTH_8U,1);

		cvCopy(frg_grad_x,current_blob->static_img_grad_x);
		cvCopy(frg_grad_y,current_blob->static_img_grad_y);
	}
	if(!(current_blob->static_img)){					
		//cout<<"\n Updating Static Image for blob "<<i<<"\n";
		current_blob->static_img=cvCreateImage(cvSize(frg_image->width,frg_image->height),IPL_DEPTH_8U,frg_image->nChannels);
	}
	cvCopy(frg_image,current_blob->static_img);


	if(SHOW_TRACKING_STATUS)
		cout<<"\nDone refreshBlobImages\n";
}

void BlobTracking::refreshTrackedBlobsImages(IplImage *frg_image,IplImage *frg_grad_x,IplImage *frg_grad_y){

	for(int i=0;i<tracked_blob_count;i++){		

		CBlob *current_blob=tracked_blobs->GetBlob(i);
		if((current_blob->is_occluded)||(current_blob->miss_count>0))
			continue;

		refreshBlobImages(current_blob,frg_image,frg_grad_x,frg_grad_y);
	}
}

void  BlobTracking::clearBlob(CBlob *current_blob){

	if(SHOW_TRACKING_STATUS)
		cout<<"\nStarting clearBlob\n";

	if(current_blob->static_img_grad_x)
		cvReleaseImage(&current_blob->static_img_grad_x);

	if(current_blob->static_img_grad_y)
		cvReleaseImage(&current_blob->static_img_grad_y);

	if(current_blob->static_img)
		cvReleaseImage(&current_blob->static_img);

	if(SHOW_TRACKING_STATUS)
		cout<<"\nDone clearBlob\n";
}

bool BlobTracking::matchBlobs(CBlob *blob1,CBlob *blob2,double blob_dist){

	if(SHOW_TRACKING_STATUS)
		cout<<"\nStarting matchBlobs\n";

	//double blob_dist_frac=blob_dist/max_dist;
	if(blob_dist>blob1->max_dist){
		/*cout<<"\nBlob positions not matching: blob_dist="<<blob_dist<<"\t";
		cout<<"max_dist="<<blob1->max_dist<"\n";*/
		return false;
	}

	double blob1_area=blob1->blob_area;

	if((blob1->is_static)&&(matching_params->blob_area_type!=AREA_CURRENT)){

		/*cout<<"\n";		
		cout<<"Original: "<<blob1->blob_area<<"\t";		
		cout<<"Running average: "<<blob1->blob_stat->mean()<<"\t";
		cout<<"Moving average: "<<blob1->area_moving_avg<<"\t";
		cout<<"\n";
		_getch();*/

		if(matching_params->blob_area_type==AREA_MOVING_AVG)
			blob1_area=blob1->area_moving_avg;
		else
			blob1_area=blob1->blob_stat->mean();
	}

	double area_diff_frac=fabs(blob1_area-blob2->blob_area)/blob1_area;
	if(area_diff_frac>matching_params->size_threshold){

		//cout<<"\nBlob sizes not matching: area_diff_frac="<<area_diff_frac<<"\n";
		return false;
	}

	if(SHOW_TRACKING_STATUS)
		cout<<"\nDone matchBlobs\n";

	return true;
}

inline bool BlobTracking::matchBlobLocations(int blob1_index,int blob2_index){
	if(blob_distance_mat[blob1_index][blob2_index]<=matching_params->dist_threshold_sqr){
		return true;
	}
	return false;
}

inline bool BlobTracking::matchBlobSizes(CBlob *blob1,CBlob *blob2){

	double area_diff_frac=fabs(blob1->blob_area-blob2->blob_area)/blob1->blob_area;

	if(area_diff_frac<=matching_params->size_threshold)
		return true;
	return false;
}

inline bool BlobTracking::matchBlobAppearances(CBlob *blob1,CBlob *blob2){

	return true;
}

void BlobTracking::updateMovingAverageOfDifference(CBlob *blob,IplImage *frg_image){
	if(SHOW_TRACKING_STATUS)
		cout<<"Starting updateMovingAverageOfDifference no grad \n";	

	double mean_diff=getMeanDifference(blob,frg_image,NO_GRAD);
	double smoothing_factor=matching_params->moving_avg_alpha;
	//int static_hit_count=blob->hit_count-tracking_params->min_hit_count_for_static;

	if(blob->diff_moving_avg==-1){
		blob->diff_moving_avg=mean_diff;
	} else{
		blob->diff_moving_avg=(smoothing_factor*mean_diff)+((1-smoothing_factor)*blob->diff_moving_avg);

	}
	blob->current_mean_diff=mean_diff;	

	if(SHOW_TRACKING_STATUS)
		cout<<"\nDone updateMovingAverageOfDifference no grad\n";
}

void BlobTracking::updateMovingAverageOfDifference(CBlob *blob,IplImage *frg_grad_x,IplImage *frg_grad_y){
	if(SHOW_TRACKING_STATUS)
		cout<<"Starting updateMovingAverageOfDifference \n";	

	double mean_diff=(getMeanDifference(blob,frg_grad_x,GRAD_X)+getMeanDifference(blob,frg_grad_y,GRAD_Y))/2.0;
	double smoothing_factor=matching_params->moving_avg_alpha;
	//int static_hit_count=blob->hit_count-tracking_params->min_hit_count_for_static;

	if(blob->diff_moving_avg==-1){
		blob->diff_moving_avg=mean_diff;
	} else{
		blob->diff_moving_avg=(smoothing_factor*mean_diff)+((1-smoothing_factor)*blob->diff_moving_avg);

	}
	blob->current_mean_diff=mean_diff;

	if(SHOW_TRACKING_STATUS)
		cout<<"\nDone updateMovingAverageOfDifference\n";
}

void BlobTracking::updateMovingAverageOfAreas(CBlob *blob1,CBlob *blob2){
	if(SHOW_TRACKING_STATUS)
		cout<<"Starting updateMovingAverageOfAreas \n";	

	//int static_hit_count=blob1->hit_count-tracking_params->min_hit_count_for_static;

	if(blob1->area_moving_avg==-1){
		blob1->area_moving_avg=blob2->blob_area;
	} else{
		blob1->area_moving_avg=(matching_params->moving_avg_alpha*blob2->blob_area)+((1-matching_params->moving_avg_alpha)*blob1->area_moving_avg);
	}	
	if(SHOW_TRACKING_STATUS)
		cout<<"\nDone updateMovingAverageOfAreas\n";
}

double BlobTracking::getMeanDifference(CBlob *blob,IplImage *frg_image,int diff_img){

	if(SHOW_TRACKING_STATUS)
		cout<<"Starting getMeanDifference \n";	

	IplImage *static_image;

	/*if(frg_image->nChannels==1){		
	cvCvtColor(blob->static_img,temp_img_gray,CV_BGR2GRAY);
	static_image=temp_img_gray;
	} else {
	static_image=blob->static_img;
	}*/
	if(diff_img==GRAD_X){
		if(!blob->static_img_grad_x){
			cout<<"\nstatic_img_grad_x not initialized \n";
			return 0; 
		}
		static_image=blob->static_img_grad_x;
	}else if(diff_img==GRAD_Y){
		if(!blob->static_img_grad_y){
			cout<<"\n static_img_grad_y not initialized \n";
			return 0; 
		}
		static_image=blob->static_img_grad_y;
	}else if(diff_img==NO_GRAD){
		if(!blob->static_img){
			cout<<"\n static_img not initialized \n";
			return 0; 
		}
		static_image=blob->static_img;
	}else{
		cout<<"Invalid differencing image specified.\n";
		exit(0);
	}

	if((diff_img==GRAD_X)||(diff_img==GRAD_Y)){
		if(frg_image->nChannels!=1){
			cout<<"Differencing image has incorrect depth.\n";
			exit(0);
		}
	}
	/*cvShowImage("frg_image",frg_image);
	cvShowImage("static_image",static_image);*/	

	CvRect bounding_box=blob->GetBoundingBox();
	int min_row=bounding_box.y;
	int max_row=min_row+bounding_box.height;
	int min_col=bounding_box.x;
	int max_col=min_col+bounding_box.width;	
	int no_of_pixels=bounding_box.width*bounding_box.height;

	double total_diff=0.0;

	for(int r=min_row;r<max_row;r++){

		for(int c=min_col;c<max_col;c++){

			int current_location=r*frg_image->widthStep+c*frg_image->nChannels;	
			double total_pixel_diff=0.0;

			for(int ch=0;ch<frg_image->nChannels;ch++){				
				unsigned char channel_diff=abs((unsigned char)static_image->imageData[current_location+ch]-(unsigned char)frg_image->imageData[current_location+ch]);
				total_pixel_diff+=(double)channel_diff;
			}
			double mean_pixel_diff=total_pixel_diff/(double)frg_image->nChannels;
			total_diff+=mean_pixel_diff;
		}
	}

	double mean_diff=total_diff/(double)no_of_pixels;

	if(SHOW_TRACKING_STATUS)
		cout<<"\nDone getMeanDifference\n";

	return mean_diff;
}

void BlobTracking::getBlobMeanStd(CBlobResult *frg_blobs,int frg_blob_count,RgbImage &frg_image){
	CBlob *current_blob;
	double blob_mean,blob_std;
	for(int i=0;i<frg_blob_count;i++){
		current_blob=frg_blobs->GetBlob(i);
		blob_mean=current_blob->Mean(frg_image.Ptr());
		blob_std=current_blob->StdDev(frg_image.Ptr());
	}
}


// returns true is more than the specified fraction of the blob's bounding box is in the foreground.
bool BlobTracking::isOccluded(CBlob *blob,BwImage &frg_mask){

	if(SHOW_TRACKING_STATUS)
		cout<<"Starting isOccluded \n";


	CvRect bounding_box=blob->GetBoundingBox();
	int min_x=bounding_box.x;
	int min_y=bounding_box.y;
	int max_x=min_x+bounding_box.width;
	int max_y=min_y+bounding_box.height;
	int frg_count=0,total_count=0;

	for(int x=min_x;x<=max_x;x++){
		for(int y=min_y;y<=max_y;y++){

			if(frg_mask(y,x)==FOREGROUND_COLOR){
				frg_count++;
			}
			total_count++;
		}
	}
	double frg_fraction=(double)frg_count/(double)total_count;

	if(frg_fraction>=matching_params->occ_threshold)
		return true;

	if(SHOW_TRACKING_STATUS)
		cout<<"\nDone isOccluded\n";

	return false;	
}

//-----------------------------------------------------------------------------------------------------------------//

void BlobTracking::eliminateStillPersonBlobs(){

	CBlob *current_blob;
	for(int i=0;i<tracked_blob_count;i++){
		current_blob=tracked_blobs->GetBlob(i);

		if(current_blob->is_still_person)
			current_blob->is_ack=true;	
	}
}

void BlobTracking::eliminateRemovedBlobs(){

	CBlob *current_blob;
	for(int i=0;i<tracked_blob_count;i++){

		current_blob=tracked_blobs->GetBlob(i);

		if(current_blob->is_still_person)
			continue;

		if(current_blob->is_removed)
			current_blob->is_ack=true;
	}
}

void BlobTracking::eliminateAbandonedBlobs(){

	CBlob *current_blob;
	for(int i=0;i<tracked_blob_count;i++){
		current_blob=tracked_blobs->GetBlob(i);

		if((current_blob->is_removed)||(current_blob->is_still_person))
			continue;

		if(current_blob->is_abandoned)
			current_blob->is_ack=true;	
	}
}

void BlobTracking::eliminateStaticBlobs(){

	CBlob *current_blob;
	for(int i=0;i<tracked_blob_count;i++){
		current_blob=tracked_blobs->GetBlob(i);

		if((current_blob->is_removed)||(current_blob->is_abandoned)||(current_blob->is_still_person))
			continue;

		if(current_blob->is_static)
			current_blob->is_ack=true;	
	}
}

void BlobTracking::fillBoundingBox(BwImage &img,CBlob *blob) {
	CvRect bounding_box=blob->GetBoundingBox();
	CvPoint min_point,max_point;
	min_point.x=bounding_box.x;
	min_point.y=bounding_box.y;		
	max_point.x=min_point.x+bounding_box.width;
	max_point.y=min_point.y+bounding_box.height;
	for(int r=min_point.y;r<=max_point.y;r++){
		for(int c=min_point.x;c<=max_point.x;c++){
			img(r,c)=FOREGROUND_COLOR;
		}
	}
}

// marks all the pixels in the bounding boxes of each of the currently tracked blobs as FOREGROUND (or 255)
void BlobTracking::updateTrackedStaticMask(){
	CBlob *current_blob;	
	static_mask.Clear();
	tracked_mask.Clear();
	for(int i=0;i<tracked_blob_count;i++){
		current_blob=tracked_blobs->GetBlob(i);
		if(current_blob->is_ack)
			continue;
		if(current_blob->miss_count>0)
			continue;
		//if((current_blob->is_static)||(current_blob->is_abandoned)||(current_blob->is_removed)){
		//current_blob->FillBlob( tracked_mask.Ptr(), CV_RGB(255,255,255));			
		fillBoundingBox(tracked_mask,current_blob);
		if(current_blob->is_static)
			fillBoundingBox(static_mask,current_blob);

		//}
	}
}

void  BlobTracking::showBlob(IplImage *img,CBlob *blob,CvScalar &blob_col){

	//cout<<"Start showBlob\n";

	CvPoint point_1,point_2;
	//parameters
	int line_thickness=1;
	int line_type=8;
	int shift=0;

	CvRect current_bounding_box=blob->GetBoundingBox();
	point_1.x=current_bounding_box.x;
	point_1.y=current_bounding_box.y;
	point_2.x=current_bounding_box.x+current_bounding_box.width;
	point_2.y=current_bounding_box.y+current_bounding_box.height;

	cvRectangle(img, point_1, point_2,blob_col,line_thickness,line_type, shift);
	//cout<<"Done showBlob\n";
}

int BlobTracking::showBoundingBoxes(IplImage *img,int object_type){

	//cout<<"Start showBoundingBoxes\n";
	int obj_count=0;

	for(int i=0;i<tracked_blob_count;i++){
		CBlob *current_blob=tracked_blobs->GetBlob(i);
		if(object_type==OBJ_ALL){
			if((current_blob->miss_count>0)||(current_blob->occluded_count>0))
				continue;
			showBlob(img,current_blob,tracked_col);
			obj_count++;
		}else if(object_type==OBJ_STATIC){
			if(current_blob->is_abandoned||current_blob->is_removed)
				continue;
			if(current_blob->is_static){
				showBlob(img,current_blob,static_col);
				obj_count++;
			}
		}else if(object_type==OBJ_ABANDONED){			
			if(current_blob->is_removed)
				continue;
			if(current_blob->is_abandoned){
				showBlob(img,current_blob,abandoned_col);
				obj_count++;

			}
		}else if(object_type==OBJ_REMOVED){

			if(current_blob->is_removed){
				showBlob(img,current_blob,removed_col);
				obj_count++;

			}
		}
	}
	//cout<<"Done showBoundingBoxes\n";

	return obj_count;
}
int BlobTracking::getSelectionImage(IplImage *current_frame,IplImage *selection_image,int object_type,int resize_image){
	//cout<<"Start getSelectionImage\n";
	//selection_image=cvCreateImage(cvSize(temp_selection_image->width,temp_selection_image->height),IPL_DEPTH_8U,3);
	int obj_count=0;
	if(resize_image){
		//cout<<"Resizing image\n";
		if((current_frame->width!=temp_selection_image->width)||(current_frame->height!=temp_selection_image->height)){
			cout<<"Incompatible images for copying.\n";
			_getch();
		}
		cvCopy(current_frame,temp_selection_image);
		/*cvShowImage("Naziora",temp_selection_image);*/
		obj_count=showBoundingBoxes(temp_selection_image,object_type);

		/*cvShowImage("Nazio",temp_selection_image);
		cvWaitKey(0);*/
		cvResize(temp_selection_image,selection_image);
		//cout<<"Done Resizing\n";

	}else{
		cvCopy(current_frame,selection_image);
		obj_count=showBoundingBoxes(selection_image,object_type);
	}
	//cout<<"Done getSelectionImage\n";

	return obj_count;
}

bool BlobTracking::isContainedInsideBlob(CvPoint point,CBlob *blob){
	CvRect bounding_box=blob->GetBoundingBox();
	if((point.x>=bounding_box.x)&&(point.x<=bounding_box.x+bounding_box.width))
		if((point.y>=bounding_box.y)&&(point.y<=bounding_box.y+bounding_box.height))
			return true;
	return false;
}

CBlob* BlobTracking::getBlobContainingPoint(CvPoint blob_point,int object_type){

	for(int i=0;i<tracked_blob_count;i++){

		CBlob *current_blob=tracked_blobs->GetBlob(i);

		if(object_type==OBJ_ALL){
			if((current_blob->miss_count>0)||(current_blob->occluded_count>0))
				continue;
			if(isContainedInsideBlob(blob_point,current_blob))
				return current_blob;
		}else if(object_type==OBJ_STATIC){
			if(current_blob->is_abandoned||current_blob->is_removed)
				continue;
			if(current_blob->is_static){
				if(isContainedInsideBlob(blob_point,current_blob))
					return current_blob;				
			}
		}else if(object_type==OBJ_ABANDONED){			
			if(current_blob->is_removed)
				continue;
			if(current_blob->is_abandoned){
				if(isContainedInsideBlob(blob_point,current_blob))
					return current_blob;
			}
		}else if(object_type==OBJ_REMOVED){

			if(current_blob->is_removed){
				if(isContainedInsideBlob(blob_point,current_blob))
					return current_blob;
			}
		}
	}
	return NULL;
}
//-----------------------------------------User interface functions-----------------------------------------//

void BlobTracking::printMatchingParameters(){

	cout<<"\n";

	cout<<"Blob matching parameters updated:\n";

	cout<<"use_gradient_diff="<<matching_params->use_gradient_diff<<"\t";

	cout<<"occ_threshold="<<matching_params->occ_threshold<<"\t";
	cout<<"size_threshold="<<matching_params->size_threshold<<"\t";
	cout<<"dist_threshold="<<matching_params->dist_threshold<<"\t";
	cout<<"appearance_threshold="<<matching_params->appearance_threshold<<"\t";
	cout<<"moving_avg_alpha="<<matching_params->moving_avg_alpha<<"\t";

	cout<<"blob_distance_measure="<<matching_params->blob_distance_measure<<"\t";
	cout<<"blob_area_measure="<<matching_params->blob_area_measure<<"\n";

	cout<<"\n";
	//_getch();
}

void BlobTracking::printTrackingParameters(){

	cout<<"\n";

	cout<<"Blob tracking parameters updated:\n";	

	cout<<"min_hit_count_for_occ="<<tracking_params->min_hit_count_for_occ<<"\t";
	cout<<"min_hit_count_for_abandoned="<<tracking_params->min_hit_count_for_abandoned<<"\t";
	cout<<"min_hit_count_for_static="<<tracking_params->min_hit_count_for_static<<"\t";
	cout<<"max_miss_count="<<tracking_params->max_miss_count<<"\t";
	cout<<"max_occ_count="<<tracking_params->max_occ_count<<"\t";
	cout<<"max_mean_diff="<<tracking_params->max_mean_diff<<"\t";
	cout<<"static_factor_occ="<<tracking_params->static_factor_occ<<"\t";
	cout<<"static_factor_miss="<<tracking_params->static_factor_miss<<"\n";

	cout<<"\n";
	//_getch();
}

void  BlobTracking::updateParamsMatch(int) {

	matching_params->occ_threshold=(double)(matching_params->occ_threshold_percent)/100.0;
	matching_params->size_threshold=(double)(matching_params->size_threshold_percent)/100.0;	
	matching_params->appearance_threshold=(double)(matching_params->appearance_threshold_percent)/100.0;
	matching_params->moving_avg_alpha=(double)(matching_params->moving_avg_alpha_percent)/100.0;

	matching_params->dist_threshold=(double)(matching_params->dist_threshold_percent)/100.0;
	matching_params->max_blob_dist=max_dist_sqr*matching_params->dist_threshold;

	matching_params->dist_threshold_sqr=matching_params->dist_threshold*matching_params->dist_threshold;

	printMatchingParameters();
}

void BlobTracking::updateAreaMethod(int){
	cout<<"\n";
	if(matching_params->blob_area_measure==BLOB_AREA){
		cout<<"Using blob contour area";		
	}else if(matching_params->blob_area_measure==BOUNDING_BOX_AREA){
		cout<<"Using blob bounding box area";		
	}
	cout<<"\n";
	area_method_updated=true;
}

void BlobTracking::updateDistanceMethod(int){
	cout<<"\n";
	if(matching_params->blob_distance_measure==EUCLIDEAN_DIST){
		cout<<"Using the Euclidean distance between the blob centroids.";
	}else if(matching_params->blob_distance_measure==BOUNDING_BOX_DIST){
		cout<<"Using the minimum distance between the blob bounding boxes and centroids.";	
	}
	cout<<"\n";	
}

void BlobTracking::updateStaticFactor(int){

	if(tracking_params->static_factor_miss<1)
		tracking_params->static_factor_miss=1;
	if(tracking_params->static_factor_occ<1)
		tracking_params->static_factor_occ=1;

}

void BlobTracking::updateParamsTrack(int) {

	tracking_params->max_mean_diff=(double)(tracking_params->max_mean_diff_10)/10.0;


	printTrackingParameters();

}

void BlobTracking::initParamsTrack(track_struct *tracking_params_init) {

	tracking_params->min_hit_count_for_abandoned=tracking_params_init->min_hit_count_for_abandoned;	
	tracking_params->min_hit_count_for_occ=tracking_params_init->min_hit_count_for_occ;
	tracking_params->min_hit_count_for_static=tracking_params_init->min_hit_count_for_static;
	tracking_params->max_miss_count=tracking_params_init->max_miss_count;
	tracking_params->max_occ_count=tracking_params_init->max_occ_count;	
	tracking_params->max_mean_diff_10=tracking_params_init->max_mean_diff_10;

	tracking_params->max_removed_count=tracking_params_init->max_removed_count;
	tracking_params->max_abandoned_count=tracking_params_init->max_abandoned_count;
	tracking_params->static_factor_occ=tracking_params_init->static_factor_occ;
	tracking_params->static_factor_miss=tracking_params_init->static_factor_miss;


}

void BlobTracking::initParamsMatch(match_struct *matching_params_init) {

	matching_params->use_gradient_diff=matching_params_init->use_gradient_diff;

	matching_params->occ_threshold_percent=matching_params_init->occ_threshold_percent;
	matching_params->size_threshold_percent=matching_params_init->size_threshold_percent;
	matching_params->dist_threshold_percent=matching_params_init->dist_threshold_percent;
	matching_params->appearance_threshold_percent=matching_params_init->appearance_threshold_percent;
	matching_params->moving_avg_alpha_percent=matching_params_init->moving_avg_alpha_percent;

	matching_params->blob_distance_measure=matching_params_init->blob_distance_measure;
	matching_params->blob_area_measure=matching_params_init->blob_area_measure;
	matching_params->blob_area_type=matching_params_init->blob_area_type;

}

void BlobTracking::updateDiffMethod(int){
	if(matching_params->use_gradient_diff){
		cout<<"Using gradient images for finding blob difference.";
	}else{
		cout<<"Using actual image for finding blob difference.";

	}
	diff_method_updated=true;
}

void BlobTracking::showTrackHelpWindow(int mouse_event,int x,int y,int flags,void* param){

	if(mouse_event==CV_EVENT_LBUTTONDOWN){
		cvNamedWindow(track_help_window,1);
		cvShowImage(track_help_window,track_help_image);
	}else if(mouse_event==CV_EVENT_RBUTTONDOWN){
		cvDestroyWindow(track_help_window);
	}
}

void BlobTracking::showMatchHelpWindow(int mouse_event,int x,int y,int flags,void* param){

	if(mouse_event==CV_EVENT_LBUTTONDOWN){
		cvNamedWindow(match_help_window,1);
		cvShowImage(match_help_window,match_help_image);
	}else if(mouse_event==CV_EVENT_RBUTTONDOWN){
		cvDestroyWindow(match_help_window);
	}
}

void BlobTracking::initWindowTrack(char *window_name){

	cvNamedWindow(window_name,1);

	cvCreateTrackbar("staticOCC",window_name,&(tracking_params->static_factor_occ),50,updateStaticFactor);
	cvCreateTrackbar("staticMiss",window_name,&(tracking_params->static_factor_miss),50,updateStaticFactor);

	cvCreateTrackbar("hitCount",window_name,&(tracking_params->min_hit_count_for_abandoned),1000,updateParamsTrack);		
	cvCreateTrackbar("missCount",window_name,&(tracking_params->max_miss_count),1000,updateParamsTrack);
	cvCreateTrackbar("occCount",window_name,&(tracking_params->max_occ_count),1000,updateParamsTrack);
	cvCreateTrackbar("minHitOcc",window_name,&(tracking_params->min_hit_count_for_occ),1000,updateParamsTrack);
	cvCreateTrackbar("maxDiff10",window_name,&(tracking_params->max_mean_diff_10),1000,updateParamsTrack);
	cvCreateTrackbar("minHitStatic",window_name,&(tracking_params->min_hit_count_for_static),1000,updateParamsTrack);

	cvCreateTrackbar("maxRem",window_name,&(tracking_params->max_removed_count),1000,updateParamsTrack);
	cvCreateTrackbar("maxAbnd",window_name,&(tracking_params->max_abandoned_count),1000,updateParamsTrack);		

	cvSetMouseCallback(window_name,showTrackHelpWindow,(void*)this);
}

void BlobTracking::initWindowMatch(char *window_name){

	cvNamedWindow( window_name,1);

	cvCreateTrackbar("diffM",window_name,&(matching_params->use_gradient_diff),1,updateDiffMethod);
	cvCreateTrackbar("distM",window_name,&(matching_params->blob_distance_measure),1,updateDistanceMethod);
	cvCreateTrackbar("areaM",window_name,&(matching_params->blob_area_measure),1,updateAreaMethod);	
	cvCreateTrackbar("areaT",window_name,&(matching_params->blob_area_type),2,NULL);

	cvCreateTrackbar("occ%",window_name,&(matching_params->occ_threshold_percent),100,updateParamsMatch);
	cvCreateTrackbar("size%",window_name,&(matching_params->size_threshold_percent),100,updateParamsMatch);
	cvCreateTrackbar("dist%",window_name,&(matching_params->dist_threshold_percent),100,updateParamsMatch);
	cvCreateTrackbar("alpha%",window_name,&(matching_params->moving_avg_alpha_percent),100,updateParamsMatch);

	cvSetMouseCallback(window_name,showMatchHelpWindow,(void*)this);
}