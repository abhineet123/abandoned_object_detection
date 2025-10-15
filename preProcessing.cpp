#include "preProcessing.hpp"

pre_process_struct* PreProcessing::params;
bool PreProcessing::noise_reduction_updated;
bool PreProcessing::contrast_enhancement_updated;

char* PreProcessing::pre_process_help_window;
IplImage* PreProcessing::pre_process_help_image;

PreProcessing::PreProcessing(CvSize img_size,pre_process_struct *params_init){

	state_variables_initialized=false;

	pre_process_help_window=new char[MAX_NAME];
	sprintf(pre_process_help_window,"Pre Processing Help");	

	pre_process_help_image=cvLoadImage("help/pre_process_help.jpg");

	initStateVariables(img_size);


	img_hist=cvCreateImage(cvSize(MAX_INTENSITY,HIST_HEIGHT),IPL_DEPTH_8U,1);
	intensity_count=new int[MAX_INTENSITY+1];

	//double kernel_vals[] = {0,-1,0, -1,5,-1, 0,-1,0};

	contrast_enhancement_kernel= cvCreateMat(3,3,CV_32FC1);
	//cvSet(&contrast_enhancement_kernel, cvScalarAll(0), NULL);	
	//contrast_enhancement_kernel= cvMat(3,3,CV_64FC1,kernel_vals);
	//cvInitMatHeader(contrast_enhancement_kernel,3,3,CV_64FC1,kernel_vals);
	//printKernel(contrast_enhancement_kernel);

	params=new pre_process_struct;
	initParams(params_init);
	updateParams(0);
}
PreProcessing::~PreProcessing(){
	clearStateVariables();

	delete(pre_process_help_window);
	delete(intensity_count);
	delete(params);
}
void PreProcessing::initStateVariables(CvSize img_size){

	clearStateVariables();

	img_red=cvCreateImage(img_size,IPL_DEPTH_8U,1);
	img_green=cvCreateImage(img_size,IPL_DEPTH_8U,1);
	img_blue=cvCreateImage(img_size,IPL_DEPTH_8U,1);
	img_gray=cvCreateImage(img_size,IPL_DEPTH_8U,1);
	temp_filter_img=cvCreateImage(img_size,IPL_DEPTH_8U,3);

	stored_img_size=img_size;

	noise_reduction_updated=false;
	contrast_enhancement_updated=false;

	state_variables_initialized=true;
	total_pixel_count=img_size.width*img_size.height;

}

void PreProcessing::clearStateVariables(){
	if(!state_variables_initialized)
		return;
	cvReleaseImage(&img_red);
	cvReleaseImage(&img_green);
	cvReleaseImage(&img_blue);
	cvReleaseImage(&img_gray);
	cvReleaseImage(&temp_filter_img);

	state_variables_initialized=false;
}

void PreProcessing::printKernel(CvMat *kernel){

	cout<<"\n";
	for(int r=0;r<kernel->rows;r++){
		//int pos=r*kernel->step;
		double* ptr = (double*)(kernel->data.ptr+r*kernel->step);
		for(int c=0;c<kernel->cols;c++){
			cout<<*ptr++<<"\t";
		}
		cout<<"\n";
	}
	_getch();
}
// split an RGB image into 3 gray scale images or combine 3 gray scale images into a single RGB image
void PreProcessing::splitOrMerge(IplImage *img_rgb,int op_flag){

	//cout<<"Start splitOrMerge\n";

	for(int r=0;r<img_rgb->height;r++){
		for(int c=0;c<img_rgb->width;c++){

			int pixel_pos_rgb=r*img_rgb->widthStep+c*img_rgb->nChannels;

			int pixel_pos_red=r*img_red->widthStep+c;
			int pixel_pos_green=r*img_green->widthStep+c;
			int pixel_pos_blue=r*img_blue->widthStep+c;

			if(op_flag==IMG_MERGE){

				img_rgb->imageData[pixel_pos_rgb]=img_blue->imageData[pixel_pos_blue];
				img_rgb->imageData[pixel_pos_rgb+1]=img_green->imageData[pixel_pos_green];
				img_rgb->imageData[pixel_pos_rgb+2]=img_red->imageData[pixel_pos_red];	

			}else if(op_flag==IMG_SPLIT){

				img_blue->imageData[pixel_pos_blue]=img_rgb->imageData[pixel_pos_rgb];
				img_green->imageData[pixel_pos_green]=img_rgb->imageData[pixel_pos_rgb+1];
				img_red->imageData[pixel_pos_red]=img_rgb->imageData[pixel_pos_rgb+2];	

			}
		}
	}
	//cout<<"Done splitOrMerge\n";
}
// equalize the histogram of an RGB image
void PreProcessing::equalizeHistogramRGB(IplImage *img){

	//cout<<"Start equalizeHistogramRGB\n";

	if((img->width!=stored_img_size.width)||(img->height!=stored_img_size.height)){
		cout<<"\n Incompatible input image.\n";
		_getch();
	}

	// split the RGB image into 3 gray scale images
	splitOrMerge(img,IMG_SPLIT);

	// equalize the histograms of each of these images
	cvEqualizeHist(img_blue,img_blue);
	cvEqualizeHist(img_green,img_green);
	cvEqualizeHist(img_red,img_red);

	// combine the transformed gray scale images into the output RGB image
	splitOrMerge(img,IMG_MERGE);

	//cout<<"Done equalizeHistogramRGB\n";
}
// normalize the histogram of an RGB image (dummy function: does not work yet)
void PreProcessing::normalizeHistogramRGB(IplImage *img){

	//cout<<"Start equalizeHistogramRGB\n";

	if((img->width!=stored_img_size.width)||(img->height!=stored_img_size.height)){
		cout<<"\n Incompatible input image.\n";
		_getch();
	}

	// split the RGB image into 3 gray scale images
	splitOrMerge(img,IMG_SPLIT);

	/*CvHistogram *hist=cvCreateHist(3,5,CV_HIST_ARRAY);
	cvCalcHist(img,hist);*/

	// normalize the histograms of each of these images
	/*cvNormalizeHist(img_blue,img_blue);
	cvNormalizeHist(img_green,img_green);
	cvNormalizeHist(img_red,img_red);*/

	// combine the transformed gray scale images into the output RGB image
	splitOrMerge(img,IMG_MERGE);

	//cout<<"Done equalizeHistogramRGB\n";
}
// computes, for each intensity value from 0 to 255, the number of pixels in the image having that intensity
int PreProcessing::getIntensityCount(IplImage *img,CvSize image_size){

	int max_count=0;
	unsigned char max_intensity=0;

	for(int i=0;i<=MAX_INTENSITY;i++)
		intensity_count[i]=0;

	for(int r=0;r<image_size.height;r++){
		int location=r*img->widthStep;
		for(int c=0;c<image_size.width;c++){

			unsigned char pixel_intensity=img->imageData[location+c];

			if(pixel_intensity>max_intensity)
				max_intensity=pixel_intensity;
			//cout<<"pixel_intensity="<<pixel_intensity<<"\n";
			intensity_count[pixel_intensity]++;
			if(intensity_count[pixel_intensity]>max_count){
				max_count=intensity_count[pixel_intensity];
				//cout<<"pixel_intensity="<<pixel_intensity<<"\t";
				//cout<<"r="<<r<<"\t c="<<c<<"\t max_count="<<max_count<<"\n";
			}

		}
	}

	//cout<<"max_intensity="<<max_intensity<<"\n";

	/*max_count=0;
	for(int i=0;i<=max_intensity;i++){
	cout<<"intensity_count["<<i<<"]="<<intensity_count[i]<<"\n";
	if(intensity_count[i]>max_count)
	max_count=intensity_count[i];

	}*/
	//cout<<"max_count="<<max_count<<"\n";
	//_getch();
	return max_count;
}
// obtains the pixel intensity values that make up the lower and upper (or minimum and maximum) cutoff percentiles 
void PreProcessing::getCutoffPixelValues(unsigned char &min_cutoff,unsigned char &max_cutoff){

	int min_count_sum=0,max_count_sum=0;
	bool found_min=false;
	bool found_max=false;
	double count_fraction;

	for(int i=0;i<=MAX_INTENSITY;i++){
		if(!found_min){
			min_count_sum+=intensity_count[i];
			count_fraction=(double)min_count_sum/(double)total_pixel_count;
			if(count_fraction>=params->cutoff_fraction){
				found_min=true;
				min_cutoff=i;
			}
		}
		if(!found_max){
			max_count_sum+=intensity_count[MAX_INTENSITY-i];
			count_fraction=(double)max_count_sum/(double)total_pixel_count;
			if(count_fraction>=params->cutoff_fraction){
				found_max=true;
				max_cutoff=MAX_INTENSITY-i;
			}
		}
		if(found_min&&found_max)
			break;
	}
}
// sets all pixels whose intensities are less than the minimum intensity to the minimum intensity; 
// also sets all pixels with intensities higher than the max intensity to the max intensity
void PreProcessing::removeCutoffIntensities(IplImage *img,unsigned char min_intensity,unsigned char max_intensity){

	for(int r=0;r<img->height;r++){
		int location=r*img->widthStep;
		for(int c=0;c<img->width;c++){

			unsigned char pixel_val=img->imageData[location+c];

			if(pixel_val<0)
				cout<<pixel_val<<"\n";

			if(pixel_val<min_intensity)
				img->imageData[location+c]=min_intensity;

			if(pixel_val>max_intensity)
				img->imageData[location+c]=max_intensity;
		}
	}

}
// normalizes the given gray scale image by first setting the minimum and maximum intensities to the specified cutoffs 
// and then applying linear stretching to make them 0 and 255 respectively
void PreProcessing::normalizeImageGS(IplImage *img){

	if(img->nChannels!=1){
		cout<<"\nError in normalizeImageGS:\n";
		cout<<"Input image is not single channel.\n";
		_getch();
	}

	unsigned char min_cutoff,max_cutoff;	

	if(SHOW_PROC_IMG)
		cvShowImage("Original image",img);

	getIntensityCount(img,stored_img_size);

	getCutoffPixelValues(min_cutoff,max_cutoff);

	if(SHOW_PROC_IMG)
		cout<<"min_cutoff="<<(int)min_cutoff<<"\t max_cutoff="<<(int)max_cutoff<<"\n";

	removeCutoffIntensities(img,min_cutoff,max_cutoff);

	if(SHOW_PROC_IMG)
		cvShowImage("Cutoff image",img);

	cvNormalize(img,img,0,MAX_INTENSITY,CV_MINMAX);

	if(SHOW_PROC_IMG)
		cvShowImage("Normalized image",img);
	//cvWaitKey(0);
}
// normalize an RGB image by normalizing its constituent band images and combining the results
void PreProcessing::normalizeImageRGB(IplImage *img){

	//cout<<"Start normalizeImageRGB\n";

	if((img->width!=stored_img_size.width)||(img->height!=stored_img_size.height)){
		cout<<"\nError in normalizeImageRGB:\n";
		cout<<"Incompatible input image.\n";
		_getch();
	}	

	// split the RGB image into 3 gray scale images
	splitOrMerge(img,IMG_SPLIT);	
	// normalize each of these images
	normalizeImageGS(img_blue);
	normalizeImageGS(img_green);
	normalizeImageGS(img_red);	

	// combine the transformed gray scale images into the output RGB image
	splitOrMerge(img,IMG_MERGE);

	//cout<<"Done normalizeImageRGB\n";
}
// apply a contrast enhancing and sharpening filter to the input image
void PreProcessing::filterImageRGB(IplImage *img){

	//cout<<"Start normalizeImageRGB\n";

	if((img->width!=stored_img_size.width)||(img->height!=stored_img_size.height)){
		cout<<"\n Incompatible input image.\n";
		_getch();
	}	

	double kernel_vals[] = {0,-1,0, -1,5,-1, 0,-1,0};		
	cvInitMatHeader(contrast_enhancement_kernel,3,3,CV_64FC1,kernel_vals);

	//printKernel(contrast_enhancement_kernel);

	if (USE_DIRECT_FILTERING){
		cvFilter2D(img, img,contrast_enhancement_kernel);
	}else{

		// split the RGB image into 3 gray scale images
		splitOrMerge(img,IMG_SPLIT);

		// apply the filter to each of these images
		cvFilter2D(img_blue, img_blue,contrast_enhancement_kernel);
		cvFilter2D(img_green, img_green,contrast_enhancement_kernel);
		cvFilter2D(img_red, img_red,contrast_enhancement_kernel);	

		// combine the transformed gray scale images into the output RGB image
		splitOrMerge(img,IMG_MERGE);
	}

	//cout<<"Done normalizeImageRGB\n";
}
// apply contrast enhancement to the input image
void PreProcessing::performContrastEnhancement(IplImage *img){

	if(params->contrast_enhancement_method==EQUALIZE_HISTOGRAM){

		equalizeHistogramRGB(img);

	} else if(params->contrast_enhancement_method==NORMALIZE_IMAGE){

		normalizeImageRGB(img);
	} else if(params->contrast_enhancement_method==FILTER_IMAGE){

		filterImageRGB(img);
	}

}
// apply noise reduction to the input image
void PreProcessing::performNoiseReduction(IplImage *img){

	if(params->noise_reduction_method==GAUSSIAN_FILTER){

		cvSmooth(img,img,CV_GAUSSIAN,params->kernel_width,params->kernel_height,0,0);

	}else if(params->noise_reduction_method==MEDIAN_FILTER){

		cvSmooth(img,temp_filter_img,CV_MEDIAN,params->kernel_width,params->kernel_height,0,0);
		cvCopy(temp_filter_img,img);

	}else if(params->noise_reduction_method==BILATERAL_FILTER){

		cvSmooth(img,temp_filter_img,CV_BILATERAL,params->kernel_width,params->kernel_height,0,0);
		cvCopy(temp_filter_img,img);

	}else if(params->noise_reduction_method==LINEAR_FILTER){

		cvSmooth(img,img,CV_BLUR,params->kernel_width,params->kernel_height,0,0);

	}
}

void PreProcessing::normalizeIntensityCount(int total_count,int max_count){

	for(int i=0;i<=MAX_INTENSITY;i++){
		intensity_count[i]=(int)((double)intensity_count[i]/(double)total_count*(double)max_count);
		//cout<<"intensity_count["<<i<<"]="<<intensity_count[i]<<"\n";
	}
}


void PreProcessing::drawCountGraph(){	
	cvSetZero(img_hist);
	for(int c=0;c<=MAX_INTENSITY;c++){
		for(int r=0;r<intensity_count[c];r++){
			img_hist->imageData[r*img_hist->widthStep+c]=(unsigned char)MAX_INTENSITY;
		}
	}	
}
// draw the histogram of the the input image after first converting it into a gray scale image if it is not already one
void PreProcessing::getImageHistogram(IplImage *img,CvSize img_size,char *window_name){	

	/*double min_val,max_val;		
	int min_val_int,max_val_int;
	CvPoint min_loc,max_loc;
	*/

	//cout<<"Done initializing intensity_count\n";


	if(img->nChannels>1){		
		//cvSetZero(img_gray);
		cvCvtColor(img,img_gray,CV_BGR2GRAY);
	}else{
		cvCopy(img,img_gray);
	}

	/*cvMinMaxLoc(img_gray,&min_val,&max_val,&min_loc,&max_loc);
	cout<<"min_val="<<min_val<<"\t max_val="<<max_val<<"\n";

	getIntensityRange(img_gray,img_size,min_val_int,max_val_int);
	cout<<"min_val="<<min_val_int<<"\t max_val="<<max_val_int<<"\n";

	cout<<"depth="<<img_gray->depth<<"\n";
	_getch();*/

	//cvShowImage("Image",img_gray);

	int max_count=getIntensityCount(img_gray,img_size);
	//cout<<"Done getIntensityCount\n";

	normalizeIntensityCount(max_count,HIST_HEIGHT);
	//cout<<"Done normalizeIntensityCount\n";

	drawCountGraph();
	//cout<<"Done drawCountGraph\n";

	cvFlip(img_hist);
	cvShowImage(window_name,img_hist);	
}

void PreProcessing::printParames(){

	cout<<"\n";

	cout<<"Pre processing parameters updated:\n";

	cout<<"contrast_enhancement_method="<<params->contrast_enhancement_method<<"\t";
	cout<<"noise_reduction_method="<<params->noise_reduction_method<<"\t";
	cout<<"use_square_kernel="<<params->use_square_kernel<<"\t";
	cout<<"kernel_width="<<params->kernel_width<<"\t";
	cout<<"kernel_height="<<params->kernel_height<<"\t";
	cout<<"cutoff_fraction="<<params->cutoff_fraction<<"\n";

	cout<<"\n";

	//_getch();
}

void PreProcessing::initParams(pre_process_struct *params_init){

	params->noise_reduction_method=params_init->noise_reduction_method;
	params->contrast_enhancement_method=params_init->contrast_enhancement_method;
	params->show_histogram=params_init->show_histogram;
	params->use_square_kernel=params_init->use_square_kernel;

	params->kernel_width_id=params_init->kernel_width_id;
	params->kernel_height_id=params_init->kernel_height_id;	
	params->cutoff_percent=params_init->cutoff_percent;
}

void PreProcessing::updateParams(int){

	params->cutoff_fraction=(double)params->cutoff_percent/100.0;

	params->kernel_width=2*params->kernel_width_id+1;

	if(params->use_square_kernel)
		params->kernel_height=params->kernel_width;	
	else	
		params->kernel_height=2*params->kernel_height_id+1;

	noise_reduction_updated=true;

	printParames();
}

void PreProcessing::updateContrastEnhancementMethod(int){

	cout<<"\n";

	if(params->contrast_enhancement_method==EQUALIZE_HISTOGRAM)	

		cout<<"Using histogram equalization for contrast enhancement.";

	else if(params->contrast_enhancement_method==NORMALIZE_HISTOGRAM)	

		cout<<"Using histogram normalization for contrast enhancement.";

	else if(params->contrast_enhancement_method==NORMALIZE_IMAGE)	

		cout<<"Using image normalization for contrast enhancement.";	
	else
		cout<<"Contrast enhancement turned off.";

	contrast_enhancement_updated=true;

	cout<<"\n";
}

void PreProcessing::updateNoiseReductionMethod(int){

	cout<<"\n";

	if(params->noise_reduction_method==GAUSSIAN_FILTER)	

		cout<<"Using Gaussian filter for noise reduction.";

	else if(params->noise_reduction_method==MEDIAN_FILTER)	

		cout<<"Using median filter for noise reduction.";

	else if(params->noise_reduction_method==BILATERAL_FILTER)	

		cout<<"Using bilateral filter for noise reduction.";

	else if(params->noise_reduction_method==LINEAR_FILTER)	

		cout<<"Using linear filter for noise reduction.";

	else
		cout<<"Noise reduction turned off.";

	noise_reduction_updated=true;

	cout<<"\n";
}

void PreProcessing::showPreProcessHelpWindow(int mouse_event,int x,int y,int flags,void* param){

	if(mouse_event==CV_EVENT_LBUTTONDOWN){
		cvNamedWindow(pre_process_help_window,1);
		cvShowImage(pre_process_help_window,pre_process_help_image);
	}else if(mouse_event==CV_EVENT_RBUTTONDOWN){
		cvDestroyWindow(pre_process_help_window);
	}
}

void PreProcessing::initWindow(char *window_name){

	cvNamedWindow( window_name,1);

	cvCreateTrackbar("contrastM",window_name,&(params->contrast_enhancement_method),4,updateContrastEnhancementMethod);
	cvCreateTrackbar("noiseM",window_name,&(params->noise_reduction_method),4,updateNoiseReductionMethod);	
	cvCreateTrackbar("histogram",window_name,&(params->show_histogram),1,NULL);	

	if(params->noise_reduction_method){
		cvCreateTrackbar("squareKernel",window_name,&(params->use_square_kernel),1,updateParams);
		cvCreateTrackbar("KWidthID",window_name,&(params->kernel_width_id),10,updateParams);
		cvCreateTrackbar("KHeightID",window_name,&(params->kernel_height_id),10,updateParams);
	}
	if(params->contrast_enhancement_method==NORMALIZE_IMAGE){
		cvCreateTrackbar("cutoff%",window_name,&(params->cutoff_percent),100,updateParams);
	}

	cvSetMouseCallback(window_name,showPreProcessHelpWindow,(void*)this);
}




/*
void PreProcessing::updateParams(int val,void *param){

if(!param){
cout<<"Invalid pointer.\n";
_getch();
}

PreProcessing *obj=(PreProcessing*)param;


obj->params->kernel_width=2*obj->params->kernel_width_id+1;

if(obj->params->use_square_kernel)
obj->params->kernel_height=obj->params->kernel_width;	
else	
obj->params->kernel_height=2*obj->params->kernel_height_id+1;

obj->noise_reduction_updated=true;

obj->printParames();
}

void PreProcessing::updateContrastEnhancementMethod(int val,void *param){

cout<<"\n";
if(!param){
cout<<"Invalid pointer.\n";
_getch();
}

PreProcessing *obj=(PreProcessing*)param;

if(obj->params->contrast_enhancement_method==EQUALIZE_HISTOGRAM)	

cout<<"Using histogram equalization for contrast enhancement.";

else if(obj->params->contrast_enhancement_method==NORMALIZE_HISTOGRAM)	

cout<<"Using histogram normalization for contrast enhancement.";

else if(obj->params->contrast_enhancement_method==NORMALIZE_IMAGE)	

cout<<"Using image normalization for contrast enhancement.";	
else
cout<<"Contrast enhancement turned off.";

obj->contrast_enhancement_updated=true;

cout<<"\n";
}

void PreProcessing::updateNoiseReductionMethod(int val,void *param){

cout<<"\n";
if(!param){
cout<<"Invalid pointer.\n";
_getch();
}

PreProcessing *obj=(PreProcessing*)param;

if(obj->params->noise_reduction_method==GAUSSIAN_FILTER)	

cout<<"Using Gaussian filter for noise reduction.";

else if(obj->params->noise_reduction_method==MEDIAN_FILTER)	

cout<<"Using median filter for noise reduction.";

else if(obj->params->noise_reduction_method==BILATERAL_FILTER)	

cout<<"Using bilateral filter for noise reduction.";

else if(obj->params->noise_reduction_method==LINEAR_FILTER)	

cout<<"Using linear filter for noise reduction.";

else
cout<<"Noise reduction turned off.";

obj->noise_reduction_updated=true;

cout<<"\n";
}


void PreProcessing::initWindow(char *window_name){

cvNamedWindow( window_name,1);

createTrackbar("contrastM",window_name,&(params->contrast_enhancement_method),4,updateContrastEnhancementMethod,this);
createTrackbar("noiseM",window_name,&(params->noise_reduction_method),4,updateNoiseReductionMethod,this);	
createTrackbar("squareKernel",window_name,&(params->use_square_kernel),1,updateParams,this);
createTrackbar("KWidthID",window_name,&(params->kernel_width_id),10,updateParams,this);
createTrackbar("KHeightID",window_name,&(params->kernel_height_id),10,updateParams,this);
}
*/
