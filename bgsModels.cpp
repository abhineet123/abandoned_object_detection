#include "bgsModels.hpp"

bgs_struct* BgsModels::bgs_params;

GrimsonParams* BgsModels::grimson_params;
GrimsonGMM* BgsModels::grimson_bgs;	

ZivkovicParams* BgsModels::zivkovic_params;
ZivkovicAGMM* BgsModels::zivkovic_bgs;

AdaptiveMedianParams* BgsModels::median_params;
AdaptiveMedianBGS* BgsModels::median_bgs;

WrenParams* BgsModels::rg_params;
WrenGA* BgsModels::rg_bgs;

bool BgsModels::bgs_method_changed;

char* BgsModels::bgs_help_window;
IplImage* BgsModels::bgs_help_image;

BgsModels::BgsModels(CvSize frame_size,bgs_struct *bgs_params_init){
	

	bgs_help_window=new char[MAX_NAME];
	sprintf(bgs_help_window,"BGS Help");	
	
	bgs_params=new bgs_struct;
	
	initBGSToggleParams(bgs_params_init->bgs_toggle_params);
	initBGSModels();
	initBGSParams(bgs_params_init);
	initStateVariables(frame_size);
	initBGSMethod();	

	bgs_method_changed=false;
	//cout<<"Done with BGS setup\n";
}
BgsModels::~BgsModels(){
	clearBGSModels();
	delete(bgs_params->bgs_toggle_params);
	delete(bgs_help_window);
	delete(bgs_params);
}
void BgsModels::initBGSToggleParams(bgs_toggle_struct *bgs_toggle_params_init){

	
	bgs_params->bgs_toggle_params=new bgs_toggle_struct;
	bgs_params->bgs_toggle_params->bgs_method=bgs_toggle_params_init->bgs_method;
	bgs_params->bgs_toggle_params->frg_thr_percent=bgs_toggle_params_init->frg_thr_percent;
	bgs_params->bgs_toggle_params->max_frg_thr_count=bgs_toggle_params_init->max_frg_thr_count;

	updateBGSToggleParams(0);
}

void BgsModels::initBGSParams(bgs_struct *bgs_params_init){

	initGrimsonGMMParams(bgs_params_init->grimson_params_local);
	updateGrimsonGMMParamsLocal(0);	

	initZivkovicGMMParams(bgs_params_init->zivkovic_params_local);
	updateZivkovicGMMParamsLocal(0);

	initAdaptiveMedianParams(bgs_params_init->median_params_local);	
	updateAdaptiveMedianParamsLocal(0);	

	initRunningGaussianParams(bgs_params_init->rg_params_local);
	updateRunningGaussianParamsLocal(0);
}

void BgsModels::initStateVariables(CvSize frame_size){

	grimson_params->SetFrameSize(frame_size.width,frame_size.height);	
	grimson_bgs->Initalize(*grimson_params);

	zivkovic_params->SetFrameSize(frame_size.width,frame_size.height);		
	zivkovic_bgs->Initalize(*zivkovic_params);

	median_params->SetFrameSize(frame_size.width,frame_size.height);	
	median_bgs->Initalize(*median_params);

	rg_params->SetFrameSize(frame_size.width,frame_size.height);		
	rg_bgs->Initalize(*rg_params);
}

// allocate objects corresponding to each of the BGS techniques
void BgsModels::initBGSModels(){

	//Grimson GMM BGS
	grimson_params=new GrimsonParams();
	grimson_bgs=new GrimsonGMM();
	bgs_params->grimson_params_local=new gmm_struct;

	//Zivkovic GMM BGS
	zivkovic_params=new ZivkovicParams();
	zivkovic_bgs=new ZivkovicAGMM();
	bgs_params->zivkovic_params_local=new gmm_struct;	

	//Adaptive Median BGS
	median_params=new AdaptiveMedianParams();
	median_bgs=new AdaptiveMedianBGS();
	bgs_params->median_params_local=new adaptive_median_struct;	

	//Running Gaussian BGS
	rg_params=new WrenParams();
	rg_bgs=new WrenGA();
	bgs_params->rg_params_local=new running_gaussian_struct;
}
void BgsModels::clearBGSModels(){

	//Grimson GMM BGS
	delete(grimson_params);
	delete(grimson_bgs);
	delete(bgs_params->grimson_params_local);

	delete(zivkovic_params);
	delete(zivkovic_bgs);
	delete(bgs_params->zivkovic_params_local);

	delete(median_params);
	delete(median_bgs);
	delete(bgs_params->median_params_local);

	delete(rg_params);
	delete(rg_bgs);
	delete(bgs_params->rg_params_local);
}
// sets the virtual class variables to the corresponding concrete class variable of the selected BGS technique
void BgsModels::initBGSMethod() {

	if(bgs_params->bgs_toggle_params->bgs_method==BGS_GRIMSON_GMM){

		cout<<"\nUsing Grimson GMM BGS method.\n";
		params=grimson_params;
		bgs=grimson_bgs;

	}else if(bgs_params->bgs_toggle_params->bgs_method==BGS_ZIVKOVIC_GMM){

		cout<<"\nUsing Zivkovic GMM BGS method.\n";
		params=zivkovic_params;
		bgs=zivkovic_bgs;

	}else if(bgs_params->bgs_toggle_params->bgs_method==BGS_ADAPTIVE_MEDIAN){

		cout<<"\nUsing Adaptive Median BGS method.\n";
		params=median_params;
		bgs=median_bgs;

	}else if(bgs_params->bgs_toggle_params->bgs_method==BGS_RUNNING_GAUSSIAN){

		cout<<"\nUsing Running Gaussian BGS method.\n";
		params=rg_params;
		bgs=rg_bgs;

	}else{
		cout<<"\nInvalid BGS method.\n";
		exit(0);
	}	
} 

void BgsModels::initGrimsonGMMParams(gmm_struct *bgs_grimson_params_init){
	bgs_params->grimson_params_local->max_std_10=bgs_grimson_params_init->max_std_10;
	bgs_params->grimson_params_local->alpha_1000=bgs_grimson_params_init->alpha_1000;
	bgs_params->grimson_params_local->max_modes=bgs_grimson_params_init->max_modes;

}

void BgsModels::updateGrimsonGMMParamsLocal(int) {

	bgs_params->grimson_params_local->max_std=(double)(bgs_params->grimson_params_local->max_std_10)/10.0;
	bgs_params->grimson_params_local->alpha=(double)bgs_params->grimson_params_local->alpha_1000/1000.0;
	updateGrimsonGMMParamsGlobal();
	printGrimsonGMMParams();
}

void BgsModels::updateGrimsonGMMParamsGlobal(){

	grimson_params->LowThreshold() =(float)(bgs_params->grimson_params_local->max_std*bgs_params->grimson_params_local->max_std);
	grimson_params->HighThreshold() = 2.0f*grimson_params->LowThreshold();
	grimson_params->Alpha() = (float)(bgs_params->grimson_params_local->alpha);
	grimson_params->MaxModes() = bgs_params->grimson_params_local->max_modes;	
	grimson_bgs->UpdateParams(*grimson_params);
}

void BgsModels::printGrimsonGMMParams(){

	cout<<"\n";

	cout<<"Grimson GMM BGS parameters updated:\n";

	cout<<"LowThreshold="<<grimson_params->LowThreshold()<<"\t";
	cout<<"HighThreshold="<<grimson_params->HighThreshold()<<"\t";
	cout<<"Alpha="<<grimson_params->Alpha()<<"\t";
	cout<<"MaxModes="<<grimson_params->MaxModes()<<"\n";	

	cout<<"\n";
}

void BgsModels::initZivkovicGMMParams(gmm_struct *bgs_zivkovic_params_init){
	bgs_params->zivkovic_params_local->max_std_10=bgs_zivkovic_params_init->max_std_10;
	bgs_params->zivkovic_params_local->alpha_1000=bgs_zivkovic_params_init->alpha_1000;
	bgs_params->zivkovic_params_local->max_modes=bgs_zivkovic_params_init->max_modes;

}
void BgsModels::updateZivkovicGMMParamsLocal(int) {

	bgs_params->zivkovic_params_local->max_std=(double)(bgs_params->zivkovic_params_local->max_std_10)/10.0;
	bgs_params->zivkovic_params_local->alpha=(double)bgs_params->zivkovic_params_local->alpha_1000/1000.0;
	updateZivkovicGMMParamsGlobal();
	printZivkovicGMMParams();
}
void BgsModels::updateZivkovicGMMParamsGlobal(){

	zivkovic_params->LowThreshold() = (float)(bgs_params->zivkovic_params_local->max_std*bgs_params->zivkovic_params_local->max_std);
	zivkovic_params->HighThreshold() = 2*zivkovic_params->LowThreshold();
	zivkovic_params->Alpha() = (float)(bgs_params->zivkovic_params_local->alpha);
	zivkovic_params->MaxModes() = bgs_params->zivkovic_params_local->max_modes;	

	zivkovic_bgs->UpdateParams(*zivkovic_params);
}
void BgsModels::printZivkovicGMMParams(){

	cout<<"\n";

	cout<<"Zivkovic GMM BGS parameters updated:\n";

	cout<<"LowThreshold="<<zivkovic_params->LowThreshold()<<"\t";
	cout<<"HighThreshold="<<zivkovic_params->HighThreshold()<<"\t";
	cout<<"Alpha="<<zivkovic_params->Alpha()<<"\t";
	cout<<"MaxModes="<<zivkovic_params->MaxModes()<<"\n";	

	cout<<"\n";
}

void BgsModels::initAdaptiveMedianParams(adaptive_median_struct *bgs_median_params_init){	

	bgs_params->median_params_local->low_threshold=bgs_median_params_init->low_threshold;
	bgs_params->median_params_local->sampling_rate=bgs_median_params_init->sampling_rate;
	bgs_params->median_params_local->learning_frames=bgs_median_params_init->learning_frames;

}

void BgsModels::updateAdaptiveMedianParamsLocal(int) {
	updateAdaptiveMedianParamsGlobal();
	printAdaptiveMedianParams();
}

void BgsModels::updateAdaptiveMedianParamsGlobal(){

	median_params->LowThreshold() = bgs_params->median_params_local->low_threshold;
	median_params->HighThreshold() = 2*median_params->LowThreshold();
	median_params->SamplingRate() = bgs_params->median_params_local->sampling_rate;
	median_params->LearningFrames() =bgs_params->median_params_local->learning_frames;

	median_bgs->UpdateParams(*median_params);
}

void BgsModels::printAdaptiveMedianParams(){

	cout<<"\n";

	cout<<"Adaptive Median BGS parameters updated:\n";

	cout<<"LowThreshold="<<(int)median_params->LowThreshold()<<"\t";
	cout<<"HighThreshold="<<(int)median_params->HighThreshold()<<"\t";
	cout<<"Alpha="<<median_params->SamplingRate()<<"\t";
	cout<<"LearningFrames="<<median_params->LearningFrames()<<"\n";	

	cout<<"\n";
}

void BgsModels::initRunningGaussianParams(running_gaussian_struct *bgs_rg_params_init){

	bgs_params->rg_params_local->max_std_10=bgs_rg_params_init->max_std_10;
	bgs_params->rg_params_local->alpha_1000=bgs_rg_params_init->alpha_1000;
	bgs_params->rg_params_local->learning_frames=bgs_rg_params_init->learning_frames;	
}

void BgsModels::updateRunningGaussianParamsLocal(int){

	bgs_params->rg_params_local->max_std=(double)(bgs_params->rg_params_local->max_std_10)/10.0;
	bgs_params->rg_params_local->alpha=(double)bgs_params->rg_params_local->alpha_1000/1000.0;
	updateRunningGaussianParamsGlobal();
	printRunningGaussianParams();
}
void BgsModels::updateRunningGaussianParamsGlobal(){

	rg_params->LowThreshold() = (float)(bgs_params->rg_params_local->max_std*bgs_params->rg_params_local->max_std);
	rg_params->HighThreshold() = 2*rg_params->LowThreshold();	
	rg_params->Alpha() = (float)(bgs_params->rg_params_local->alpha);
	rg_params->LearningFrames() = bgs_params->rg_params_local->learning_frames;

	rg_bgs->UpdateParams(*rg_params);
}

void BgsModels::printRunningGaussianParams(){

	cout<<"\n";

	cout<<"Running Gaussian BGS parameters updated:\n";

	cout<<"LowThreshold="<<rg_params->LowThreshold()<<"\t";
	cout<<"HighThreshold="<<rg_params->HighThreshold()<<"\t";
	cout<<"Alpha="<<rg_params->Alpha()<<"\t";
	cout<<"LearningFrames="<<rg_params->LearningFrames() <<"\n";	

	cout<<"\n";
}


void BgsModels::updateBGSMethod(int) {
	bgs_method_changed=true;
}

void BgsModels::updateBGSToggleParams(int){

	bgs_params->bgs_toggle_params->frg_thr=(double)bgs_params->bgs_toggle_params->frg_thr_percent/100.0;
	printBGSTogglesParams();
}

void BgsModels::printBGSTogglesParams(){

	cout<<"BGS resetting parameters updated:\n";

	cout<<"frg_thr="<<bgs_params->bgs_toggle_params->frg_thr<<"\t";
	cout<<"max_frg_thr_count="<<bgs_params->bgs_toggle_params->max_frg_thr_count<<"\n";
}

void BgsModels::showBGSHelpWindow(int mouse_event,int x,int y,int flags,void* param){

	if(mouse_event==CV_EVENT_LBUTTONDOWN){
		cvNamedWindow(bgs_help_window,1);
		if((bgs_params->bgs_toggle_params->bgs_method==BGS_GRIMSON_GMM)){
			bgs_help_image=cvLoadImage("help/grimson_gmm_help.jpg");
		}else if(bgs_params->bgs_toggle_params->bgs_method==BGS_ZIVKOVIC_GMM){
			bgs_help_image=cvLoadImage("help/zivkovic_gmm_help.jpg");
		}else if(bgs_params->bgs_toggle_params->bgs_method==BGS_ADAPTIVE_MEDIAN){
			bgs_help_image=cvLoadImage("help/adaptive_median_help.jpg");
		}else if(bgs_params->bgs_toggle_params->bgs_method==BGS_RUNNING_GAUSSIAN){
			bgs_help_image=cvLoadImage("help/running_gaussian_help.jpg");
		}
		cvShowImage(bgs_help_window,bgs_help_image);
	}else if(mouse_event==CV_EVENT_RBUTTONDOWN){
		cvDestroyWindow(bgs_help_window);
	}
}

void BgsModels::initWindow(char *window_name){

	cvNamedWindow(window_name, 1 );

	cvCreateTrackbar("bgsMethod",window_name,&(bgs_params->bgs_toggle_params->bgs_method),MAX_BGS_ID,updateBGSMethod);	
	cvCreateTrackbar("frgThr%",window_name,&(bgs_params->bgs_toggle_params->frg_thr_percent),100,updateBGSToggleParams);
	cvCreateTrackbar("frgCount",window_name,&(bgs_params->bgs_toggle_params->max_frg_thr_count),1000,updateBGSToggleParams);

	if((bgs_params->bgs_toggle_params->bgs_method==BGS_GRIMSON_GMM)){

		cvCreateTrackbar("maxStd10",window_name,&(bgs_params->grimson_params_local->max_std_10),100,updateGrimsonGMMParamsLocal);
		cvCreateTrackbar("alpha1000",window_name,&(bgs_params->grimson_params_local->alpha_1000),100,updateGrimsonGMMParamsLocal);
		cvCreateTrackbar("maxModes",window_name,&(bgs_params->grimson_params_local->max_modes),10,updateGrimsonGMMParamsLocal);

	}else if(bgs_params->bgs_toggle_params->bgs_method==BGS_ZIVKOVIC_GMM){

		cvCreateTrackbar("maxStd10",window_name,&(bgs_params->zivkovic_params_local->max_std_10),100,updateZivkovicGMMParamsLocal);
		cvCreateTrackbar("alpha1000",window_name,&(bgs_params->zivkovic_params_local->alpha_1000),100,updateZivkovicGMMParamsLocal);
		cvCreateTrackbar("maxModes",window_name,&(bgs_params->zivkovic_params_local->max_modes),10,updateZivkovicGMMParamsLocal);

	}else if(bgs_params->bgs_toggle_params->bgs_method==BGS_ADAPTIVE_MEDIAN){
		
		cvCreateTrackbar("lowThreshold",window_name,&(bgs_params->median_params_local->low_threshold),100,updateAdaptiveMedianParamsLocal);
		cvCreateTrackbar("learningFrames",window_name,&(bgs_params->median_params_local->learning_frames),100,updateAdaptiveMedianParamsLocal);
		cvCreateTrackbar("samplingRate",window_name,&(bgs_params->median_params_local->sampling_rate),100,updateAdaptiveMedianParamsLocal);

	} else if(bgs_params->bgs_toggle_params->bgs_method==BGS_RUNNING_GAUSSIAN){
		
		cvCreateTrackbar("maxStd10",window_name,&(bgs_params->rg_params_local->max_std_10),100,updateRunningGaussianParamsLocal);
		cvCreateTrackbar("alpha1000",window_name,&(bgs_params->rg_params_local->alpha_1000),100,updateRunningGaussianParamsLocal);
		cvCreateTrackbar("learningFrames",window_name,&(bgs_params->rg_params_local->learning_frames),100,updateRunningGaussianParamsLocal);
	}
	cvSetMouseCallback(window_name,showBGSHelpWindow,(void*)this);
}