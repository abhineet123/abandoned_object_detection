#include "blobFilter.hpp"

filter_struct* BlobFilter::filter_params;
bool BlobFilter::match_updated;

char* BlobFilter::blob_filtering_help_window;
IplImage* BlobFilter::blob_filtering_help_image;

BlobFilter::BlobFilter(CvSize image_size,filter_struct *filter_params_init){

	blob_filtering_help_window=new char[MAX_NAME];
	sprintf(blob_filtering_help_window,"Blob Detection Help");	

	blob_filtering_help_image=cvLoadImage("help/blob_filtering_help.jpg");

	outline_params=new obj_outline_struct;
	outline_params->line_thickness=1;
	outline_params->line_type=8;
	outline_params->shift=0;
	outline_params->obj_col=CV_RGB(255,0,0);

	test_obj=new obj_struct;
	backup_obj=new obj_struct;
	new_obj=NULL;

	selection_window=new char[MAX_NAME];
	sprintf(selection_window,"Select one or more objects");

	state_variables_initialized=false;
	initStateVariables(image_size);

	filter_params=new filter_struct;
	initParams(filter_params_init);
	updateParams(0);
}

BlobFilter::~BlobFilter() {

	clearStateVariables();

	delete(blob_filtering_help_window);
	delete(selection_window);

	delete(outline_params);
	delete(filter_params);

	delete(test_obj);
	delete(backup_obj);		
}

void BlobFilter::initStateVariables(CvSize image_size){

	clearStateVariables();	

	temp_selection_image=cvCreateImage(image_size,IPL_DEPTH_8U,3);
	hover_image=cvCreateImage(image_size,IPL_DEPTH_8U,3);
	original_size=image_size;

	point_selected=false;
	mouse_hover_event=false;
	left_button_clicked=false;
	right_button_clicked=false;

	state_variables_initialized=true;
	match_updated=false;
}

void BlobFilter::clearStateVariables(){

	if(!state_variables_initialized)
		return;

	freeObjects();
	candidate_removed_objects.clear();

	freeImages();
	candidate_removed_obj_images.clear();	

	cvReleaseImage(&temp_selection_image);

	state_variables_initialized=false;
}

void BlobFilter::freeImages(){

	int img_count=candidate_removed_obj_images.size();

	for(int i=0;i<img_count;i++){
		cvReleaseImage(&candidate_removed_obj_images[i]);
	}
}

void BlobFilter::freeObjects(){

	int obj_count=candidate_removed_objects.size();

	for(int i=0;i<obj_count;i++){
		delete(candidate_removed_objects[i]);
	}
}

void BlobFilter::copyPixel(IplImage *src,IplImage *dst,int r,int c){

	int pixel_pos=r*dst->widthStep+c*dst->nChannels;
	for(int ch=0;ch<dst->nChannels;ch++){
		dst->imageData[pixel_pos+ch]=src->imageData[pixel_pos+ch];			
	}

}

void BlobFilter::clearSelectionImage(IplImage *current_frame,obj_struct *obj){

	if(SHOW_FILTER_STATUS)
		cout<<"Start clearSelectionImage\n";

	int r=obj->min_point.y;
	int c=obj->min_point.x;

	while(r<=obj->max_point.y){
		copyPixel(current_frame,temp_selection_image,r,c);
		r++;
	}
	r--;

	while(c<=obj->max_point.x){
		copyPixel(current_frame,temp_selection_image,r,c);
		c++;
	}
	c--;

	while(r>=obj->min_point.y){
		copyPixel(current_frame,temp_selection_image,r,c);
		r--;
	}
	r++;

	while(c>=obj->min_point.x){
		copyPixel(current_frame,temp_selection_image,r,c);
		c--;
	}


	/*for(int r=obj->min_point.y;r<=obj->max_point.y;r++){
	for(int c=obj->min_point.x;c<=obj->max_point.x;c++){
	int pixel_pos=r*temp_selection_image->widthStep+c*temp_selection_image->nChannels;
	for(int ch=0;ch<temp_selection_image->nChannels;ch++){
	temp_selection_image->imageData[pixel_pos+ch]=current_frame->imageData[pixel_pos+ch];
	//temp_selection_image->imageData[pixel_pos+ch]=0;
	}
	}
	}*/

	if(SHOW_FILTER_STATUS)
		cout<<"Done clearSelectionImage\n";
}

void BlobFilter::removeObjectsFromEnd(IplImage *current_frame,int removed_obj_count){

	if(SHOW_FILTER_STATUS)
		cout<<"Start removeObjectsFromEnd\n";

	int total_obj_count=candidate_removed_objects.size();

	if(removed_obj_count>total_obj_count){
		cout<<"Too many objects specified for removal";
		_getch();
	}

	int removed_obj_id=total_obj_count-1;

	for(int i=0;i<removed_obj_count;i++){
		obj_struct *temp_obj=candidate_removed_objects[removed_obj_id];
		clearSelectionImage(current_frame,temp_obj);
		delete(temp_obj);
		candidate_removed_objects.pop_back();		
		removed_obj_id--;
	}

	if(SHOW_FILTER_STATUS)
		cout<<"Done removeObjectsFromEnd\n";
}

void BlobFilter::addObjectDataFromEnd(int obj_count){

	if(SHOW_FILTER_STATUS)
		cout<<"Start addObjectDataFromEnd\n";

	int total_obj_count=candidate_removed_objects.size();
	int total_img_count=candidate_removed_obj_images.size();

	if(obj_count>total_obj_count){
		cout<<"Too many objects specified for processing";
		_getch();
	}

	for(int i=1;i<=obj_count;i++){
		obj_struct *new_obj=candidate_removed_objects[total_obj_count-i];

		new_obj->centroid.x=(int)((double)(new_obj->min_point.x+new_obj->max_point.x)/2.0);
		new_obj->centroid.y=(int)((double)(new_obj->min_point.y+new_obj->max_point.y)/2.0);

		new_obj->obj_area=(new_obj->max_point.x-new_obj->min_point.x)*(new_obj->max_point.y-new_obj->min_point.y);

		new_obj->obj_img=candidate_removed_obj_images[total_img_count-1];

	}
	if(SHOW_FILTER_STATUS)
		cout<<"Done addObjectDataFromEnd\n";
}

void BlobFilter::showObjectsFromEnd(int obj_count){

	if(SHOW_FILTER_STATUS)
		cout<<"Start showObjectsFromEnd\n";

	int total_obj_count=candidate_removed_objects.size();

	if((obj_count>total_obj_count)||(obj_count<=0)){		
		//cout<<"Displaying all the objects.\n";
		obj_count=total_obj_count;
	}

	for(int i=1;i<=obj_count;i++){
		obj_struct *new_obj=candidate_removed_objects[total_obj_count-i];

		cvRectangle(temp_selection_image, new_obj->min_point, new_obj->max_point,outline_params->obj_col,outline_params->line_thickness,outline_params->line_type, outline_params->shift);
	}

	if(SHOW_FILTER_STATUS)
		cout<<"Done showObjectsFromEnd\n";
}

void BlobFilter::getClickedPoint(int mouse_event,int x,int y,int flags,void* param){

	//cout<<"Start getClickedPoint\n";

	BlobFilter *filter_obj=(BlobFilter*)param;

	if(mouse_event==CV_EVENT_LBUTTONDOWN){

		cout<<"\nReceived a left click.";

		filter_obj->mouse_click_point.x=x;
		filter_obj->mouse_click_point.y=y;
		filter_obj->left_button_clicked=true;
		//filter_obj->clicked_point_count=(filter_obj->clicked_point_count+1)%2;
		filter_obj->point_selected=true;	

	}else if(mouse_event==CV_EVENT_RBUTTONDOWN){

		cout<<"\nReceived a right click.";

		filter_obj->point_selected=true;
		filter_obj->right_button_clicked=true;
	}
	else if(mouse_event==CV_EVENT_MOUSEMOVE){

		filter_obj->mouse_hover_point.x=x;
		filter_obj->mouse_hover_point.y=y;
		filter_obj->mouse_hover_event=true;				
	}

	//cout<<"Done getClickedPoint\n";
}
int BlobFilter::getObject(IplImage *selection_image){

	if(SHOW_FILTER_STATUS)
		cout<<"Start getObject\n";

	new_obj=NULL;
	bool add_object=false;
	int hover_count=0;

	clicked_point_count=0;
	while(clicked_point_count<2){

		point_selected=false;
		while((!point_selected)&&(!mouse_hover_event)){
			//cout<<"Inside the loop\n";
			//cvWaitKey(1);
			int pressed_key=cvWaitKey(1);
			if(pressed_key==27){				
				return CANCEL_OP;
			}	else if	((pressed_key==13)||(pressed_key==10)){
				return OP_COMPLETE;
			}
		}
		if(left_button_clicked){
			left_button_clicked=false;
			if(clicked_point_count==0){	
				new_obj=new obj_struct;
				new_obj->min_point.x=mouse_click_point.x;
				new_obj->min_point.y=mouse_click_point.y;
			}else if(clicked_point_count==1){
				new_obj->max_point.x=mouse_click_point.x;
				new_obj->max_point.y=mouse_click_point.y;

				if(new_obj->min_point.x>new_obj->max_point.x){
					int temp=new_obj->min_point.x;
					new_obj->min_point.x=new_obj->max_point.x;
					new_obj->max_point.x=temp;
				}
				if(new_obj->min_point.y>new_obj->max_point.y){
					int temp=new_obj->min_point.y;
					new_obj->min_point.y=new_obj->max_point.y;
					new_obj->max_point.y=temp;
				}
				add_object=true;
				break;
			}			
		}else if(right_button_clicked){
			right_button_clicked=false;
			if(clicked_point_count>0){
				if(new_obj)
					delete(new_obj);
				return DO_NOTHING;
			}else{
				add_object=false;				
				return REMOVE_OBJ; 
			}

		}else if(mouse_hover_event){
			mouse_hover_event=false;			
			if((new_obj)&&(clicked_point_count==1)){
				cvCopy(selection_image,hover_image);
				cvRectangle(hover_image,new_obj->min_point,mouse_hover_point,outline_params->obj_col,outline_params->line_thickness,outline_params->line_type, outline_params->shift);
				cvShowImage(selection_window,hover_image);				
			}
		}	
		if(point_selected){
			clicked_point_count++;
			//cout<<"\nclicked_point_count="<<clicked_point_count<<"\n";
		}

	}	
	if(SHOW_FILTER_STATUS)
		cout<<"Done getObject\n";

	return ADD_OBJ;
}
void BlobFilter::addCandidateRemovedObjects(IplImage *current_frame){

	int added_objects=0;
	bool received_key_press=false;
	int pressed_key=0;
	bool remove_objects=false;

	if((temp_selection_image->width!=current_frame->width)||(temp_selection_image->height!=current_frame->height)){
		cout<<"The current frame has invalid dimensions.\n";
		cout<<"current_frame: width="<<current_frame->width<<"\t height="<<current_frame->height<<"\n";
		cout<<"temp_selection_image: width="<<temp_selection_image->width<<"\t height="<<temp_selection_image->height<<"\n";
		return;
	}

	refreshSelectionImage(current_frame);

	int total_obj=candidate_removed_objects.size();

	cvNamedWindow(selection_window,1);	
	cvSetMouseCallback(selection_window,getClickedPoint,(void*)this);
	//int pressed_key=cvWaitKey(wait_status);
	/*if(pressed_key!=27)
	use_obj=1;*/
	while(!received_key_press){

		cout<<"\nadded_objects="<<added_objects<<"\n";
		cvShowImage(selection_window,temp_selection_image);
		int obj_add_status=getObject(temp_selection_image);
		if(obj_add_status==ADD_OBJ){
			cout<<"\nAdding an object.\n\n";
			cvRectangle(temp_selection_image, new_obj->min_point, new_obj->max_point,outline_params->obj_col,outline_params->line_thickness,outline_params->line_type, outline_params->shift);			
			candidate_removed_objects.push_back(new_obj);
			added_objects++;
			total_obj++;
		}else if(obj_add_status==REMOVE_OBJ){
			if(total_obj>0){
				removeObjectsFromEnd(current_frame,1);
				added_objects--;
				total_obj--;
				if(total_obj>0)
					showObjectsFromEnd(total_obj);
			}else{
				cout<<"\nNo objects to remove.\n";
			}
		}else if(obj_add_status==CANCEL_OP){
			remove_objects=true;
			break;
		}else if(obj_add_status==OP_COMPLETE){
			remove_objects=false;
			break;
		}
		pressed_key=cvWaitKey(1);
		if(pressed_key>0)
			received_key_press=true;
	}

	if((pressed_key==27)&&(added_objects>0)){
		remove_objects=true;
	}
	if(remove_objects){
		cout<<"\nRemoving all the added objects.\n";
		removeObjectsFromEnd(current_frame,added_objects);
		added_objects=0;
	}

	cout<<"\n";
	if(added_objects==0){
		cout<<"No objects added to the list.\n";
	}else if(added_objects>0){
		cout<<added_objects<<" objects added to the list.\n";
		IplImage *new_img=cvCreateImage(original_size,IPL_DEPTH_8U,3);
		cvCopy(current_frame,new_img);
		candidate_removed_obj_images.push_back(new_img);		
		addObjectDataFromEnd(added_objects);		
	}else{
		cout<<-added_objects<<" objects removed from the list.\n";
	}
	cout<<"Total objects added so far: "<<total_obj<<"\n";
	cout<<"\n";

	cvDestroyWindow(selection_window);
	if(SHOW_FILTER_STATUS)
		cout<<"Done getObject\n";
}

void BlobFilter::initializeTestObject(CBlob *init_blob,IplImage *init_img){

	CvRect bounding_box=init_blob->GetBoundingBox();

	test_obj->min_point.x=bounding_box.x;
	test_obj->min_point.y=bounding_box.y;

	test_obj->max_point.x=bounding_box.x+bounding_box.width;
	test_obj->max_point.y=bounding_box.y+bounding_box.height;

	test_obj->centroid.x=(int)((double)(test_obj->min_point.x+test_obj->max_point.x)/2.0);
	test_obj->centroid.y=(int)((double)(test_obj->min_point.y+test_obj->max_point.y)/2.0);

	test_obj->obj_img=init_img;

	test_obj->obj_area=bounding_box.width*bounding_box.height;	
}

void BlobFilter::resizeCandidateObjectImages(CvSize img_size){

	int obj_count=candidate_removed_objects.size();
	for(int i=0;i<obj_count;i++){
		obj_struct *temp_obj=candidate_removed_objects[i];

		temp_obj->obj_img_resized=cvCreateImage(img_size,IPL_DEPTH_8U,3);
		cvResize(temp_obj->obj_img,temp_obj->obj_img_resized);
	}
}

bool BlobFilter::filterRemovedOjects(CBlobResult *blobs,IplImage *bkg_img,double resize_factor){

	bool not_found_match=false;
	CvSize img_size=cvSize(bkg_img->width,bkg_img->height);
	resizeCandidateObjectImages(img_size);	

	if((img_size.width!=original_size.width/resize_factor)||(img_size.height!=original_size.height/resize_factor)){
		cout<<"Incompatible input image and resize factor specified.\n";
		return true;
	}		

	int blob_count=blobs->GetNumBlobs();
	for(int i=0;i<blob_count;i++){

		CBlob *current_blob=blobs->GetBlob(i);
		if((!current_blob->is_removed)||(current_blob->is_occluded))
			continue;

		//convert object from CBlob to obj_struct format
		initializeTestObject(current_blob,bkg_img);

		if(findMatchingeObject(resize_factor)<0){
			not_found_match=true;
			current_blob->is_removed=false;
			current_blob->is_ack=true;
		}
	}
	return not_found_match;
}

int BlobFilter::getObjectContainingPoint(CvPoint obj_point){

	int obj_count=candidate_removed_objects.size();

	for(int i=0;i<obj_count;i++){

		obj_struct *current_obj=candidate_removed_objects[i];

		if((obj_point.x>=current_obj->min_point.x)&&(obj_point.x<=current_obj->max_point.x)){
			if((obj_point.y>=current_obj->min_point.y)&&(obj_point.y<=current_obj->max_point.y)){
				return i;
			}
		}
	}
	return -1;
}

void BlobFilter::removeCandidateObject(int obj_id){

	vector<obj_struct*>::iterator blob_iterator = candidate_removed_objects.begin();
	blob_iterator+=obj_id;
	candidate_removed_objects.erase(blob_iterator);
}

int BlobFilter::matchObjectLocation(obj_struct *obj){
	double obj_dist_sqr=getSquaredDistance(obj,test_obj);
	if(obj_dist_sqr<filter_params->dist_thr_sqr)
		return 1;
	return 0;

}

int BlobFilter::matchObjectSize(obj_struct *obj){

	int size_diff=abs(obj->obj_area-test_obj->obj_area);
	double size_diff_frac=(double)size_diff/(double)test_obj->obj_area;

	if(size_diff_frac<filter_params->size_thr)
		return 1;

	return 0;
}


double  BlobFilter::getMeanSquaredPixelDifference(IplImage *img1,IplImage *img2,CvPoint min_point,CvPoint max_point,int pixel_count){

	if((img1->width!=img2->width)||(img1->height!=img2->height)){
		cout<<"Incompatible image dimensions for comparison.\n";
		_getch();
		return -1;
	}

	double sqr_pixel_diff_sum=0.0;

	for(int r=min_point.y;r<=max_point.y;r++){
		for(int c=min_point.x;c<=max_point.x;c++){
			int pixel_pos=r*img1->widthStep+c*img1->nChannels;
			double pixel_diff_sqr=0.0;
			for(int ch=0;ch<img1->nChannels;ch++){
				double channel_diff=img1->imageData[pixel_pos+ch]-img2->imageData[pixel_pos+ch];
				pixel_diff_sqr+=channel_diff*channel_diff;
			}
			sqr_pixel_diff_sum+=pixel_diff_sqr;
		}
	}

	double mean_sqr_pixel_diff=sqr_pixel_diff_sum/pixel_count;

	return mean_sqr_pixel_diff;
}

int BlobFilter::isSamePoint(CvPoint pt1,CvPoint pt2){
	if((pt1.x!=pt2.x)||(pt1.y!=pt2.y))
		return 0;
	return 1;
}

int BlobFilter::matchObjectAppearance(obj_struct *obj1,double resize_factor){
	
	double pixel_diff1=getMeanSquaredPixelDifference(obj1->obj_img,test_obj->obj_img,obj1->min_point,obj1->max_point,obj1->obj_area);
	if(pixel_diff1<filter_params->appearance_thr_sqr)
		return 1;

	if((isSamePoint(obj1->min_point,test_obj->min_point))&&(isSamePoint(obj1->max_point,test_obj->max_point)))
		return 0;

	double pixel_diff2=getMeanSquaredPixelDifference(obj1->obj_img,test_obj->obj_img,test_obj->min_point,test_obj->max_point,test_obj->obj_area);
	if(pixel_diff2<filter_params->appearance_thr_sqr)
		return 1;

	return 0;

}
void BlobFilter::backupAndResizeObject(obj_struct *obj,double resize_factor){

	backup_obj->min_point.x=obj->min_point.x;
	backup_obj->min_point.y=obj->min_point.y;
	backup_obj->max_point.x=obj->max_point.x;
	backup_obj->max_point.y=obj->max_point.y;
	backup_obj->centroid.x=obj->centroid.x;
	backup_obj->centroid.y=obj->centroid.y;
	backup_obj->obj_area=obj->obj_area;

	obj->min_point.x=(int)((double)obj->min_point.x/resize_factor);
	obj->min_point.y=(int)((double)obj->min_point.y/resize_factor);
	obj->max_point.x=(int)((double)obj->max_point.x/resize_factor);
	obj->max_point.y=(int)((double)obj->max_point.y/resize_factor);

	obj->centroid.x=(int)((double)(obj->min_point.x+obj->max_point.x)/2.0);
	obj->centroid.y=(int)((double)(obj->min_point.y+obj->max_point.y)/2.0);

	test_obj->obj_area=(int)((double)test_obj->obj_area/(resize_factor*resize_factor));
}

void BlobFilter::restoreBackupObject(obj_struct *obj){

	obj->min_point.x=backup_obj->min_point.x;
	obj->min_point.y=backup_obj->min_point.y;
	obj->max_point.x=backup_obj->max_point.x;
	obj->max_point.y=backup_obj->max_point.y;

	obj->centroid.x=backup_obj->centroid.x;
	obj->centroid.y=backup_obj->centroid.y;

	obj->obj_area=backup_obj->obj_area;
}

int BlobFilter::findMatchingeObject(double resize_factor){

	int total_obj_count=candidate_removed_objects.size();	
	for(int i=0;i<total_obj_count;i++){
		obj_struct *candidate_obj=candidate_removed_objects[i];

		backupAndResizeObject(candidate_obj,resize_factor);

		if(filter_params->match_location){
			if(!matchObjectLocation(candidate_obj))
				continue;
		}

		if(filter_params->match_size){
			if(!matchObjectSize(candidate_obj))
				continue;
		}

		if(filter_params->match_appearance){
			if(!matchObjectAppearance(candidate_obj,resize_factor))
				continue;
		}
		restoreBackupObject(candidate_obj);
		return i;
	}
	return -1;

}

double BlobFilter::getSquaredDistance(obj_struct *obj1,obj_struct *obj2){
	double dist=0.0;	

	if(filter_params->dist_method==DIST_EUCLIDEAN){

		dist=getSquaredEuclideanDistance(obj1->centroid,obj2->centroid);

	}else if(filter_params->dist_method==DIST_BOUNDING_BOX){

		double dist_1=findMinimumBoundingBoxDistance(obj2->centroid,obj1->min_point,obj1->min_point);
		double dist_2=findMinimumBoundingBoxDistance(obj1->centroid,obj2->min_point,obj2->max_point);
		if(dist_1<dist_2)
			dist=dist_1;
		else
			dist=dist_2;		
	}

	return dist;
}

double BlobFilter::findMinimumBoundingBoxDistance(CvPoint centroid,CvPoint min_point,CvPoint max_point){

	double min_dist=0.0;	
	CvPoint nearest_point;

	if(centroid.x<min_point.x){
		nearest_point.x=min_point.x;
	}else if(centroid.x<=max_point.x){
		nearest_point.x=centroid.x;
	}else{
		nearest_point.x=max_point.x;
	}
	if(centroid.y<min_point.y){
		nearest_point.y=min_point.y;
	}else if(centroid.y<=max_point.y){
		nearest_point.y=centroid.y;
	}else{
		nearest_point.y=max_point.y;
	}

	if((nearest_point.x!=centroid.x)||(nearest_point.y!=centroid.y)){
		min_dist=getSquaredEuclideanDistance(centroid,nearest_point);
	}
	return min_dist;
}

void BlobFilter::refreshSelectionImage(IplImage *current_frame){
	cvCopy(current_frame,temp_selection_image);
	if(candidate_removed_objects.size()>0)
		showObjectsFromEnd(0);
}

double BlobFilter::getSquaredEuclideanDistance(CvPoint pt1,CvPoint pt2){
	double dist_x=(pt1.x-pt2.x);
	double dist_y=(pt1.y-pt2.y);
	double dist_sqr=(dist_x*dist_x)+(dist_y*dist_y);
	return dist_sqr;
}

void BlobFilter::printParams(){

	cout<<"\n";
	cout<<"Blob Filtering parameters updated:\n";

	cout<<"enable_filtering="<<filter_params->enable_filtering<<"\t";
	cout<<"dist_thr="<<filter_params->dist_thr<<"\t";
	cout<<"size_thr="<<filter_params->size_thr<<"\t";
	cout<<"appearance_thr="<<filter_params->appearance_thr<<"\t";
	cout<<"dist_method="<<filter_params->dist_method<<"\n";

	cout<<"\n";
	//_getch();

}
void BlobFilter::initParams(filter_struct *filter_params_init) {

	filter_params->enable_filtering=filter_params_init->enable_filtering;

	filter_params->match_location=filter_params_init->match_location;
	filter_params->match_size=filter_params_init->match_size;
	filter_params->match_appearance=filter_params_init->match_appearance;

	filter_params->dist_thr_percent=filter_params_init->dist_thr_percent;	
	filter_params->size_thr_percent=filter_params_init->size_thr_percent;
	filter_params->appearance_thr_percent=filter_params_init->appearance_thr_percent;

	filter_params->dist_method=filter_params_init->dist_method;
}

void BlobFilter::updateParams(int){

	filter_params->dist_thr=(double)filter_params->dist_thr_percent/100.0;
	filter_params->size_thr=(double)filter_params->size_thr_percent/100.0;
	filter_params->appearance_thr=(double)filter_params->appearance_thr_percent/100.0;

	filter_params->dist_thr_sqr=filter_params->dist_thr*filter_params->dist_thr;
	filter_params->appearance_thr_sqr=filter_params->appearance_thr*filter_params->appearance_thr;

	printParams();
}

void BlobFilter::updateFiltering(int){
	cout<<"\n";
	if(filter_params->enable_filtering){
		cout<<"Blob filtering enabled.";
	}else{		
		cout<<"Blob filtering disabled.";
	}
	cout<<"\n";
}

void BlobFilter::updateMatch(int){
	match_updated=true;
}

void BlobFilter::showBlobFilteringHelpWindow(int mouse_event,int x,int y,int flags,void* param){

	if(mouse_event==CV_EVENT_LBUTTONDOWN){
		cvNamedWindow(blob_filtering_help_window,1);
		cvShowImage(blob_filtering_help_window,blob_filtering_help_image);
	}else if(mouse_event==CV_EVENT_RBUTTONDOWN){
		cvDestroyWindow(blob_filtering_help_window);
	}
}

void BlobFilter::initWindow(char *window_name){

	cvNamedWindow( window_name,1);

	cvCreateTrackbar("filter",window_name,&(filter_params->enable_filtering),1,updateFiltering);

	cvCreateTrackbar("location",window_name,&(filter_params->match_location),1,updateMatch);
	cvCreateTrackbar("size",window_name,&(filter_params->match_size),1,updateMatch);
	cvCreateTrackbar("appearance",window_name,&(filter_params->match_appearance),1,updateMatch);
	
	if(filter_params->match_location){
		cvCreateTrackbar("method",window_name,&(filter_params->dist_method),1,updateParams);
		cvCreateTrackbar("dist%",window_name,&(filter_params->dist_thr_percent),100,updateParams);
	}
	if(filter_params->match_size)
		cvCreateTrackbar("size%",window_name,&(filter_params->size_thr_percent),100,updateParams);
	if(filter_params->match_appearance)
		cvCreateTrackbar("app%",window_name,&(filter_params->appearance_thr_percent),100,updateParams);

	cvSetMouseCallback(window_name,showBlobFilteringHelpWindow,(void*)this);

}
