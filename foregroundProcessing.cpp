#include "foregroundProcessing.hpp"


frg_struct* ForegroundProc::params;
morph_struct* ForegroundProc::morph_params;
bool ForegroundProc::frg_updated;
bool ForegroundProc::shadow_method_updated;

char* ForegroundProc::frg_proc_help_window;
char* ForegroundProc::frg_toggle_help_window;

IplImage* ForegroundProc::frg_proc_help_image;
IplImage* ForegroundProc::frg_toggle_help_image;


ForegroundProc::ForegroundProc(CvSize image_size,frg_struct *frg_params_init,morph_struct *morph_params_init) {
	// initializing state images
	bkg_grad=new img_grad;
	frg_grad=new img_grad;

	frg_proc_help_window=new char[MAX_NAME];
	sprintf(frg_proc_help_window,"Blob Tracking Help");

	frg_toggle_help_window=new char[MAX_NAME];
	sprintf(frg_toggle_help_window,"Blob Matching Help");

	frg_proc_help_image=cvLoadImage("help/frg_proc_help.jpg");
	frg_toggle_help_image=cvLoadImage("help/frg_toggle_help.jpg");

		
	params=new frg_struct;	
	initParams(frg_params_init);
	updateParams();

	morph_params=new morph_struct;
	initMorphParams(morph_params_init);

	//cout<<"Done with params initialization\n";
	//updateParameters();	

	stat_obj=new RunningStatScalar;

	state_variables_initialized=false;
	initStateVariables(image_size);

	//cout<<"Done with getWindowExtentForImage\n";	
}

ForegroundProc::~ForegroundProc() {

	clearStateVariables();

	delete(bkg_grad);
	delete(frg_grad);
	delete(frg_proc_help_window);
	delete(frg_toggle_help_window);
	delete(params);
	delete(morph_params);
	delete(stat_obj);
}

void ForegroundProc::initStateVariables(CvSize image_size){

	clearStateVariables();
	
	bkg_gray = cvCreateImage(image_size, IPL_DEPTH_8U, 1);
	frg_gray = cvCreateImage(image_size, IPL_DEPTH_8U, 1);
	
	//cout<<"Done with image allocation\n";

	bkg_grad->grad_x=cvCreateImage(image_size, IPL_DEPTH_8U, 1);
	bkg_grad->grad_y=cvCreateImage(image_size, IPL_DEPTH_8U, 1);
	frg_grad->grad_x=cvCreateImage(image_size, IPL_DEPTH_8U, 1);
	frg_grad->grad_y=cvCreateImage(image_size, IPL_DEPTH_8U, 1);
	//cout<<"Done with img_grad allocation\n";

	//Initializing the matrix of window extents	
	getWindowExtentForImage(img_window_extents,params->neighbourhood_size,image_size);
	getWindowExtentForImage(shadow_window_extents,cvSize(3,3),image_size);

	shadow_method_updated=false;

	state_variables_initialized=true;

}

void ForegroundProc::clearStateVariables(){

	if(!state_variables_initialized)
		return;

	bkg_gray.freeImage();
	frg_gray.freeImage();

	bkg_grad->grad_x.freeImage();
	bkg_grad->grad_y.freeImage();
	frg_grad->grad_x.freeImage();
	frg_grad->grad_y.freeImage();

	clearWindowExtentForImage(img_window_extents);
	clearWindowExtentForImage(shadow_window_extents);

	state_variables_initialized=false;
}

void ForegroundProc::printProcParams(){

	cout<<"\n";

	cout<<"Foreground processing parameters updated:\n";

	if(!params->perform_foreground_analysis){
		cout<<"\nForeground processing disabled.\n";
	}else{
		cout<<"frg_similarity_threshold="<<params->frg_similarity_threshold<<"\t";
		if(params->shadow_detection_method==NO_SHADOW_DETECTION){
			cout<<"\nShadow detection disabled.\n";
		}else if(params->shadow_detection_method==COMPLEX_NCC){
			cout<<"complex_ncc_threshold="<<params->complex_ncc_threshold<<"\t";
			cout<<"min_frame_intensity="<<params->min_frame_intensity<<"\t";
		}else if(params->shadow_detection_method==SIMPLE_NCC){
			cout<<"simple_ncc_threshold="<<params->simple_ncc_threshold<<"\t";
			if(!params->shadow_refinement){
				cout<<"\nShadow refinement disabled.\n";

			}else{
				cout<<"min_intensity_ratio="<<params->min_intensity_ratio<<"\t";
				cout<<"intensity_ratio_std_threshold="<<params->intensity_ratio_std_threshold<<"\t";
			}
		}
	}
	cout<<"\n";
}

void ForegroundProc::printToggleParams(int){	

	cout<<"\n";

	cout<<"Foreground toggle parameters updated:\n";

	cout<<"perform_foreground_analysis="<<params->perform_foreground_analysis<<"\t";
	cout<<"shadow_refinement="<<params->shadow_refinement<<"\t";	

	cout<<"\n";

}

void ForegroundProc::printMorphParams(int){

	cout<<"\n";

	cout<<"Morphological processing parameters updated:\n";

	cout<<"performMorphologicalOp="<<morph_params->performMorphologicalOp<<"\t";
	cout<<"performClosing="<<morph_params->performClosing<<"\t";
	cout<<"performOpening="<<morph_params->performOpening<<"\n";
	cout<<"no_of_iterations="<<morph_params->no_of_iterations<<"\n";

	cout<<"\n";

}
// wrapper function for detecting both shadows and sudden lighting changes
void ForegroundProc::removeFalseForeground(IplImage* bkg_img,IplImage* frg_img,BwImage& frg_mask) {

	//cout<<"Starting removeFalseForeground \n";

	if(params->shadow_detection_method==SIMPLE_NCC) {		
		isShadow=&ForegroundProc::detectShadowUsingSimpleNCC;
	}else if(params->shadow_detection_method==COMPLEX_NCC) {		
		isShadow=&ForegroundProc::detectShadowUsingComplexNCC;
	}else if(params->shadow_detection_method!=NO_SHADOW_DETECTION){
		std::cout<<"Invalid shadow detection method specified.\n";
		return;
	}

	int width=frg_img->width,height=frg_img->height;
	if((width!=bkg_img->width)||(height!=bkg_img->height)) {
		std::cout<<"Incompatible background and current frames\n";
		return;
	}
	//std::cout<<"width="<<width<<" height="<<height<<"\n";     

	bkg_gray.Clear();
	frg_gray.Clear();
	bkg_grad->grad_x.Clear();
	bkg_grad->grad_y.Clear();
	frg_grad->grad_x.Clear();
	frg_grad->grad_y.Clear();

    /***************************** Using C method *********************************/

    /****************** Processing background image***********************/
    //std::cout<<"About to start processing background image\n";
    cvCvtColor(bkg_img,bkg_gray.Ptr(),CV_BGR2GRAY);
    cvSobel( bkg_gray.Ptr(), bkg_grad->grad_x.Ptr(),1,0,3);
    cvSobel( bkg_gray.Ptr(), bkg_grad->grad_y.Ptr(),0,1,3);
    //std::cout<<"Done processing background image\n";

    /****************** Processing foreground image***************************/
    //std::cout<<"About to start processing foreground image\n";
    cvCvtColor(frg_img, frg_gray.Ptr(), CV_BGR2GRAY);
    cvSobel( frg_gray.Ptr(),frg_grad->grad_x.Ptr(),1,0,3);
    cvSobel( frg_gray.Ptr(),frg_grad->grad_y.Ptr(),0,1,3);
    //std::cout<<"Done processing foreground image\n";

    /*cvShowImage( "bkg_gray", bkg_gray.Ptr());
    cvShowImage( "frg_gray", frg_gray.Ptr());
    cvShowImage( "frg_grad_x", frg_grad.grad_x.Ptr());
    cvShowImage( "frg_grad_y", frg_grad.grad_y.Ptr());
    cvShowImage( "bkg_grad_x", bkg_grad.grad_x.Ptr());
    cvShowImage( "bkg_grad_y", bkg_grad.grad_y.Ptr());
    return;*/

    for(int r=0; r<height; r++) {
        for(int c=0; c<width; c++) {

            if(frg_mask(r,c)==BACKGROUND_COLOR)
                continue;

            //getWindowExtent(texture_neigh_size,current_point,image_size,&neigh_window_extent);

            //std::cout<<"About to get intensity\n";
            //std::cout<<"About to compare\n";
            //std::cout<<"About to run getTextureSimilarity\n";            
            //std::cout<<"texture_similarity="<<texture_similarity<<"\n";
            //std::cout<<"Done running getTextureSimilarity\n";

            if(!detectLightingChange(r,c)) {
                //std::cout<<"About to assign\n";
                frg_mask(r,c)=BACKGROUND_COLOR;
				//frg_mask(r,c)=0;
            } else if(params->shadow_detection_method!=NO_SHADOW_DETECTION) {
                //std::cout<<"About to run isShadow\n";
                //std::cout<<"intensity_cross_correlation="<<intensity_cross_correlation<<"\n";
                //std::cout<<"Done running getTextureSimilarity\n";
                if((this->*isShadow)(r,c)) {
                    frg_mask(r,c)=BACKGROUND_COLOR;
					//frg_mask(r,c)=100;
                }
            }
        }
    }
	//cout<<"Done removeFalseForeground \n";

}
// uses Complex NCC based approach to detect shadows
bool ForegroundProc::detectShadowUsingComplexNCC(int r,int c) {

    //std::cout<<"Using Complex NCC\n";

    if((getComplexNCC(r,c)>params->complex_ncc_threshold_sqr)&&((double)frg_gray(r,c)>=params->min_frame_intensity))
        return true;

    return false;

}
// uses Simple NCC based approach to detect shadows
bool ForegroundProc::detectShadowUsingSimpleNCC(int r,int c) {

	//std::cout<<"Using Simple NCC\n";
	//double current_frame_intensity=frg_gray(r,c);

	if(isShadowCandidate(r,c)) {
		if((!params->shadow_refinement)||(!detectFalseShadow(r,c)))
			return true;
	}

	return false;

}
// refines the candidate shadow pixels detected by the function isShadowCandidate
bool ForegroundProc::detectFalseShadow(int r,int c){

	/*ofstream fout;
	fout.open("running_stat.txt");*/

	stat_obj->clearAll();

	int min_row=shadow_window_extents[r][c]->min_row;
	int max_row=shadow_window_extents[r][c]->max_row;
	int min_col=shadow_window_extents[r][c]->min_col;
	int max_col=shadow_window_extents[r][c]->max_col;

	//cout<<"\nIntensity Ratios:\n";

	for(int row=min_row; row<=max_row; row++) {
		for(int col=min_col; col<=max_col; col++) {

			double current_frame_intensity=(double)frg_gray(row,col);
			double current_bkg_intensity=(double)bkg_gray(row,col);

			if((current_frame_intensity<0)||(current_bkg_intensity<0)){
				cout<<"\n";
				cout<<"Encountered negative intensity.\n";
				cout<<"current_frame_intensity="<<current_frame_intensity<<"\t";
				cout<<"current_bkg_intensity="<<current_bkg_intensity<<"\n";
				cout<<"\n";
				_getch();
			}

			double current_ratio=(current_frame_intensity+1)/(current_bkg_intensity+1);
			if((current_ratio<params->min_intensity_ratio)||(current_ratio>=1))
				return true;
			stat_obj->addElement(current_ratio); 
			//cout<<current_ratio<<"\t";
			//fout<<current_ratio<<"\n";
		}
		//cout<<"\n";
	}	
	double ratio_var=stat_obj->var();

	//cout<<"\nStandard Deviation: "<<ratio_std<<"\t Mean: "<<stat_obj->final_mean<<"\n";
	//fout<<"\nStandard Deviation: "<<ratio_std<<"\t Mean: "<<stat_obj->final_mean<<"\n";
	//fout.close();
	//_getch();

	if((ratio_var>=params->intensity_ratio_var_threshold))
		return true;

	return false;
	
}
// detect regions of sudden lighting change by a comparison of the gradient images of current and background images
bool ForegroundProc::detectLightingChange(int r,int c) {
    double frg_grad_x,frg_grad_y;
    double bkg_grad_x,bkg_grad_y;
    
    double bkg_grad_magnitude_square,frg_grad_magnitude_square;
    double temp_num,temp_den;
    double sum_num=0.0, sum_den=0.0;
    double texture_similarity;

    int min_row=img_window_extents[r][c]->min_row;
    int max_row=img_window_extents[r][c]->max_row;
    int min_col=img_window_extents[r][c]->min_col;
    int max_col=img_window_extents[r][c]->max_col;

    for(int row=min_row; row<=max_row; row++) {
        for(int col=min_col; col<=max_col; col++) {
            frg_grad_x=(double)frg_grad->grad_x(row,col);
            frg_grad_y=(double)frg_grad->grad_y(row,col);
            bkg_grad_x=(double)bkg_grad->grad_x(row,col);
            bkg_grad_y=(double)bkg_grad->grad_y(row,col);

			if((frg_grad_x<0)||(frg_grad_y<0)){
				cout<<"\n";
				cout<<"Encountered negative intensity.\n";
				cout<<"frg_grad_x="<<frg_grad_x<<"\t";
				cout<<"frg_grad_y="<<frg_grad_y<<"\n";
				cout<<"\n";
				_getch();
			}

			if((bkg_grad_x<0)||(bkg_grad_y<0)){
				cout<<"\n";
				cout<<"Encountered negative intensity.\n";
				cout<<"bkg_grad_x="<<bkg_grad_x<<"\t";
				cout<<"bkg_grad_y="<<bkg_grad_y<<"\n";
				cout<<"\n";
				_getch();
			}

            frg_grad_magnitude_square=(frg_grad_x*frg_grad_x)+(frg_grad_y*frg_grad_y);
            bkg_grad_magnitude_square=(bkg_grad_x*bkg_grad_x)+(bkg_grad_y*bkg_grad_y);

            temp_num=2.0*((frg_grad_x*bkg_grad_x)+(frg_grad_y*bkg_grad_y));
            temp_den=frg_grad_magnitude_square+bkg_grad_magnitude_square;

            sum_num+=temp_num;
            sum_den+=temp_den;
        }
    }
    if(sum_den==0) {
        /*std::cout<<"Encountered divide by zero error in getTextureSimilarity\n";
        std::cout<<"sum_num="<<sum_num<<" sum_den="<<sum_den<<"\n";*/
        return true;
	}
	//std::cout<<"sum_num="<<sum_num<<" sum_den="<<sum_den<<"\n";

	texture_similarity=sum_num/sum_den;
	if(texture_similarity>=params->frg_similarity_threshold)
		return false;

	return true;
}

// calculates the complex NCC value for the given image
double ForegroundProc::getComplexNCC(int r,int c) {
    double current_frame_intensity,current_bkg_intensity;
    double frame_sum=0.0,bkg_sum=0.0;
    double frame_square_sum=0.0,bkg_square_sum=0.0;
    double frame_bkg_sum=0.0;
    double no_of_pixels=0.0;

    int min_row=img_window_extents[r][c]->min_row;
    int max_row=img_window_extents[r][c]->max_row;
    int min_col=img_window_extents[r][c]->min_col;
    int max_col=img_window_extents[r][c]->max_col;


    for(int row=min_row; row<=max_row; row++) {
        for(int col=min_col; col<=max_col; col++) {
            current_frame_intensity=(double)frg_gray(row,col);
            current_bkg_intensity=(double)bkg_gray(row,col);

			if((current_frame_intensity<0)||(current_bkg_intensity<0)){
				cout<<"\n";
				cout<<"Encountered negative intensity.\n";
				cout<<"current_frame_intensity="<<current_frame_intensity<<"\t";
				cout<<"current_bkg_intensity="<<current_bkg_intensity<<"\n";
				cout<<"\n";
				_getch();
			}

            frame_sum+=current_frame_intensity;
            bkg_sum+=current_bkg_intensity;
            frame_square_sum+=current_frame_intensity*current_frame_intensity;
            bkg_square_sum+=current_bkg_intensity*current_bkg_intensity;
            frame_bkg_sum+=current_frame_intensity*current_bkg_intensity;

            no_of_pixels++;
        }
    }
    /*std::cout<<"frame_sum="<<frame_sum<<" bkg_sum="<<bkg_sum<<"\n";
    std::cout<<"frame_square_sum="<<frame_square_sum<<" bkg_square_sum="<<bkg_square_sum<<"\n";
    std::cout<<"frame_bkg_sum="<<frame_bkg_sum<<"\n";*/


    double numerator=frame_bkg_sum-((frame_sum*bkg_sum)/no_of_pixels);
	numerator=numerator*numerator;

    double denominator_frame=frame_square_sum-((frame_sum*frame_sum)/no_of_pixels);
    double denominator_bkg=bkg_square_sum-((bkg_sum*bkg_sum)/no_of_pixels);
    double denominator=denominator_frame*denominator_bkg;

    if(denominator==0) {
        //std::cout<<"Encountered divide by zero error in getIntensityCrossCorrelation\n";
        return 0;
    }
    //std::cout<<"numerator="<<numerator<<" denominator="<<denominator<<"\n";

    double intensity_cc_sqrt=numerator/denominator;	

    return intensity_cc_sqrt;
}
// determines if a given pixel is a candidate for being a shadow pixel
bool ForegroundProc::isShadowCandidate(int r,int c) {

    double current_frame_intensity,current_bkg_intensity;
    double frame_square_sum=0.0,bkg_square_sum=0.0;
    double frame_bkg_sum=0.0;   

    int min_row=img_window_extents[r][c]->min_row;
    int max_row=img_window_extents[r][c]->max_row;
    int min_col=img_window_extents[r][c]->min_col;
    int max_col=img_window_extents[r][c]->max_col;


    for(int row=min_row; row<=max_row; row++) {
        for(int col=min_col; col<=max_col; col++) {
            current_frame_intensity=(double)frg_gray(row,col);
            current_bkg_intensity=(double)bkg_gray(row,col);
            frame_square_sum+=current_frame_intensity*current_frame_intensity;
            bkg_square_sum+=current_bkg_intensity*current_bkg_intensity;
            frame_bkg_sum+=current_frame_intensity*current_bkg_intensity;
        }
    }

    /*std::cout<<"frame_sum="<<frame_sum<<" bkg_sum="<<bkg_sum<<"\n";
    std::cout<<"frame_square_sum="<<frame_square_sum<<" bkg_square_sum="<<bkg_square_sum<<"\n";
    std::cout<<"frame_bkg_sum="<<frame_bkg_sum<<"\n";*/


    double numerator=frame_bkg_sum*frame_bkg_sum;
    double denominator=frame_square_sum*bkg_square_sum;

    if(denominator==0) {
        //std::cout<<"Encountered divide by zero error in getIntensityCrossCorrelation\n";
        return false;
    }
    //std::cout<<"numerator="<<numerator<<" denominator="<<denominator<<"\n";

    double simple_ncc_sqr=numerator/denominator;

    if((simple_ncc_sqr>=params->simple_ncc_threshold_sqr)&&(frame_square_sum<bkg_square_sum))
        return true;

    return false;
}
// gets the maximum and minimum row and column values for a filtering window of the given size when it is centered at the pixel at the given location
CvWindow* ForegroundProc::getWindowExtentForPixel(CvSize window_size,int r,int c,CvSize image_size) {

    CvWindow *extent=new CvWindow;

    int row_disp=(int)(window_size.height/2.0);
    int col_disp=(int)(window_size.width/2.0);


    extent->min_row=r-row_disp;
    extent->max_row=r+row_disp;

    if(extent->min_row<0)
        extent->min_row=0;
    if(extent->max_row>=image_size.height)
        extent->max_row=image_size.height-1;

    extent->min_col=c-col_disp;
    extent->max_col=c+col_disp;

    if(extent->min_col<0)
        extent->min_col=0;
    if(extent->max_col>=image_size.width)
        extent->max_col=image_size.width-1;

    return extent;

}
// evaluates the filtering window extent for all the pixels in the image
void ForegroundProc::getWindowExtentForImage(CvWindowMat &window_extents,CvSize window_size,CvSize image_size) {

	//char c;
	window_extents.resize(image_size.height);
	for(int r=0; r<image_size.height; r++) {
		window_extents[r].resize(image_size.width);
		for(int c=0; c<image_size.width; c++) {
			window_extents[r][c]=getWindowExtentForPixel(window_size,r,c,image_size);
			/*std::cout<<"r="<<r<<" c="<<c;
			std::cout<<" min_row="<<img_window_extents[r][c]->min_row<<" max_row="<<img_window_extents[r][c]->max_row;
			std::cout<<" min_col="<<img_window_extents[r][c]->min_col<<" max_col="<<img_window_extents[r][c]->max_col<<"\n";*/
		}
		//std::cin>>c;
	}
	
}

void ForegroundProc::clearWindowExtentForImage(CvWindowMat &window_extents){
	int nrows=window_extents.size();
	for(int r=0;r<nrows;r++){
		int ncols=window_extents[r].size();
		for(int c=0;c<ncols;c++){
			if(window_extents[r][c])
				delete(window_extents[r][c]);
		}
	}
}

void ForegroundProc::initParams(frg_struct *frg_params_init) {

	params->shadow_detection_method=frg_params_init->shadow_detection_method;	
	params->shadow_refinement=frg_params_init->shadow_refinement;	
	params->perform_foreground_analysis=frg_params_init->perform_foreground_analysis;
	

	params->neighbourhood_size=cvSize(5,5);

	/*if(params->shadow_detection_method==SIMPLE_NCC){
		params->shadow_similarity_threshold_percent=95;
	}else if(params->shadow_detection_method==COMPLEX_NCC){
		params->shadow_similarity_threshold_percent=60;	}*/

	params->simple_ncc_threshold_percent=frg_params_init->simple_ncc_threshold_percent;
	params->complex_ncc_threshold_percent=frg_params_init->complex_ncc_threshold_percent;

	params->frg_similarity_threshold_percent=frg_params_init->frg_similarity_threshold_percent;
	params->min_frame_intensity=frg_params_init->min_frame_intensity;
	params->min_intensity_ratio_percent=frg_params_init->min_intensity_ratio_percent;
	params->intensity_ratio_std_threshold_percent=frg_params_init->intensity_ratio_std_threshold_percent;


}

void ForegroundProc::initMorphParams(morph_struct *morph_params_init){

	morph_params->performMorphologicalOp=morph_params_init->performMorphologicalOp;
	morph_params->performOpening=morph_params_init->performOpening;
	morph_params->performClosing=morph_params_init->performClosing;
	morph_params->no_of_iterations=morph_params_init->no_of_iterations;
}

void ForegroundProc::updateParams(int) {	

	params->frg_similarity_threshold=(double)(params->frg_similarity_threshold_percent)/100.0;	
	params->min_intensity_ratio=(double)(params->min_intensity_ratio_percent)/100.0;
	params->intensity_ratio_std_threshold=(double)(params->intensity_ratio_std_threshold_percent)/100.0;

	params->simple_ncc_threshold=(double)(params->simple_ncc_threshold_percent)/100.0;
	params->complex_ncc_threshold=(double)(params->complex_ncc_threshold_percent)/100.0;

	params->simple_ncc_threshold_sqr=params->simple_ncc_threshold*params->simple_ncc_threshold;
	params->complex_ncc_threshold_sqr=params->complex_ncc_threshold*params->complex_ncc_threshold;

	params->intensity_ratio_var_threshold=params->intensity_ratio_std_threshold*params->intensity_ratio_std_threshold;

	printProcParams();
}
void ForegroundProc::updateShadowMethod(int){	

	cout<<"\nshadow_detection_method="<<params->shadow_detection_method<<"\n";
	shadow_method_updated=true;

}

void ForegroundProc::showForegroundProcHelpWindow(int mouse_event,int x,int y,int flags,void* param){

	if(mouse_event==CV_EVENT_LBUTTONDOWN){
		cvNamedWindow(frg_proc_help_window,1);
		cvShowImage(frg_proc_help_window,frg_proc_help_image);
	}else if(mouse_event==CV_EVENT_RBUTTONDOWN){
		cvDestroyWindow(frg_proc_help_window);
	}
}

void ForegroundProc::showForegroundToggleHelpWindow(int mouse_event,int x,int y,int flags,void* param){

	if(mouse_event==CV_EVENT_LBUTTONDOWN){
		cvNamedWindow(frg_toggle_help_window,1);
		cvShowImage(frg_toggle_help_window,frg_toggle_help_image);
	}else if(mouse_event==CV_EVENT_RBUTTONDOWN){
		cvDestroyWindow(frg_toggle_help_window);
	}
}

void ForegroundProc::initWindow(char *window_name){

	cvNamedWindow(window_name,1);	

	cvCreateTrackbar("frg%",window_name,&(params->frg_similarity_threshold_percent),100,updateParams);	

	if(params->shadow_detection_method==SIMPLE_NCC){
		cvCreateTrackbar("simpleNCC%",window_name,&(params->simple_ncc_threshold_percent),100,updateParams);
		cvCreateTrackbar("minRatio%",window_name,&(params->min_intensity_ratio_percent),100,updateParams);
		cvCreateTrackbar("ratioStd%",window_name,&(params->intensity_ratio_std_threshold_percent),100,updateParams);
	} else if(params->shadow_detection_method==COMPLEX_NCC){
		cvCreateTrackbar("complexNCC%",window_name,&(params->complex_ncc_threshold_percent),100,updateParams);
		cvCreateTrackbar("minIntensity",window_name,&(params->min_frame_intensity),100,updateParams);
	}

	cvSetMouseCallback(window_name,showForegroundProcHelpWindow,(void*)this);
}

void ForegroundProc::initMorphWindow(char *window_name){

	cvNamedWindow(window_name,1);		

	cvCreateTrackbar("frgAnalysis",window_name,&(params->perform_foreground_analysis),1,printToggleParams);		
	cvCreateTrackbar("shadowRefine",window_name,&(params->shadow_refinement),1,printToggleParams);
	cvCreateTrackbar("shadowMethod",window_name,&(params->shadow_detection_method),2,updateShadowMethod);
	cvCreateTrackbar("morphology",window_name,&(morph_params->performMorphologicalOp),1,printMorphParams);
	cvCreateTrackbar("opening",window_name,&(morph_params->performOpening),1,printMorphParams);
	cvCreateTrackbar("closing",window_name,&(morph_params->performClosing),1,printMorphParams);	
	cvCreateTrackbar("iterations",window_name,&(morph_params->no_of_iterations),10,printMorphParams);	

	cvSetMouseCallback(window_name,showForegroundToggleHelpWindow,(void*)this);
}

/*
//Using C++ method
Mat bkg_img_mat(bkg_img.Ptr());
Mat bkg_gray_mat(bkg_gray.Ptr());
Mat frg_img_mat(frg_img.Ptr());
Mat frg_gray_mat(frg_gray.Ptr());

///Processing background image

GaussianBlur( bkg_img_mat, bkg_img_mat, Size(3,3), 0, 0, BORDER_DEFAULT );

/// Convert it to gray
cvtColor(bkg_img_mat, bkg_gray_mat, CV_RGB2GRAY );
/// Gradient X
Scharr( bkg_gray_mat, bkg_grad.grad_x, filter_parameters.ddepth, 1, 0, filter_parameters.scale, filter_parameters.delta, BORDER_DEFAULT );
//Sobel( bkg_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
convertScaleAbs( bkg_grad.grad_x, bkg_grad.abs_grad_x );

/// Gradient Y
Scharr( bkg_gray_mat, bkg_grad.grad_y, filter_parameters.ddepth, 0, 1, filter_parameters.scale, filter_parameters.delta, BORDER_DEFAULT );
//Sobel( bkg_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
convertScaleAbs( bkg_grad.grad_y, bkg_grad.abs_grad_y );

/// Total Gradient (approximate)
addWeighted( bkg_grad.abs_grad_y, 0.5, bkg_grad.abs_grad_y, 0.5, 0, bkg_grad.grad_overall);


// Processing foreground image

GaussianBlur( frg_img_mat, frg_img_mat, Size(3,3), 0, 0, BORDER_DEFAULT );

/// Convert it to gray
cvtColor( frg_img_mat, frg_gray_mat, CV_RGB2GRAY );
/// Gradient X
Scharr( frg_gray_mat, frg_grad.grad_x, filter_parameters.ddepth, 1, 0, filter_parameters.scale, filter_parameters.delta, BORDER_DEFAULT );
//Sobel( bkg_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
convertScaleAbs( frg_grad.grad_x, frg_grad.abs_grad_x );

/// Gradient Y
Scharr(frg_gray_mat, frg_grad.grad_y, filter_parameters.ddepth, 0, 1, filter_parameters.scale, filter_parameters.delta, BORDER_DEFAULT );
//Sobel( bkg_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
convertScaleAbs( frg_grad.grad_y, frg_grad.abs_grad_y );

/// Total Gradient (approximate)
addWeighted( frg_grad.abs_grad_y, 0.5, frg_grad.abs_grad_y, 0.5, 0, frg_grad.grad_overall );
*/
