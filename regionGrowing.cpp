#include "abandonmentAnalysis.hpp"

//----------------------------------------------- Get Contour Extents----------------------------------------------------//


void AbandonmentAnalysis::getContourExtents() {
	if(SHOW_STATUS_ABND)
		cout<<"start: getContourExtents\n";
	int no_of_points=sorted_by_x->total;

	if(no_of_points!=sorted_by_y->total) {
		std::cout<<"\n-----------Inconsistent x and y sorted contour points-------------\n";
		exit(0);
	}

	if(USE_VERT) {
		getVerticalContourExtents(no_of_points);
	}

	if(USE_HORZ) {
		getHorizontalContourExtents(no_of_points);
	}
	if(SHOW_STATUS_ABND)
		cout<<"end:   getContourExtents\n";

}

void AbandonmentAnalysis::getVerticalContourExtents(int no_of_points) {

	int count=0;

	CvPoint *min_point,*max_point;
	min_point=(CvPoint*)cvGetSeqElem(sorted_by_y,0);
	max_point=(CvPoint*)cvGetSeqElem(sorted_by_y,no_of_points-1);
	int min_y=min_point->y,max_y=max_point->y;
	bool found_min_x,found_max_x;

	contour_horz_extent.clear();
	contour_horz_extent.resize(max_y-min_y+1);

	for(int y=min_y; y<=max_y; y++) {

		contour_horz_extent[count]=new horz_extent;
		contour_horz_extent[count]->y=y;
		found_min_x=found_max_x=false;

		for(int i=0; i<no_of_points; i++) {
			if(!found_min_x) {
				min_point=(CvPoint*)cvGetSeqElem(sorted_by_x,i);
				if(min_point->y==y) {
					contour_horz_extent[count]->min_x=min_point->x;
					found_min_x=true;
				}
			}
			if(!found_max_x) {
				max_point=(CvPoint*)cvGetSeqElem(sorted_by_x,no_of_points-i-1);
				if(max_point->y==y) {
					contour_horz_extent[count]->max_x=max_point->x;
					found_max_x=true;
				}
			}

			if(found_min_x&&found_max_x)
				break;
		}
		if(!found_min_x) {
			std::cout<<"\n----------Not found matching min x for y="<<y<<"---------------\n";
			exit(0);
		}
		if(!found_max_x) {
			std::cout<<"\n----------Not found matching max x for y="<<y<<"---------------\n";
			exit(0);
		}
		count++;

	}

}

void AbandonmentAnalysis::getHorizontalContourExtents(int no_of_points) {
	int count=0;

	CvPoint *min_point,*max_point;

	min_point=(CvPoint*)cvGetSeqElem(sorted_by_x,0);
	max_point=(CvPoint*)cvGetSeqElem(sorted_by_x,no_of_points-1);

	int min_x=min_point->x,max_x=max_point->x;
	bool found_min_y,found_max_y;

	contour_vert_extent.clear();
	contour_vert_extent.resize(max_x-min_x+1);

	for(int x=min_x; x<=max_x; x++) {

		contour_vert_extent[count]=new vert_extent;
		contour_vert_extent[count]->x=x;

		found_min_y=found_max_y=false;

		for(int i=0; i<no_of_points; i++) {
			if(!found_min_y) {
				min_point=(CvPoint*)cvGetSeqElem(sorted_by_x,i);
				if(min_point->x==x) {
					contour_vert_extent[count]->min_y=min_point->y;
					found_min_y=true;
				}
			}
			if(!found_max_y) {
				max_point=(CvPoint*)cvGetSeqElem(sorted_by_x,no_of_points-i-1);
				if(max_point->x==x) {
					contour_vert_extent[count]->max_y=max_point->y;
					found_max_y=true;
				}
			}

			if(found_min_y&&found_max_y)
				break;
		}
		if(!found_min_y) {
			std::cout<<"\n----------Not found matching min y for x="<<x<<"---------------\n";
			exit(0);
		}
		if(!found_max_y) {
			std::cout<<"\n----------Not found matching max y for x="<<x<<"---------------\n";
			exit(0);
		}
		count++;
	}
}

//-------------------------------------------------------------------------------------------------------------------------//

//---------------------------------- Get Eroded Contour-------------------------------------------//

void AbandonmentAnalysis::getErodedContour() {
    if(SHOW_STATUS_ABND)
        cout<<"start: getErodedContour\n";  

    if(USE_VERT)
        getVerticalErodedContour();
    if(USE_HORZ) {
        getHorizontalErodedContour();
    }
    if(SHOW_STATUS_ABND)
        cout<<"end:   getErodedContour\n";
}

void AbandonmentAnalysis::getVerticalErodedContour() {

    int no_of_points=contour_horz_extent.size();
    int new_min_x,new_max_x;

    eroded_contour_horz_extent.clear();
    eroded_contour_horz_extent.resize(no_of_points);

    for(int i=0; i<no_of_points; i++) {
        eroded_contour_horz_extent[i]=new horz_extent;

        /*min_x=(double)contour_horz_extent[i]->min_x;
        max_x=(double)contour_horz_extent[i]->max_x;
        new_min_x=((1-alpha)*min_x)+(alpha*max_x);
        new_max_x=((1-alpha)*max_x)+(alpha*min_x);*/

        if(contour_horz_extent[i]->max_x-contour_horz_extent[i]->min_x<=1) {
            new_min_x=contour_horz_extent[i]->min_x;
            new_max_x=contour_horz_extent[i]->max_x;
        } else {
            new_min_x=contour_horz_extent[i]->min_x+params->erosion_width;
            new_max_x=contour_horz_extent[i]->max_x-params->erosion_width;
            while(new_min_x>new_max_x) {
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

void AbandonmentAnalysis::getHorizontalErodedContour() {

    int no_of_points=contour_vert_extent.size();
    int new_min_y,new_max_y;

    eroded_contour_vert_extent.clear();
    eroded_contour_vert_extent.resize(no_of_points);

    for(int i=0; i<no_of_points; i++) {
        eroded_contour_vert_extent[i]=new vert_extent;

        /*min_x=(double)contour_horz_extent[i]->min_x;
        max_x=(double)contour_horz_extent[i]->max_x;
        new_min_x=((1-alpha)*min_x)+(alpha*max_x);
        new_max_x=((1-alpha)*max_x)+(alpha*min_x);*/

        if(contour_vert_extent[i]->max_y-contour_vert_extent[i]->min_y<=1) {
            new_min_y=contour_vert_extent[i]->min_y;
            new_max_y=contour_vert_extent[i]->max_y;
        } else {
            new_min_y=contour_vert_extent[i]->min_y+params->erosion_width;
            new_max_y=contour_vert_extent[i]->max_y-params->erosion_width;
            while(new_min_y>new_max_y) {
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

//----------------------------------------------- Initialize Seed Points----------------------------------------------------//


void AbandonmentAnalysis::initializeSeedPoints(RgbImage &img) {
    if(SHOW_STATUS_ABND)
        cout<<"start: initializeSeedPoints\n";

    if(img.Ptr()->nChannels!=NCHANNELS) {
        cout<<"Invalid image\n";
        exit(0);
    }

    if(USE_VERT) {
        initializeVerticalSeedPoints(img);
    }

    if(USE_HORZ) {
        initializeHorizontalSeedPoints(img);
    }
    if(SHOW_STATUS_ABND)
        cout<<"end:   initializeSeedPoints\n";
}

void AbandonmentAnalysis::initializeVerticalSeedPoints(RgbImage &img) {

    int no_of_points=eroded_contour_horz_extent.size();
    Pixel_t current_rgb;
    int r,c;

    in_obj->clearAll();
    out_obj->clearAll();

    inward_growing_points.clear();
    outward_growing_points.clear();
    inward_growing_points.resize(no_of_points);
    outward_growing_points.resize(no_of_points);

    for(int i=0; i<no_of_points; i++) {
        inward_growing_points[i]=new CvPoint;
        outward_growing_points[i]=new CvPoint;

        r=inward_growing_points[i]->y=eroded_contour_horz_extent[i]->y;
        c=inward_growing_points[i]->x=eroded_contour_horz_extent[i]->min_x;

        for(int channel=0; channel<NCHANNELS; channel++) {
            current_rgb.val[channel]=(double)img(r,c,channel);
        }
        in_obj->addElement(&current_rgb);

        r=outward_growing_points[i]->y=eroded_contour_horz_extent[i]->y;
        c=outward_growing_points[i]->x=eroded_contour_horz_extent[i]->max_x;

        for(int channel=0; channel<NCHANNELS; channel++) {
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

    horz_seed_count=no_of_points;

}

void AbandonmentAnalysis::initializeHorizontalSeedPoints(RgbImage &img) {

    int no_of_points=eroded_contour_vert_extent.size();
    Pixel_t current_rgb;
    int r,c;

    up_obj->clearAll();
    down_obj->clearAll();

    upward_growing_points.clear();
    downward_growing_points.clear();
    upward_growing_points.resize(no_of_points);
    downward_growing_points.resize(no_of_points);

    for(int i=0; i<no_of_points; i++) {
        upward_growing_points[i]=new CvPoint;
        downward_growing_points[i]=new CvPoint;

        r=upward_growing_points[i]->y=eroded_contour_vert_extent[i]->min_y;
        c=upward_growing_points[i]->x=eroded_contour_vert_extent[i]->x;

        for(int channel=0; channel<NCHANNELS; channel++) {
            current_rgb.val[channel]=(double)img(r,c,channel);
        }
        up_obj->addElement(&current_rgb);

        r=downward_growing_points[i]->y=eroded_contour_vert_extent[i]->max_y;
        c=downward_growing_points[i]->x=eroded_contour_vert_extent[i]->x;

        for(int channel=0; channel<NCHANNELS; channel++) {
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

    vert_seed_count=no_of_points;
}



//-------------------------------------------------------------------------------------------------------------------------//


//----------------------------------------------- Apply Region Growing----------------------------------------------------//



void AbandonmentAnalysis::applyRegionGrowing(RgbImage &img,CvSize image_size) {
    if(SHOW_STATUS_ABND)
        cout<<"start: applyRegionGrowing\n";

    if(img.Ptr()->nChannels!=NCHANNELS) {
        cout<<"Invalid image\n";
        exit(0);
    }

    if(params->similarity_method==REGION_SIMILARITY) {
        evaluateSimilarity=&AbandonmentAnalysis::evaluateRegionSimilarity;
    } else if(params->similarity_method==PIXEL_SIMILARITY) {
        evaluateSimilarity=&AbandonmentAnalysis::evaluatePixellSimilarity;
    } else if(params->similarity_method!=PIXEL_COMPARISON) {
        evaluateSimilarity=&AbandonmentAnalysis::comparePixellSimilarity;
        return;
    }

    if(USE_VERT) {
        applyVerticalRegionGrowing(img,image_size);
    }
    if(USE_HORZ) {
        applyHorizontalRegionGrowing(img,image_size);
    }
    if(SHOW_STATUS_ABND)
        cout<<"end: applyRegionGrowing\n";
}

void AbandonmentAnalysis::applyVerticalRegionGrowing(RgbImage &img,CvSize image_size) {

    CvPoint existing_point;
    Pixel_t new_rgb;
    int no_of_points;

    no_of_points=inward_growing_points.size();
    for(int i=0; i<no_of_points; i++) {

        existing_point.x=inward_growing_points[i]->x;
        existing_point.y=inward_growing_points[i]->y;

        //bool is_similar=evaluatePixellSimilarity(img,existing_point,INWARD_GROWING);
        bool is_similar=(this->*evaluateSimilarity)(img,existing_point,INWARD_GROWING);
        //bool is_similar=evaluateSimilarity(img,existing_point,INWARD_GROWING);

        if(!is_similar)
            continue;

        CvPoint *new_point=new CvPoint;
        getAdjacentPoint(existing_point,new_point,INWARD_GROWING);
        inward_growing_points.push_back(new_point);
        no_of_points++;

        for(int channel=0; channel<NCHANNELS; channel++) {
            new_rgb.val[channel]=(double)img(new_point->y,new_point->x,channel);
        }

        in_obj->addElement(&new_rgb);
        in_obj->updateState();
        point_set_mean[INWARD_GROWING]=in_obj->final_mean;
        point_set_std[INWARD_GROWING]=in_obj->final_std;

        //cout<<"no_of_points="<<no_of_points<<"\n";
    }

    //cout<<"Done inward\n";
    no_of_points=outward_growing_points.size();
    for(int i=0; i<no_of_points; i++) {

        existing_point.x=outward_growing_points[i]->x;
        existing_point.y=outward_growing_points[i]->y;

        //bool is_similar=evaluatePixellSimilarity(img,existing_point,OUTWARD_GROWING);
        bool is_similar=(this->*evaluateSimilarity)(img,existing_point,OUTWARD_GROWING);
        //bool is_similar=evaluateRegionSimilarity(img,existing_point,OUTWARD_GROWING);
        if(!is_similar)
            continue;

        CvPoint *new_point=new CvPoint;
        getAdjacentPoint(existing_point,new_point,OUTWARD_GROWING);
        outward_growing_points.push_back(new_point);
        no_of_points++;

        for(int channel=0; channel<NCHANNELS; channel++) {
            new_rgb.val[channel]=(double)img(new_point->y,new_point->x,channel);
        }

        out_obj->addElement(&new_rgb);
        out_obj->updateState();
        point_set_mean[OUTWARD_GROWING]=out_obj->final_mean;
        point_set_std[OUTWARD_GROWING]=out_obj->final_std;

        //cout<<"no_of_points="<<no_of_points<<"\n";
    }
    //cout<<"Done outward\n";
}

void AbandonmentAnalysis::applyHorizontalRegionGrowing(RgbImage &img,CvSize image_size) {

    CvPoint existing_point;
    Pixel_t new_rgb;
    int no_of_points;

    no_of_points=upward_growing_points.size();
    for(int i=0; i<no_of_points; i++) {

        existing_point.x=upward_growing_points[i]->x;
        existing_point.y=upward_growing_points[i]->y;

        //bool is_similar=evaluatePixellSimilarity(img,existing_point,UPWARD_GROWING);
        bool is_similar=(this->*evaluateSimilarity)(img,existing_point,UPWARD_GROWING);
        //bool is_similar=evaluateSimilarity(img,existing_point,UPWARD_GROWING);

        if(!is_similar)
            continue;

        CvPoint *new_point=new CvPoint;
        getAdjacentPoint(existing_point,new_point,UPWARD_GROWING);
        upward_growing_points.push_back(new_point);
        no_of_points++;

        for(int channel=0; channel<NCHANNELS; channel++) {
            new_rgb.val[channel]=(double)img(new_point->y,new_point->x,channel);
        }

        up_obj->addElement(&new_rgb);
        up_obj->updateState();
        point_set_mean[UPWARD_GROWING]=up_obj->final_mean;
        point_set_std[UPWARD_GROWING]=up_obj->final_std;
    }

    no_of_points=downward_growing_points.size();
    for(int i=0; i<no_of_points; i++) {

        existing_point.x=downward_growing_points[i]->x;
        existing_point.y=downward_growing_points[i]->y;

        //bool is_similar=evaluatePixellSimilarity(img,existing_point,DOWNWARD_GROWING);
        bool is_similar=(this->*evaluateSimilarity)(img,existing_point,DOWNWARD_GROWING);
        //bool is_similar=evaluateSimilarity(img,existing_point,DOWNWARD_GROWING);

        if(!is_similar)
            continue;

        CvPoint *new_point=new CvPoint;
        getAdjacentPoint(existing_point,new_point,DOWNWARD_GROWING);
        downward_growing_points.push_back(new_point);
        no_of_points++;

        for(int channel=0; channel<NCHANNELS; channel++) {
            new_rgb.val[channel]=(double)img(new_point->y,new_point->x,channel);
        }

        down_obj->addElement(&new_rgb);
        down_obj->updateState();
        point_set_mean[DOWNWARD_GROWING]=down_obj->final_mean;
        point_set_std[DOWNWARD_GROWING]=down_obj->final_std;
    }
}

//-------------------------------------------------------------------------------------------------------------------------//


//-------------------------------Tests to determine whether or not to add a point to a region----------------------------------------------//

bool AbandonmentAnalysis::getAdjacentPoint(CvPoint pt1,CvPoint *pt2,int point_set_id) {

    pt2->x=pt1.x;
    pt2->y=pt1.y;

    if(point_set_id==INWARD_GROWING) {
        pt2->x--;
    } else if(point_set_id==OUTWARD_GROWING) {
        pt2->x++;
    } else if(point_set_id==UPWARD_GROWING) {
        pt2->y--;
    } else if(point_set_id==DOWNWARD_GROWING) {
        pt2->y++;
    }

    if(pt2->y<0||pt2->y>=proc_size.height)
        return false;
    if(pt2->x<0||pt2->x>=proc_size.width)
        return false;

    return true;
}

bool AbandonmentAnalysis::evaluateRegionSimilarity(RgbImage &img,CvPoint pt1,int point_set_id) {

    if(SHOW_STATUS_ABND)
        cout<<"start: evaluateRegionSimilarity\n";

    CvPoint pt2;
    if(!getAdjacentPoint(pt1,&pt2,point_set_id))
        return false;

    for(int i=0; i<NCHANNELS; i++) {
        double channel_val=(double)img(pt2.y,pt2.x,i);
        double channel_mean=point_set_mean[point_set_id]->val[i];
        double channel_std=point_set_std[point_set_id]->val[i];

        double dist=fabs(channel_mean-channel_val);
        double max_dist=params->region_growing_threshold*channel_std;

        /*cout<<"\nchannel:"<<i<<"\tmean="<<channel_mean<<"\tstd="<<channel_std<<"\tval="<<channel_val<<"\n";
        cout<<"dist:"<<dist<<"\tsimilarity_threshold="<<similarity_threshold<<"\tmax_dist="<<max_dist<<"\n";
        cin>>c;*/

        if(dist>max_dist)
            return false;

        if(SHOW_STATUS_ABND)
            cout<<"end: evaluateRegionSimilarity\n";
    }
    return true;
}

bool AbandonmentAnalysis::evaluatePixellSimilarity(RgbImage &img,CvPoint pt1,int point_set_id) {

    if(SHOW_STATUS_ABND)
        cout<<"start: evaluatePixellSimilarity\n";

    CvPoint pt2;
    if(!getAdjacentPoint(pt1,&pt2,point_set_id))
        return false;

    double squared_diff=0.0;
    for(int i=0; i<img.nChannels(); i++) {

        /*cout<<"old point: x="<<pt1.x<<" y="<<pt1.y<<"\n";
        cout<<"new point: x="<<pt2.x<<" y="<<pt2.y<<"\n";*/

        double val1=(double)img(pt1.y,pt1.x,i);
        double val2=(double)img(pt2.y,pt2.x,i);
        double diff=val1-val2;
        squared_diff+=diff*diff;
    }

    if(squared_diff>params->max_squared_diff)
        return false;

    if(SHOW_STATUS_ABND)
        cout<<"end: evaluatePixellSimilarity\n";

    return true;
}

bool AbandonmentAnalysis::comparePixellSimilarity(RgbImage &img,CvPoint pt1,int point_set_id) {

    if(SHOW_STATUS_ABND)
        cout<<"start: comparePixellSimilarity\n";

    CvPoint pt2;
    if(!getAdjacentPoint(pt1,&pt2,point_set_id))
        return false;

    CvPoint pt3;
    if(!getAdjacentPoint(pt2,&pt3,point_set_id))
        return false;

    double squared_diff_21=0.0,squared_diff_23=0.0;
    for(int i=0; i<NCHANNELS; i++) {

        double val1=(double)img(pt1.y,pt1.x,i);
        double val2=(double)img(pt2.y,pt2.x,i);
        double val3=(double)img(pt3.y,pt3.x,i);

        double diff_21=val1-val2;
        double diff_23=val2-val3;

        squared_diff_21+=diff_21*diff_21;
        squared_diff_23+=diff_23*diff_23;
    }

    if(squared_diff_21>squared_diff_23)
        return false;

    if(SHOW_STATUS_ABND)
        cout<<"end: comparePixellSimilarity\n";

    return true;
}

//-------------------------------------------------------------------------------------------------------------------------//

/*--------------------------------------- Testing and debugging functions---------------------------------------------------*/

void AbandonmentAnalysis::testRegionGrowing(RgbImage &img,CvSize image_size,CBlobResult *blobs) {
    CvSeq *point_set;
    CBlobContour *blob_contour;
    CBlob *current_blob;
    int no_of_blobs=blobs->GetNumBlobs();
    /*BwImage img_gray= cvCreateImage(image_size, IPL_DEPTH_8U,1);
    cvCvtColor(img.Ptr(),img_gray.Ptr(),CV_BGR2GRAY);*/
    //clearContourImage();
    cout<<"\n";
    for(int i=0; i<no_of_blobs; i++) {
        current_blob=blobs->GetBlob(i);
        blob_contour=current_blob->GetExternalContour();
        point_set=blob_contour->GetContourPoints();

		getSortedContourPoints(point_set);
		//cout<<"calling getContourExtents\n";
		getContourExtents();
		getErodedContour();

        initializeSeedPoints(img);
        cout<<"initial points for blob "<<i<<" = "<<getTotalPointCount()<<"\t";
        //drawContour(CV_RGB(255,0,0));
        //cvShowImage( "Before Region Growing", contour_image.Ptr());
        //cvWaitKey(0);

        applyRegionGrowing(img,image_size);
        cout<<"total points for blob "<<i<<" = "<<getTotalPointCount()<<"\n";
        drawContour(0,CV_RGB(0,255,0));
    }
    cout<<"\n";
}

void AbandonmentAnalysis::drawOriginalContour(CBlobResult *blobs,CvScalar color) {
    int no_of_blobs=blobs->GetNumBlobs();
    CvSeq *point_set;
    CBlob *current_blob;
    CBlobContour *blob_contour;
    int contour_length;
    CvPoint *contour_point;
    int r,c;
    int no_of_channels=contour_image.Ptr()->nChannels;

    for(int i=0; i<no_of_blobs; i++) {
        current_blob=blobs->GetBlob(i);
        blob_contour=current_blob->GetExternalContour();
        point_set=blob_contour->GetContourPoints();

        contour_length=point_set->total;
        for(int j=0; j<contour_length; j++) {
            contour_point=(CvPoint*)cvGetSeqElem(point_set,j);
            r=contour_point->y;
            c=contour_point->x;
            for(int k=0; k<no_of_channels; k++) {
                contour_image(r,c,k)=(unsigned char)color.val[k];
            }
        }
    }
}

void AbandonmentAnalysis::drawErodedContour(CBlobResult *blobs,CvScalar color) {

    int no_of_blobs=blobs->GetNumBlobs();   
    int no_of_channels=contour_image.Ptr()->nChannels;    
    //contour_image.Clear();
    for(int i=0; i<no_of_blobs; i++) {

        CBlob *current_blob=blobs->GetBlob(i);
        CBlobContour *blob_contour=current_blob->GetExternalContour();
        CvSeq *point_set=blob_contour->GetContourPoints();

		getSortedContourPoints(point_set);
		//cout<<"calling getContourExtents\n";
		getContourExtents();
		getErodedContour();

        int no_of_points=eroded_contour_horz_extent.size();

        for(int j=0; j<no_of_points; j++) {

            int r=eroded_contour_horz_extent[j]->y;
            int c_min=eroded_contour_horz_extent[j]->min_x;
            int c_max=eroded_contour_horz_extent[j]->max_x;

            for(int k=0; k<no_of_channels; k++) {

                contour_image(r,c_min,k)=(unsigned char)color.val[k];

                if(c_max!=c_min) {
                    contour_image(r,c_max,k)=(unsigned char)color.val[k];
                }
            }
        }
    }
}

void AbandonmentAnalysis::clearSeedPoints(){

	if(SHOW_STATUS_ABND)
		cout<<"start: clearSeedPoints\n";

	int seed_point_count=blob_seed_points.size();
	for(int i=0;i<seed_point_count;i++){
		delete(blob_seed_points[i]);
	}
	blob_seed_points.clear();

	if(SHOW_STATUS_ABND)
		cout<<"end: clearSeedPoints\n";
}

void AbandonmentAnalysis::getBlobSeedPoints(CBlob *blob){

	if(SHOW_STATUS_ABND)
		cout<<"start: getBlobSeedPoints\n";
	
	clearSeedPoints();	

	cvSetZero(blob_image);
	blob->FillBlob(blob_image,CV_RGB(255,255,255));
	//cvShowImage("blob_image original",blob_image);
	cvErode(blob_image,blob_image,NULL,params->erosion_width);
	//cvShowImage("blob_image eroded",blob_image);
	//cvWaitKey(0);
	eroded_blobs=new CBlobResult(blob_image,NULL,BACKGROUND_COLOR);
	int eroded_blob_count=eroded_blobs->GetNumBlobs();
	//cout<<"\n eroded_blob_count="<<eroded_blob_count<<"\n";
	for(int j=0;j<eroded_blob_count;j++) {

		CBlob *eroded_blob=eroded_blobs->GetBlob(j);
		CBlobContour *blob_contour=eroded_blob->GetExternalContour();
		//cout<<"Done GetExternalContour\n";
		CvSeq *point_set=blob_contour->GetContourPoints();
		//cout<<"Done GetContourPoints\n";
		if(!point_set)
			return;

		int contour_length=point_set->total;		
		for(int j=0; j<contour_length; j++) {

			CvPoint *contour_point=(CvPoint*)cvGetSeqElem(point_set,j);
			seed_point *new_point=new seed_point(contour_point->x,contour_point->y);
			//cvSetZero(blob_image);
			//blob->FillBlob(blob_image,CV_RGB(255,255,255));
			getGrowingDirection(new_point);
			blob_seed_points.push_back(new_point);
		}
	}

	if(SHOW_STATUS_ABND)
		cout<<"end: getBlobSeedPoints\n";

}

bool AbandonmentAnalysis::evaluatePointSimilarity(IplImage *img,CvPoint *pt1,CvPoint *pt2){

	if(SHOW_STATUS_ABND)
		cout<<"start: evaluatePointSimilarity\n";

	if((pt1->x<0)||(pt1->x>=img->width))
		return false;
	if((pt1->y<0)||(pt1->y>=img->height))
		return false;
	if((pt2->x<0)||(pt2->x>=img->width))
		return false;
	if((pt2->y<0)||(pt2->y>=img->height))
		return false;

	double squared_diff=0.0;

	int pos1=pt1->y*img->widthStep+pt1->x*img->nChannels;
	int pos2=pt2->y*img->widthStep+pt2->x*img->nChannels;

	for(int ch=0; ch<img->nChannels; ch++) {
		double val1=(double)img->imageData[pos1+ch];
		double val2=(double)img->imageData[pos2+ch];
		double diff=val1-val2;
		squared_diff+=(diff*diff);
	}

	if(squared_diff>params->max_squared_diff)
		return false;

	if(SHOW_STATUS_ABND)
		cout<<"end: evaluatePointSimilarity\n";

	return true;
}

void AbandonmentAnalysis::performRegionGrowing(IplImage *img){

	if(SHOW_STATUS_ABND)
		cout<<"start: performRegionGrowing\n";

	int seed_point_count=blob_seed_points.size();
	for(int i=0;i<seed_point_count;i++){
		seed_point *current_seed_point=blob_seed_points[i];
		
		CvPoint current_point;
		current_point.x=current_seed_point->x;
		current_point.y=current_seed_point->y;

		CvPoint test_point;
		if(current_seed_point->vert_growing<0)
			test_point.y=current_seed_point->y+1;
		else if(current_seed_point->vert_growing>0)
			test_point.y=current_seed_point->y-1;
		else 
			test_point.y=current_seed_point->y;

		if(current_seed_point->horz_growing<0)
			test_point.x=current_seed_point->x+1;
		else if(current_seed_point->horz_growing>0)
			test_point.x=current_seed_point->x-1;
		else 
			test_point.x=current_seed_point->x;

		if((test_point.x==current_point.x)&&(test_point.y==current_point.y))
			continue;		

		if(evaluatePointSimilarity(img,&test_point,&current_point)){
			seed_point *new_point=new seed_point(test_point.x,test_point.y);
			new_point->horz_growing=current_seed_point->horz_growing;
			new_point->vert_growing=current_seed_point->vert_growing;
			blob_seed_points.push_back(new_point);
			seed_point_count++;
		}
	}

	if(SHOW_STATUS_ABND)
		cout<<"end: performRegionGrowing\n";

}

void AbandonmentAnalysis::getGrowingDirection(seed_point *root_point){

	if(SHOW_STATUS_ABND)
		cout<<"start: getGrowingDirection\n";

	int neighbour_x=root_point->x;
	int neighbour_y=root_point->y;

	if(root_point->y+1<blob_image->height){
		int pixel_pos=(root_point->y+1)*blob_image->widthStep+root_point->x;
		unsigned char pixel_val=(unsigned char) blob_image->imageData[pixel_pos];
		if(pixel_val==FOREGROUND_COLOR)
			root_point->vert_growing++;
	}

	if(root_point->y-1>=0){
		int pixel_pos=(root_point->y-1)*blob_image->widthStep+root_point->x;
		unsigned char pixel_val=(unsigned char) blob_image->imageData[pixel_pos];
		if(pixel_val==FOREGROUND_COLOR)
			root_point->vert_growing--;
	}

	if(root_point->x+1<blob_image->width){
		int pixel_pos=root_point->y*blob_image->widthStep+(root_point->x+1);
		unsigned char pixel_val=(unsigned char) blob_image->imageData[pixel_pos];
		if(pixel_val==FOREGROUND_COLOR)
			root_point->horz_growing++;
	}

	if(root_point->x-1>=0){
		int pixel_pos=root_point->y*blob_image->widthStep+(root_point->x-1);
		unsigned char pixel_val=(unsigned char) blob_image->imageData[pixel_pos];
		if(pixel_val==FOREGROUND_COLOR)
			root_point->horz_growing--;
	}

	if(SHOW_STATUS_ABND)
		cout<<"end: getGrowingDirection\n";

}


void AbandonmentAnalysis::drawBlobBoundary(CBlob *current_blob,CvScalar color){

	if(!current_blob)
		return;

	if(SHOW_STATUS_ABND)
		cout<<"starting drawBlobBoundary\n";

	CBlobContour *blob_contour=current_blob->GetExternalContour();
	//cout<<"Done GetExternalContour\n";
	CvSeq *point_set=blob_contour->GetContourPoints();
	//cout<<"Done GetContourPoints\n";
	if(!point_set)
		return;

	int contour_length=point_set->total;
	//cout<<"\n contour_length="<<contour_length<<"\n";
	for(int j=0; j<contour_length; j++) {

		CvPoint *contour_point=(CvPoint*)cvGetSeqElem(point_set,j);
		//cout<<"row="<<contour_point->y<<"\t"<<"col="<<contour_point->x<<"\n";

		if((contour_point->x>=boundary_image->width)||(contour_point->y>=boundary_image->height)){
			cout<<"Invalid boundary point.\n";
			cout<<"row="<<contour_point->y<<"\t"<<"col="<<contour_point->x<<"\n";
			return;
		}
		
		int pixel_pos=contour_point->y*boundary_image->widthStep+contour_point->x*boundary_image->nChannels;
		for(int ch=0; ch<boundary_image->nChannels; ch++) {			
			boundary_image->imageData[pixel_pos+ch]=(unsigned char)color.val[ch];
		}
	}

	if(SHOW_STATUS_ABND)
		cout<<"done drawBlobBoundary\n";
}

void AbandonmentAnalysis::drawErodedContour2(CBlobResult *blobs) {

	if(SHOW_STATUS_ABND)
		cout<<"starting drawErodedContour2\n";

	int no_of_blobs=blobs->GetNumBlobs();	
	int no_of_channels=contour_image.Ptr()->nChannels;
	cvSetZero(boundary_image);
		
	//contour_image.Clear();
	//cout<<"\n no_of_blobs="<<no_of_blobs<<"\n";
	for(int i=0; i<no_of_blobs; i++) {
		CBlob *current_blob=blobs->GetBlob(i);
		
		drawBlobBoundary(current_blob,CV_RGB(0,255,0));
		CBlobContour *blob_contour=current_blob->GetExternalContour();
		CvSeq *point_set=blob_contour->GetContourPoints();

		cvSetZero(blob_image);
		current_blob->FillBlob(blob_image,CV_RGB(255,255,255));
		cvShowImage("blob_image original",blob_image);
		cvErode(blob_image,blob_image,NULL,params->erosion_width);
		cvShowImage("blob_image eroded",blob_image);
		//cvWaitKey(0);
		eroded_blobs=new CBlobResult(blob_image,NULL,BACKGROUND_COLOR);
		int eroded_blob_count=eroded_blobs->GetNumBlobs();
		//cout<<"\n eroded_blob_count="<<eroded_blob_count<<"\n";
		for(int j=0;j<eroded_blob_count;j++) {

			CBlob *eroded_blob=eroded_blobs->GetBlob(j);
			cvSetZero(blob_image);
			eroded_blob->FillBlob(blob_image,CV_RGB(255,255,255));
			//cvShowImage("blob_image eroded 2",blob_image);
			//cvWaitKey(0);
			drawBlobBoundary(eroded_blob,CV_RGB(255,0,0));
		}
		delete(eroded_blobs);
	}
	cvShowImage("Boundary Image",boundary_image);
	//cvWaitKey(0);

	if(SHOW_STATUS_ABND)
		cout<<"done drawErodedContour2\n";
}

void AbandonmentAnalysis::drawContour(int show_all,CvScalar color) {

    if(USE_VERT) {
        drawPoints(show_all,color,INWARD_GROWING);
        drawPoints(show_all,color,OUTWARD_GROWING);
    }

    if(USE_HORZ) {
        drawPoints(show_all,color,UPWARD_GROWING);
        drawPoints(show_all,color,DOWNWARD_GROWING);
    }
}

void AbandonmentAnalysis::drawPoints(int show_all,CvScalar color,int point_set_id) {
    vector<CvPoint*> &point_list=point_set_id==INWARD_GROWING?inward_growing_points:
                                 point_set_id==OUTWARD_GROWING?outward_growing_points:
                                 point_set_id==UPWARD_GROWING?upward_growing_points:downward_growing_points;

    /*if(point_set_id==INWARD_GROWING)
    point_list=inward_growing_points;
    if(point_set_id==OUTWARD_GROWING)
    point_list=outward_growing_points;
    if(point_set_id==UPWARD_GROWING)
    point_list=upward_growing_points;
    if(point_set_id==DOWNWARD_GROWING)
    point_list=downward_growing_points;*/
    int start_index=0;
    if(!show_all) {
        if((point_set_id==INWARD_GROWING)||(point_set_id==OUTWARD_GROWING))
            start_index=horz_seed_count;

        if((point_set_id==UPWARD_GROWING)||(point_set_id==DOWNWARD_GROWING))
            start_index=vert_seed_count;
    }

    int no_of_points=point_list.size();
    int no_of_channels=contour_image.nChannels();

    for(int j=start_index; j<no_of_points; j++) {

        for(int k=0; k<no_of_channels; k++) {
            contour_image(point_list[j]->y,point_list[j]->x,k)=(unsigned char)color.val[k];
        }
    }
}

void AbandonmentAnalysis::testStateVariables(CvSeq *contour_points) {
    int no_of_points=sorted_by_x->total;
    CvPoint *current_point;

    if(no_of_points!=sorted_by_y->total) {
        std::cout<<"\n-----------Inconsistent x and y sorted contour points-------------\n";
        exit(0);
    }

    std::cout<<"\nOriginal Contour Points:\n";
    for(int i=0; i<no_of_points; i++) {
        current_point=(CvPoint*)cvGetSeqElem(contour_points,i);
        std::cout<<"Point "<<i<<": x="<<current_point->x<<" y="<<current_point->y<<"\n";
    }

    std::cout<<"\nPoints sorted by x coordinates:\n";
    for(int i=0; i<no_of_points; i++) {
        current_point=(CvPoint*)cvGetSeqElem(sorted_by_x,i);
        std::cout<<"Point "<<i<<": x="<<current_point->x<<" y="<<current_point->y<<"\n";
    }

    std::cout<<"\nPoints sorted by y coordinates:\n";
    for(int i=0; i<no_of_points; i++) {
        current_point=(CvPoint*)cvGetSeqElem(sorted_by_y,i);
        std::cout<<"Point "<<i<<": x="<<current_point->x<<" y="<<current_point->y<<"\n";
    }

    std::cout<<"\nHorizontal extents of the contour:\n";
    no_of_points=contour_horz_extent.size();
    for(int i=0; i<no_of_points; i++) {
        current_point=(CvPoint*)cvGetSeqElem(sorted_by_y,i);
        std::cout<<"Point "<<i<<": y="<<contour_horz_extent[i]->y<<" max_x="<<contour_horz_extent[i]->max_x<<" min_x="<<contour_horz_extent[i]->min_x<<"\n";
    }

    std::cout<<"Press any key to continue.\n";
    _getch();
}

void AbandonmentAnalysis::clearContourImage() {
    cvSet(contour_image.Ptr(),CV_RGB(255,255,255));
}


//------------------------------------------------------------------------------------------------------------//

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

