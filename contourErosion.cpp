#include "abandonmentAnalysis.hpp"

//---------------------------------- Get Eroded Contour-------------------------------------------//

void AbandonmentAnalysis::getErodedContour(CvSeq *contour_points){
	if(PRINT_NAME)
		cout<<"start: getErodedContour\n";

	getSortedContourPoints(contour_points);
	getContourExtents();

	if(USE_VERT)
		getVerticalErodedContour();
	if(USE_HORZ){
		getHorizontalErodedContour();
	}
	if(PRINT_NAME)
		cout<<"end:   getErodedContour\n";


}

void AbandonmentAnalysis::getVerticalErodedContour(){

	int no_of_points=contour_horz_extent.size();	
	int new_min_x,new_max_x;

	eroded_contour_horz_extent.clear();
	eroded_contour_horz_extent.resize(no_of_points);

	for(int i=0;i<no_of_points;i++){
		eroded_contour_horz_extent[i]=new horz_extent;

		/*min_x=(double)contour_horz_extent[i]->min_x;
		max_x=(double)contour_horz_extent[i]->max_x;
		new_min_x=((1-alpha)*min_x)+(alpha*max_x);
		new_max_x=((1-alpha)*max_x)+(alpha*min_x);*/

		if(contour_horz_extent[i]->max_x-contour_horz_extent[i]->min_x<=1){
			new_min_x=contour_horz_extent[i]->min_x;
			new_max_x=contour_horz_extent[i]->max_x;
		}else{
			new_min_x=contour_horz_extent[i]->min_x+params->erosion_width;
			new_max_x=contour_horz_extent[i]->max_x-params->erosion_width;
			while(new_min_x>new_max_x){
				new_min_x--;
				new_max_x++;
			}
			if(new_min_x<contour_horz_extent[i]->min_x)
				new_min_x=contour_horz_extent[i]->min_x;

			if(new_max_x>contour_horz_extent[i]->max_x)
				new_max_x=contour_horz_extent[i]->max_x;
		}

		eroded_contour_horz_extent[i]->y=contour_horz_extent[i]->y;
		eroded_contour_horz_extent[i]->min_x=(int)new_min_x;
		eroded_contour_horz_extent[i]->max_x=(int)new_max_x;
	}

}

void AbandonmentAnalysis::getHorizontalErodedContour(){

	int no_of_points=contour_vert_extent.size();	
	int new_min_y,new_max_y;

	eroded_contour_vert_extent.clear();
	eroded_contour_vert_extent.resize(no_of_points);

	for(int i=0;i<no_of_points;i++){
		eroded_contour_vert_extent[i]=new vert_extent;

		/*min_x=(double)contour_horz_extent[i]->min_x;
		max_x=(double)contour_horz_extent[i]->max_x;
		new_min_x=((1-alpha)*min_x)+(alpha*max_x);
		new_max_x=((1-alpha)*max_x)+(alpha*min_x);*/

		if(contour_vert_extent[i]->max_y-contour_vert_extent[i]->min_y<=1){
			new_min_y=contour_vert_extent[i]->min_y;
			new_max_y=contour_vert_extent[i]->max_y;
		}else{
			new_min_y=contour_vert_extent[i]->min_y+params->erosion_width;
			new_max_y=contour_vert_extent[i]->max_y-params->erosion_width;
			while(new_min_y>new_max_y){
				new_min_y--;
				new_max_y++;
			}
			if(new_min_y<contour_vert_extent[i]->min_y)
				new_min_y=contour_vert_extent[i]->min_y;

			if(new_max_y>contour_vert_extent[i]->max_y)
				new_max_y=contour_vert_extent[i]->max_y;
		}

		eroded_contour_vert_extent[i]->x=contour_vert_extent[i]->x;
		eroded_contour_vert_extent[i]->min_y=(int)new_min_y;
		eroded_contour_vert_extent[i]->max_y=(int)new_max_y;
	}

}

//-------------------------------------------------------------------------------------------------------------------------//


//----------------------------------------------- Get Contour Extents----------------------------------------------------//


void AbandonmentAnalysis::getContourExtents(){
	if(PRINT_NAME)
		cout<<"start: getContourExtents\n";
	int no_of_points=sorted_by_x->total;

	if(no_of_points!=sorted_by_y->total){
		std::cout<<"\n-----------Inconsistent x and y sorted contour points-------------\n";
		exit(0);
	}

	if(USE_VERT){
		getVerticalContourExtents(no_of_points);
	}

	if(USE_HORZ){
		getHorizontalContourExtents(no_of_points);
	}
	if(PRINT_NAME)
		cout<<"end:   getContourExtents\n";

}

void AbandonmentAnalysis::getVerticalContourExtents(int no_of_points){

	int count=0;

	CvPoint *min_point,*max_point;
	min_point=(CvPoint*)cvGetSeqElem(sorted_by_y,0);
	max_point=(CvPoint*)cvGetSeqElem(sorted_by_y,no_of_points-1);
	int min_y=min_point->y,max_y=max_point->y;
	bool found_min_x,found_max_x;

	contour_horz_extent.clear();
	contour_horz_extent.resize(max_y-min_y+1);

	for(int y=min_y;y<=max_y;y++){

		contour_horz_extent[count]=new horz_extent;
		contour_horz_extent[count]->y=y;
		found_min_x=found_max_x=false;

		for(int i=0;i<no_of_points;i++){
			if(!found_min_x){
				min_point=(CvPoint*)cvGetSeqElem(sorted_by_x,i);			
				if(min_point->y==y){
					contour_horz_extent[count]->min_x=min_point->x;
					found_min_x=true;
				}
			}
			if(!found_max_x){
				max_point=(CvPoint*)cvGetSeqElem(sorted_by_x,no_of_points-i-1);
				if(max_point->y==y){
					contour_horz_extent[count]->max_x=max_point->x;
					found_max_x=true;
				}
			}			

			if(found_min_x&&found_max_x)
				break;
		}
		if(!found_min_x){
			std::cout<<"\n----------Not found matching min x for y="<<y<<"---------------\n";
			exit(0);
		}
		if(!found_max_x){
			std::cout<<"\n----------Not found matching max x for y="<<y<<"---------------\n";
			exit(0);
		}
		count++;

	}

}

void AbandonmentAnalysis::getHorizontalContourExtents(int no_of_points){	
	int count=0;	

	CvPoint *min_point,*max_point;

	min_point=(CvPoint*)cvGetSeqElem(sorted_by_x,0);
	max_point=(CvPoint*)cvGetSeqElem(sorted_by_x,no_of_points-1);

	int min_x=min_point->x,max_x=max_point->x;
	bool found_min_y,found_max_y;

	contour_vert_extent.clear();
	contour_vert_extent.resize(max_x-min_x+1);

	for(int x=min_x;x<=max_x;x++){

		contour_vert_extent[count]=new vert_extent;
		contour_vert_extent[count]->x=x;

		found_min_y=found_max_y=false;

		for(int i=0;i<no_of_points;i++){
			if(!found_min_y){
				min_point=(CvPoint*)cvGetSeqElem(sorted_by_x,i);			
				if(min_point->x==x){
					contour_vert_extent[count]->min_y=min_point->y;
					found_min_y=true;
				}
			}
			if(!found_max_y){
				max_point=(CvPoint*)cvGetSeqElem(sorted_by_x,no_of_points-i-1);
				if(max_point->x==x){
					contour_vert_extent[count]->max_y=max_point->y;
					found_max_y=true;
				}
			}			

			if(found_min_y&&found_max_y)
				break;
		}
		if(!found_min_y){
			std::cout<<"\n----------Not found matching min y for x="<<x<<"---------------\n";
			exit(0);
		}
		if(!found_max_y){
			std::cout<<"\n----------Not found matching max y for x="<<x<<"---------------\n";
			exit(0);
		}
		count++;

	}

}


//-------------------------------------------------------------------------------------------------------------------------//


//----------------------------------------------- Initialize Seed Points----------------------------------------------------//


void AbandonmentAnalysis::initializeSeedPoints(RgbImage &img){
	if(PRINT_NAME)
		cout<<"start: initializeSeedPoints\n";

	if(img.Ptr()->nChannels!=NCHANNELS){
		cout<<"Invalid image\n";
		exit(0);
	}

	if(USE_VERT){
		initializeVerticalSeedPoints(img);
	}

	if(USE_HORZ){
		initializeHorizontalSeedPoints(img);
	}
	if(PRINT_NAME)
		cout<<"end:   initializeSeedPoints\n";

}

void AbandonmentAnalysis::initializeVerticalSeedPoints(RgbImage &img){

	int no_of_points=eroded_contour_horz_extent.size();	
	Pixel_t current_rgb;
	int r,c;	

	in_obj=new RunningStatVector;
	out_obj=new RunningStatVector;

	inward_growing_points.clear();
	outward_growing_points.clear();
	inward_growing_points.resize(no_of_points);
	outward_growing_points.resize(no_of_points);

	for(int i=0;i<no_of_points;i++){
		inward_growing_points[i]=new CvPoint;
		outward_growing_points[i]=new CvPoint;

		r=inward_growing_points[i]->y=eroded_contour_horz_extent[i]->y;
		c=inward_growing_points[i]->x=eroded_contour_horz_extent[i]->min_x;

		for(int channel=0;channel<NCHANNELS;channel++){
			current_rgb.val[channel]=(double)img(r,c,channel);
		}
		in_obj->addElement(&current_rgb);

		r=outward_growing_points[i]->y=eroded_contour_horz_extent[i]->y;
		c=outward_growing_points[i]->x=eroded_contour_horz_extent[i]->max_x;

		for(int channel=0;channel<NCHANNELS;channel++){
			current_rgb.val[channel]=(double)img(r,c,channel);
		}		
		out_obj->addElement(&current_rgb);

	}

	in_obj->updateState();
	point_set_mean[INWARD_GROWING]=in_obj->final_mean;
	point_set_std[INWARD_GROWING]=in_obj->final_std;

	out_obj->updateState();
	point_set_mean[OUTWARD_GROWING]=out_obj->final_mean;
	point_set_std[OUTWARD_GROWING]=out_obj->final_std;



}

void AbandonmentAnalysis::initializeHorizontalSeedPoints(RgbImage &img){

	int no_of_points=eroded_contour_vert_extent.size();	
	Pixel_t current_rgb;
	int r,c;	

	up_obj=new RunningStatVector;
	down_obj=new RunningStatVector;

	upward_growing_points.clear();
	downward_growing_points.clear();
	upward_growing_points.resize(no_of_points);
	downward_growing_points.resize(no_of_points);

	for(int i=0;i<no_of_points;i++){
		upward_growing_points[i]=new CvPoint;
		downward_growing_points[i]=new CvPoint;

		r=upward_growing_points[i]->y=eroded_contour_vert_extent[i]->min_y;
		c=upward_growing_points[i]->x=eroded_contour_vert_extent[i]->x;

		for(int channel=0;channel<NCHANNELS;channel++){
			current_rgb.val[channel]=(double)img(r,c,channel);
		}
		up_obj->addElement(&current_rgb);

		r=downward_growing_points[i]->y=eroded_contour_vert_extent[i]->max_y;
		c=downward_growing_points[i]->x=eroded_contour_vert_extent[i]->x;

		for(int channel=0;channel<NCHANNELS;channel++){
			current_rgb.val[channel]=(double)img(r,c,channel);
		}
		down_obj->addElement(&current_rgb);

	}
	up_obj->updateState();
	point_set_mean[UPWARD_GROWING]=up_obj->final_mean;
	point_set_std[UPWARD_GROWING]=up_obj->final_std;

	down_obj->updateState();
	point_set_mean[DOWNWARD_GROWING]=down_obj->final_mean;
	point_set_std[DOWNWARD_GROWING]=down_obj->final_std;

}



//-------------------------------------------------------------------------------------------------------------------------//


//----------------------------------------------- Apply Region Growing----------------------------------------------------//



void AbandonmentAnalysis::applyRegionGrowing(RgbImage &img,CvSize image_size){
	if(PRINT_NAME)
		cout<<"start: applyRegionGrowing\n";

	if(img.Ptr()->nChannels!=NCHANNELS){
		cout<<"Invalid image\n";
		exit(0);
	}


	if(USE_VERT){
		applyVerticalRegionGrowing(img,image_size);
	}

	if(USE_HORZ){
		applyHorizontalRegionGrowing(img,image_size);
	}

	if(PRINT_NAME)
		cout<<"end:   applyRegionGrowing\n";


}

void AbandonmentAnalysis::applyVerticalRegionGrowing(RgbImage &img,CvSize image_size){

	int new_x,new_y;
	CvPoint *new_point;
	CvPoint existing_point;
	Pixel_t new_rgb;
	int no_of_points;
	int r,c;

	no_of_points=inward_growing_points.size();
	for(int i=0;i<no_of_points;i++){	

		r=new_y=inward_growing_points[i]->y;	
		c=new_x=inward_growing_points[i]->x-1;			

		if(new_x<0||new_x>=image_size.width)
			continue;
		if(new_y<0||new_y>=image_size.height)
			continue;

		existing_point.x=inward_growing_points[i]->x;
		existing_point.y=inward_growing_points[i]->y;		

		//bool is_similar=evaluatePixellSimilarity(img,image_size,existing_point,INWARD_GROWING);
		bool is_similar=comparePixellSimilarity(img,image_size,existing_point,INWARD_GROWING);
		//bool is_similar=evaluateSimilarity(img,new_x,new_y,INWARD_GROWING);

		if(!is_similar)
			continue;

		for(int channel=0;channel<NCHANNELS;channel++){
			new_rgb.val[channel]=(double)img(r,c,channel);
		}			
		in_obj->addElement(&new_rgb);
		in_obj->updateState();
		point_set_mean[INWARD_GROWING]=in_obj->final_mean;	
		point_set_std[INWARD_GROWING]=in_obj->final_std;

		new_point=new CvPoint;
		new_point->x=new_x;
		new_point->y=new_y;
		inward_growing_points.push_back(new_point);
		no_of_points++;
		//cout<<"no_of_points="<<no_of_points<<"\n";


	}

	//cout<<"Done inward\n";
	no_of_points=outward_growing_points.size();
	for(int i=0;i<no_of_points;i++){

		r=new_y=outward_growing_points[i]->y;
		c=new_x=outward_growing_points[i]->x+1;	

		if(new_x<0||new_x>=image_size.width)
			continue;
		if(new_y<0||new_y>=image_size.height)
			continue;

		existing_point.x=outward_growing_points[i]->x;
		existing_point.y=outward_growing_points[i]->y;

		//bool is_similar=evaluatePixellSimilarity(img,image_size,existing_point,OUTWARD_GROWING);
		bool is_similar=comparePixellSimilarity(img,image_size,existing_point,OUTWARD_GROWING);
		//bool is_similar=evaluateSimilarity(img,new_x,new_y,OUTWARD_GROWING);
		if(!is_similar)
			continue;

		for(int channel=0;channel<NCHANNELS;channel++){
			new_rgb.val[channel]=(double)img(r,c,channel);
		}	
		out_obj->addElement(&new_rgb);
		out_obj->updateState();
		point_set_mean[OUTWARD_GROWING]=out_obj->final_mean;	
		point_set_std[OUTWARD_GROWING]=out_obj->final_std;		

		new_point=new CvPoint;
		new_point->x=new_x;
		new_point->y=new_y;
		outward_growing_points.push_back(new_point);
		no_of_points++;
		//cout<<"no_of_points="<<no_of_points<<"\n";

	}
	//cout<<"Done outward\n";
}

void AbandonmentAnalysis::applyHorizontalRegionGrowing(RgbImage &img,CvSize image_size){

	int new_x,new_y;
	CvPoint *new_point;
	CvPoint existing_point;
	Pixel_t new_rgb;
	int no_of_points;
	int r,c;

	no_of_points=upward_growing_points.size();
	for(int i=0;i<no_of_points;i++){

		r=new_y=upward_growing_points[i]->y-1;
		c=new_x=upward_growing_points[i]->x;		

		if(new_y<0||new_y>=image_size.height)
			continue;		
		if(new_x<0||new_x>=image_size.width)
			continue;

		existing_point.x=upward_growing_points[i]->x;
		existing_point.y=upward_growing_points[i]->y;

		//bool is_similar=evaluatePixellSimilarity(img,image_size,existing_point,UPWARD_GROWING);
		bool is_similar=comparePixellSimilarity(img,image_size,existing_point,UPWARD_GROWING);
		//bool is_similar=evaluateSimilarity(img,new_x,new_y,UPWARD_GROWING);

		if(!is_similar)
			continue;

		for(int channel=0;channel<NCHANNELS;channel++){
			new_rgb.val[channel]=(double)img(r,c,channel);
		}
		up_obj->addElement(&new_rgb);
		up_obj->updateState();
		point_set_mean[UPWARD_GROWING]=up_obj->final_mean;	
		point_set_std[UPWARD_GROWING]=up_obj->final_std;				

		new_point=new CvPoint;
		new_point->x=new_x;
		new_point->y=new_y;
		upward_growing_points.push_back(new_point);
		no_of_points++;
	}

	no_of_points=downward_growing_points.size();
	for(int i=0;i<no_of_points;i++){

		r=new_y=downward_growing_points[i]->y+1;	
		c=new_x=downward_growing_points[i]->x;			

		if(new_y<0||new_y>=image_size.height)
			continue;
		if(new_x<0||new_x>=image_size.width)
			continue;

		existing_point.x=downward_growing_points[i]->x;
		existing_point.y=downward_growing_points[i]->y;		

		//bool is_similar=evaluatePixellSimilarity(img,image_size,existing_point,DOWNWARD_GROWING);
		bool is_similar=comparePixellSimilarity(img,image_size,existing_point,DOWNWARD_GROWING);
		//bool is_similar=evaluateSimilarity(img,new_x,new_y,DOWNWARD_GROWING);

		if(!is_similar)
			continue;

		for(int channel=0;channel<NCHANNELS;channel++){
			new_rgb.val[channel]=(double)img(r,c,channel);
		}
		down_obj->addElement(&new_rgb);
		down_obj->updateState();
		point_set_mean[DOWNWARD_GROWING]=down_obj->final_mean;	
		point_set_std[DOWNWARD_GROWING]=down_obj->final_std;	

		new_point=new CvPoint;
		new_point->x=new_x;
		new_point->y=new_y;
		downward_growing_points.push_back(new_point);
		no_of_points++;

	}
}

//-------------------------------------------------------------------------------------------------------------------------//


//-------------------------------Tests to determine whether or not to add a point to a region----------------------------------------------//

bool AbandonmentAnalysis::evaluateSimilarity(RgbImage &img,int x,int y,int point_set_id){
	double channel_val,channel_mean,channel_std;	
	double dist,max_dist;	

	for(int i=0;i<NCHANNELS;i++){
		channel_val=(double)img(y,x,i);
		channel_mean=point_set_mean[point_set_id]->val[i];
		channel_std=point_set_std[point_set_id]->val[i];

		dist=fabs(channel_mean-channel_val);
		max_dist=params->region_growing_threshold*channel_std;

		/*cout<<"\nchannel:"<<i<<"\tmean="<<channel_mean<<"\tstd="<<channel_std<<"\tval="<<channel_val<<"\n";
		cout<<"dist:"<<dist<<"\tsimilarity_threshold="<<similarity_threshold<<"\tmax_dist="<<max_dist<<"\n";
		cin>>c;*/

		if(dist>max_dist)
			return false;
	}
	return true;

}

bool AbandonmentAnalysis::evaluatePixellSimilarity(RgbImage &img,CvSize image_size,CvPoint pt1,int point_set_id){

	double val1,val2;	
	double squared_diff=0.0;
	CvPoint pt2;

	pt2.x=pt1.x;
	pt2.y=pt1.y;

	if(point_set_id==INWARD_GROWING){
		pt2.x--;
	}else if(point_set_id==OUTWARD_GROWING){
		pt2.x++;
	}else if(point_set_id==UPWARD_GROWING){
		pt2.y--;
	}else if(point_set_id==DOWNWARD_GROWING){
		pt2.y++;
	}

	if(pt2.y<0||pt2.y>=image_size.height)
		return false;
	if(pt2.x<0||pt2.x>=image_size.width)
		return false;;

	for(int i=0;i<NCHANNELS;i++){

		val1=(double)img(pt1.y,pt1.x,i);
		val2=(double)img(pt2.y,pt2.x,i);
		double diff=val1-val2;
		squared_diff+=diff*diff;
	}

	if(squared_diff>params->max_squared_diff)
		return false;

	return true;
}

bool AbandonmentAnalysis::comparePixellSimilarity(RgbImage &img,CvSize image_size,CvPoint pt1,int point_set_id){

	double val1,val2,val3;	
	double squared_diff_21=0.0,squared_diff_23=0.0;
	CvPoint pt2,pt3;

	pt3.x=pt2.x=pt1.x;
	pt3.y=pt2.y=pt1.y;

	if(point_set_id==INWARD_GROWING){
		pt2.x--;
		pt3.x=pt2.x-1;
	}else if(point_set_id==OUTWARD_GROWING){
		pt2.x++;
		pt3.x=pt2.x+1;
	}else if(point_set_id==UPWARD_GROWING){
		pt2.y--;
		pt3.y=pt2.y-1;
	}else if(point_set_id==DOWNWARD_GROWING){
		pt2.y++;
		pt3.y=pt2.y+1;
	}

	if(pt2.y<0||pt2.y>=image_size.height)
		return false;
	if(pt2.x<0||pt2.x>=image_size.width)
		return false;

	if(pt3.y<0||pt3.y>=image_size.height)
		return false;
	if(pt3.x<0||pt3.x>=image_size.width)
		return false;

	for(int i=0;i<NCHANNELS;i++){

		val1=(double)img(pt1.y,pt1.x,i);
		val2=(double)img(pt2.y,pt2.x,i);
		val3=(double)img(pt3.y,pt3.x,i);

		double diff_21=val1-val2;
		double diff_23=val2-val3;

		squared_diff_21+=diff_21*diff_21;
		squared_diff_23+=diff_23*diff_23;

	}

	if(squared_diff_21>squared_diff_23)
		return false;

	return true;
}

//-------------------------------------------------------------------------------------------------------------------------//


/*
void AbandonmentAnalysis::getRegionMeanY(BwImage &frg_gray){
int no_of_points;
CvPoint *current_point;
double region_mean;
double current_gray;

no_of_points=inward_growing_points.size();
region_mean=0.0;
for(int i=0;i<no_of_points;i++){
current_point=inward_growing_points[i];
current_gray=frg_gray(current_point->y,current_point->x);
region_mean+=current_gray/(double)no_of_points;
}
point_set_mean[INWARD_GROWING]=region_mean;

no_of_points=outward_growing_points.size();
region_mean=0.0;
for(int i=0;i<no_of_points;i++){
current_point=outward_growing_points[i];
current_gray=frg_gray(current_point->y,current_point->x);
region_mean+=current_gray/(double)no_of_points;
}
point_set_mean[OUTWARD_GROWING]=region_mean;
}
}*/

