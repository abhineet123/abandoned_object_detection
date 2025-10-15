#include <windows.h>
#include <iostream>
#include<vector>
#include<stdio.h>

#include <core\core.hpp>
#include<opencv2\opencv.hpp>

#include "systemParameters.hpp"
#include "combineImages.hpp"
#include "preProcessing.hpp"

//#include "connectedComponents.hpp"

#define MAX_FILE_NAME 100
#define MAX_NAME_SIZE 200
#define SHOW_STATUS 0

#define ESC_KEY 27
#define SPACE_BAR_KEY 32
#define BACK_SPACE_KEY 8
#define TAB_KEY 9
#define DEL_KEY 127

using namespace std;
using namespace cv;
using namespace Algorithms::BackgroundSubtraction;

// structure to calculate average and current frame rates
struct FPS{
public:

	FPS();
	void initFPS();
	void updateFPS(int frame_count);

	SYSTEMTIME SystemTime;	
	double start_time,previous_time,current_time;
	double avg_frame_rate,current_frame_rate;
	int show_fps;	
};

FPS::FPS(){
	show_fps=0;

	start_time=previous_time=current_time=0;
	avg_frame_rate=current_frame_rate=0;
}

void FPS::initFPS(){

	GetSystemTime(&SystemTime);
	cout<<"Start time: "<<SystemTime.wHour<<":"<<SystemTime.wMinute<<":"<<SystemTime.wSecond<<"."<<SystemTime.wMilliseconds<<"\n";
	start_time=(SystemTime.wHour*3600.0)+(SystemTime.wMinute*60.0)+SystemTime.wSecond+(SystemTime.wMilliseconds/1000.0);
	current_time=start_time;
}
void FPS::updateFPS(int frame_count){

	GetSystemTime(&SystemTime);
	//cout<<"Current time: "<<SystemTime.wHour<<":"<<SystemTime.wMinute<<":"<<SystemTime.wSecond<<"."<<SystemTime.wMilliseconds<<"\n";
	previous_time=current_time;
	current_time=(SystemTime.wHour*3600.0)+(SystemTime.wMinute*60.0)+SystemTime.wSecond+(SystemTime.wMilliseconds/1000.0);
	avg_frame_rate=(double)(frame_count+1)/(current_time-start_time);
	current_frame_rate=1.0/(current_time-previous_time);
	if(show_fps){
		cout<<"fps: average: "<<avg_frame_rate<<" current: "<<current_frame_rate<<"\n";
	}
}

struct window_name_struct{

	char *io;	
	char *proc;
	char *pre_processing;
	char *bgs;
	char *frg_analysis;	
	char *morph_analysis;
	char *blob_detection;
	char *blob_tracking;
	char *blob_matching;
	char *abandonment_analysis;
	char *object_selection;
	char *blob_filtering;

	void initWindowNames();
	void clearWindowNames();
};

window_name_struct *window_names=NULL;

PreProcessing *pre_process_obj=NULL;
BgsModels *bgs_obj=NULL;
ForegroundProc *frg_proc_obj=NULL;
BlobDetection *detection_obj=NULL;
BlobTracking *tracking_obj=NULL;
AbandonmentAnalysis *abandonment_obj=NULL;
BlobFilter *filter_obj=NULL;

//miscellaneous functions that cannot be included in any specific module. Their definition follows that of the main function.
void removeAllObjects(UserInterface *ui_obj,int object_type);
void selectAndRemoveObject(UserInterface *ui_obj,char *selection_window_name,int object_type);
void selectAndRemoveCandidateObject(UserInterface *ui_obj,char *selection_window_name);
void removeMarkedObjects(UserInterface *ui_obj);

void initSystemObjects(UserInterface *ui_obj,SystemParameters *param_obj,int pre_process_flag,int filter_flag);
void initStateVariables(UserInterface *ui_obj,int pre_process_flag,int filter_flag);
void destroySystemObjects();

double getForegroundRatio(IplImage *img);
void selectAndPushObjectToBackground(UserInterface *ui_obj);
inline void processIOChanges(UserInterface *ui_obj,int &frame_count,int min_blob_size_full_res);

// initialize the titles of the many windows used to display outputs to the user
void window_name_struct::initWindowNames(){

	io=new char[MAX_NAME_SIZE];
	sprintf(io,"Current Frame");

	proc=new char[MAX_NAME_SIZE];
	sprintf(proc,"Combined");	

	pre_processing=new char[MAX_NAME_SIZE];
	sprintf(pre_processing,"Pre processing Parameters");

	bgs=new char[MAX_NAME_SIZE];
	sprintf(bgs,"BGS parameters");	

	frg_analysis=new char[MAX_NAME_SIZE];	
	sprintf(frg_analysis,"Foreground Analysis Parameters");	

	morph_analysis=new char[MAX_NAME_SIZE];
	sprintf(morph_analysis,"Foreground Processing Toggles");	

	blob_detection=new char[MAX_NAME_SIZE];
	sprintf(blob_detection,"Blobs");

	blob_tracking=new char[MAX_NAME_SIZE];
	sprintf(blob_tracking,"Blob Tracking Parameters");	

	blob_matching=new char[MAX_NAME_SIZE];
	sprintf(blob_matching,"Blob Matching Parameters");

	abandonment_analysis=new char[MAX_NAME_SIZE];
	sprintf(abandonment_analysis,"Abandonment Analysis Parameters");

	object_selection=new char[MAX_NAME_SIZE];
	sprintf(object_selection,"Left click inside an object to select it");

	blob_filtering=new char[MAX_NAME_SIZE];
	sprintf(blob_filtering,"Blob Filtering Parameters");
}

void window_name_struct::clearWindowNames(){

	delete(io);
	delete(proc);
	delete(pre_processing);
	delete(bgs);
	delete(frg_analysis);
	delete(morph_analysis);
	delete(blob_detection);
	delete(blob_tracking);
	delete(blob_matching);
	delete(abandonment_analysis);
	delete(object_selection);
	delete(blob_filtering);	
}

//obsolete stuff not being used any more
double static_threshold;
int static_threshold_percent;
void updateParamsStatic(int) {
	static_threshold=(double)(static_threshold_percent)/100.0;
}

//function and variables to provide for the keyboard based input options
bool skip_frames=false;
bool pause_frames=false;
bool show_help=false;
bool rewind_frames=false;

inline void waitForKeyPress(int wait_timeout,UserInterface *ui_obj){
	unsigned char pressed_key=cvWaitKey(wait_timeout);

	if(pressed_key=='x'){
		selectAndRemoveObject(ui_obj,window_names->object_selection,OBJ_ALL);			
	}else if(pressed_key=='X'){
		removeAllObjects(ui_obj,OBJ_ALL);
		//tracking_obj->eliminateStaticBlobs();
	}else if(pressed_key=='s'){
		selectAndRemoveObject(ui_obj,window_names->object_selection,OBJ_STATIC);			
	}else if(pressed_key=='S'){
		removeAllObjects(ui_obj,OBJ_STATIC);
		//tracking_obj->eliminateStaticBlobs();
	}if(pressed_key=='a'){
		selectAndRemoveObject(ui_obj,window_names->object_selection,OBJ_ABANDONED);			
	}else if(pressed_key=='A'){
		removeAllObjects(ui_obj,OBJ_ABANDONED);
		//tracking_obj->eliminateStaticBlobs();
	}else if(pressed_key=='r'){
		selectAndRemoveObject(ui_obj,window_names->object_selection,OBJ_REMOVED);			
	}else if(pressed_key=='R'){
		removeAllObjects(ui_obj,OBJ_REMOVED);			
	}else if(pressed_key=='f'){
		filter_obj->addCandidateRemovedObjects(ui_obj->frame_data_original);
	}else if(pressed_key=='F'){
		cout<<"\n";
		cout<<"Clearing all objects and images for stored for filtering...\n";
		filter_obj->initStateVariables(ui_obj->original_size);
	}else if(pressed_key=='C'){
		selectAndRemoveCandidateObject(ui_obj,window_names->object_selection);
	}else if(pressed_key=='b'){		
		selectAndPushObjectToBackground(ui_obj);
		cout<<"\n";
		cout<<"Resetting the background model for selected objects...\n";
	}else if(pressed_key=='B'){
		cout<<"\n";
		cout<<"Resetting the background model...\n";
		bgs_obj->bgs->resetBackground(*(ui_obj->frame_data_resized),NULL);
	}else if(pressed_key=='I'){
		cout<<"\n";
		cout<<"Resetting the saved images of all tracked blobs...\n";
		tracking_obj->refreshTrackedBlobsImages(ui_obj->frame_data_resized->Ptr(),frg_proc_obj->frg_grad->grad_x.Ptr(),frg_proc_obj->frg_grad->grad_y.Ptr());
	}else if(pressed_key==SPACE_BAR_KEY){
		skip_frames=!skip_frames;
		cout<<"\n";
		if(skip_frames)
			cout<<"Skipping frame processing.";
		else
			cout<<"Continuing frame processing.";
		cout<<"\n";
	}else if((pressed_key=='p')&&(!rewind_frames)){
		pause_frames=!pause_frames;
		cout<<"\n";
		if(pause_frames)
			cout<<"Pausing frame processing.";
		else
			cout<<"Continuing frame processing.";
		cout<<"\n";
	}else if((pressed_key==BACK_SPACE_KEY)&&(!pause_frames)){
		rewind_frames=!rewind_frames;
		cout<<"\n";
		if(rewind_frames)
			cout<<"Rewinding frames.";
		else
			cout<<"Continuing frames.";
		cout<<"\n";
	}else if(pressed_key==TAB_KEY){
		show_help=!show_help;
	}
	else if(pressed_key==ESC_KEY){
		ui_obj->io_params->exit_program=1;
	}
}

/**********************************************************The main function***************************************************************************/


int main(int argc, const char* argv[]) {

	/*char c;
	IplImage* dst;
	int frame_interval=10;*/	
	//char *ref_bkg_file_name;	

	// setup buffer to hold individual frames from video stream
	IplImage *frame_data;		

	//-----------------------------------------setup track bars---------------------------------------------------------//

	//obsolete code not used anymore
	if(SHOW_STATUS)
		cout<<"Starting track bar setup.\n";		

	static_threshold_percent=65;
	static_threshold=(double)(static_threshold_percent)/100.0;
	/*cvNamedWindow( "Static Mask", 1 );
	cvCreateTrackbar("Static Threshold %","Static Mask",&static_threshold_percent,100,updateParamsStatic);*/	

	if(SHOW_STATUS)
		cout<<"Done track bar setup.\n";		

	//-------------------------------------------------------------------------------------------------------------------------------//

	window_names=new window_name_struct;
	window_names->initWindowNames();

	if(SHOW_STATUS)
		cout<<"Done with window creation\n";

	SystemParameters *param_obj=new SystemParameters();
	char param_file_name[]="init_parameters.txt";
	// reading initial parameter values from text file
	param_obj->readInitialParams(param_file_name);

	UserInterface *ui_obj=new UserInterface(param_obj->ui_io_params_init,param_obj->ui_proc_params_init);
	ui_obj->initIOWindow(window_names->io);
	ui_obj->initProcWindow(window_names->proc);

	if(SHOW_STATUS)
		cout<<"Done with ui object creation\n";	

	// initializing the various system modules
	initSystemObjects(ui_obj,param_obj,1,1);

	if(SHOW_STATUS)
		cout<<"Done with system object creation\n";	

	// backup images to be displayed when no useful information is being shown in the respective windows.

	/*IplImage *frg_img=cvLoadImage("frg_proc_params.jpg");
	//cvShowImage(window_names->frg_analysis,frg_img);

	IplImage *blob_tracking_img=cvLoadImage("blob_tracking_params.jpg");
	//cvShowImage(window_names->blob_tracking,blob_tracking_img);

	IplImage *abandonment_img=cvLoadImage("abandonment_analysis_params.jpg");
	//cvShowImage(window_names->abandonment_analysis,abandonment_img);*/

	IplImage *pre_processing_disabled=cvLoadImage("pre_processing_disabled.jpg");

	//image showing the keyboard inputs accepted by the program.
	IplImage *key_image=cvLoadImage("help/keys.jpg");

	//-------------------------------------------------------------------------------------------------------------------------------//

	int min_blob_size_full_res=detection_obj->params->min_blob_size;
	double resize_factor=ui_obj->proc_params->proc_resize_factor;

	detection_obj->params->min_blob_size=(int)((double)min_blob_size_full_res/(resize_factor*resize_factor));

	int line_thickness=1;
	int line_type=8;
	int shift=0;	

	CvScalar outline_color=CV_RGB(0,255,0);	
	CvScalar frg_color=CV_RGB(0,255,0); 
	CvScalar track_color=CV_RGB(255,0,0);

	RgbImage *bkg_img;		

	int show_fps=0;	
	int training_period=0;
	int frame_count=-1;
	
	if(training_period>0){
		cout<<"Training Background for "<<training_period<<" seconds.....\n";
	}	

	FPS *fps_obj=new FPS;	

	char *fps_text=new char[100];
	CvFont fps_font;
	cvInitFont(&fps_font, CV_FONT_HERSHEY_COMPLEX, 0.5, 0.5, 0, 1, 8);
	int fps_x=5,fps_y=15;
	CvScalar fps_col= CV_RGB(0,255,0);

	char *orig_hist=new char[MAX_NAME_SIZE];
	sprintf(orig_hist,"Original Histogram");

	char *eq_hist=new char[MAX_NAME_SIZE];
	sprintf(eq_hist,"Processed Histogram");	

	IplImage *tracked_mask_img=NULL;	
	BwImage *temp_tracked_mask=NULL;

	int frg_thr_count=0; 
	int histogram_visible=0;

	int buffer_actual_index;

	//cout<<"Starting the main loop.\n";	

	while(!ui_obj->io_params->exit_program) {		

		//---------------------- acquiring and resizing the next frame-------------------------------//
		if(rewind_frames){
			if(ui_obj->buffer_current_index>ui_obj->buffer_start_index){
				ui_obj->buffer_current_index--;	
				buffer_actual_index=(ui_obj->buffer_current_index+ui_obj->buffer_start_index)%REWIND_BUFFER_SIZE;
				cvShowImage(window_names->io,ui_obj->rewind_buffer[buffer_actual_index]);							
				waitForKeyPress(1,ui_obj);
				continue;
			}else{
				rewind_frames=false;
			}
		}		

		if(!pause_frames){

			processIOChanges(ui_obj,frame_count,min_blob_size_full_res);
			frame_count++;	

			if(frame_count==0){
				fps_obj->initFPS();			
			}

			/*if(i % 10 == 0)
			cout << "Processing frame " << i << " of " << num_frames << "...\n" ;*/	
			// grab next frame from input video stream
			/*if(!cvGrabFrame(video_reader)) {
			std::cerr << "Could not grab AVI frame." << std::endl;
			return 0;
			}*/
			//frame_data = cvRetrieveFrame(video_reader);

			if(SHOW_STATUS)
				cout<<"Just before getting a frame\n";

			if((ui_obj->buffer_end_index>0)&&(ui_obj->buffer_current_index<ui_obj->buffer_end_index)){
				//getting next frame from the rewind buffer
				ui_obj->buffer_current_index++;
				buffer_actual_index=(ui_obj->buffer_current_index+ui_obj->buffer_start_index)%REWIND_BUFFER_SIZE;
				cvCopy(ui_obj->rewind_buffer[buffer_actual_index],frame_data);			
			}else{
				//getting next frame from the input video stream
				if (!(frame_data=cvQueryFrame(ui_obj->video_reader))) {
					if(ui_obj->io_params->capture_from_camera)
						cout<<"\nUnable to detect the camera.\n";
					else
						cout<<"\nReached the end of the input video file.\n";			
					break;
				}
				if(ui_obj->buffer_end_index==REWIND_BUFFER_SIZE-1){
					ui_obj->buffer_start_index=(ui_obj->buffer_start_index+1)%REWIND_BUFFER_SIZE;
				}else{
					ui_obj->buffer_current_index++;
					ui_obj->buffer_end_index++;
				}
				buffer_actual_index=(ui_obj->buffer_end_index+ui_obj->buffer_start_index)%REWIND_BUFFER_SIZE;
				cvCopy(frame_data,ui_obj->rewind_buffer[buffer_actual_index]);
			}
		}else if(frame_count<0){
			waitForKeyPress(1,ui_obj);
			continue;
		}	

		if(frame_count<=ui_obj->skipped_secs*ui_obj->io_params->fps)
			continue;	

		//saving a copy of the original frame before writing frame rates to it.
		cvCopy(frame_data,ui_obj->frame_data_original);

		if(ui_obj->proc_params->change_proc_resolution){
			cvResize(frame_data,ui_obj->frame_data_resized->Ptr());
		}else{
			cvCopy(frame_data,ui_obj->frame_data_resized->Ptr());
			//ui_obj->frame_data_resized=&frame_data;
		}
		//---------------------- done acquiring and resizing the next frame-------------------------------//

		if(!pause_frames){
			sprintf(fps_text,"%5.2f %5.2f",fps_obj->avg_frame_rate,fps_obj->current_frame_rate);
			cvPutText(frame_data,fps_text,cvPoint(fps_x,fps_y), &fps_font,fps_col);
		}
		cvShowImage(window_names->io,frame_data);

		if(SHOW_STATUS)
			cout<<"Done resizing\n";

		if(skip_frames){			
			waitForKeyPress(1,ui_obj);
			fps_obj->updateFPS(frame_count);
			continue;
		}		

		//-------------------------------preprocessing the frame-----------------------------------------//

		if(pre_process_obj->params->show_histogram){
			histogram_visible=1;
			pre_process_obj->getImageHistogram(ui_obj->frame_data_resized->Ptr(),ui_obj->proc_size,orig_hist);
		}else if(histogram_visible){			
			cvDestroyWindow(orig_hist);
		}

		if(pre_process_obj->params->contrast_enhancement_method>0){
			pre_process_obj->performContrastEnhancement(ui_obj->frame_data_resized->Ptr());
			if(SHOW_STATUS)
				cout<<"Done contrast enhancement\n";
		}

		if(pre_process_obj->params->show_histogram){
			histogram_visible=1;
			pre_process_obj->getImageHistogram(ui_obj->frame_data_resized->Ptr(),ui_obj->proc_size,eq_hist);
		}else if(histogram_visible){
			cvDestroyWindow(eq_hist);
		}
		if(histogram_visible&&!pre_process_obj->params->show_histogram){
			histogram_visible=0;
		}

		if(pre_process_obj->params->noise_reduction_method>0){
			pre_process_obj->performNoiseReduction(ui_obj->frame_data_resized->Ptr());
			if(SHOW_STATUS)
				cout<<"Done noise reduction\n";
		}
		if((pre_process_obj->params->contrast_enhancement_method>0)||(pre_process_obj->params->noise_reduction_method>0)){

			if((ui_obj->output_size.width!=ui_obj->proc_size.width)||(ui_obj->output_size.height!=ui_obj->proc_size.height)){
				cvResize(ui_obj->frame_data_resized->Ptr(),ui_obj->output_img_rgb);
			}else{
				cvCopy(ui_obj->frame_data_resized->Ptr(),ui_obj->output_img_rgb);
				//ui_obj->output_img_rgb=bkg_img->Ptr();
			}			
			cvShowImage(window_names->pre_processing,ui_obj->output_img_rgb);
		}else{
			cvShowImage(window_names->pre_processing,pre_processing_disabled);
		}	

		if((pre_process_obj->noise_reduction_updated)||(pre_process_obj->contrast_enhancement_updated)){
			bgs_obj->bgs->resetBackground(*(ui_obj->frame_data_resized),NULL);
			if(SHOW_STATUS)
				cout<<"Done resetBackground\n";
			cvDestroyWindow(window_names->pre_processing);
			pre_process_obj->initWindow(window_names->pre_processing);
			if(pre_process_obj->noise_reduction_updated){				
				pre_process_obj->noise_reduction_updated=false;
			}
			if(pre_process_obj->contrast_enhancement_updated){
				pre_process_obj->contrast_enhancement_updated=false;
			}			
		}
		//-------------------------------done preprocessing the frame-----------------------------------------//


		//-------------------------------performing background subtraction-----------------------------------------//

		// initialize background model to the first frame of the video stream
		if ((ui_obj->input_source_changed)||(frame_count==ui_obj->skipped_secs*ui_obj->io_params->fps+1)){
			ui_obj->input_source_changed=false;
			bgs_obj->bgs->InitModel(*(ui_obj->frame_data_resized));	
			if(SHOW_STATUS)
				cout<<"Done InitModel\n";
		}

		if(SHOW_STATUS)
			cout<<"Starting background subtraction\t frame_count="<<frame_count<<"\tskipped_count="<<ui_obj->skipped_secs*ui_obj->io_params->fps<<"\n";
		
		if((bgs_obj->bgs_params->bgs_toggle_params->bgs_method==BGS_ADAPTIVE_MEDIAN)||(bgs_obj->bgs_params->bgs_toggle_params->bgs_method==BGS_RUNNING_GAUSSIAN)){

			bgs_obj->bgs->Subtract(*(ui_obj->frame_data_resized),*(ui_obj->low_threshold_mask),*(ui_obj->high_threshold_mask));
			bgs_obj->bgs->Update(*(ui_obj->frame_data_resized),tracking_obj->tracked_mask);

			if((bgs_obj->bgs_params->bgs_toggle_params->bgs_method==BGS_ADAPTIVE_MEDIAN)&&(frame_count<=bgs_obj->median_params->LearningFrames())){					
				continue;
			}
			if((bgs_obj->bgs_params->bgs_toggle_params->bgs_method==BGS_RUNNING_GAUSSIAN)&&(frame_count<=bgs_obj->rg_params->LearningFrames())){					
				continue;
			}

		}else if(bgs_obj->bgs_params->bgs_toggle_params->bgs_method==BGS_ZIVKOVIC_GMM){	

			bgs_obj->bgs->Subtract2(*(ui_obj->frame_data_resized),*(ui_obj->low_threshold_mask),*(ui_obj->high_threshold_mask),tracking_obj->tracked_mask);

		}else if(bgs_obj->bgs_params->bgs_toggle_params->bgs_method==BGS_GRIMSON_GMM){

			bgs_obj->bgs->Subtract2(*(ui_obj->frame_data_resized), *(ui_obj->low_threshold_mask),*(ui_obj->high_threshold_mask),
				*(ui_obj->static_threshold_mask),tracking_obj->tracked_mask,static_threshold);

		} 
		if(SHOW_STATUS)
			cout<<"Done background subtraction\n";

		if(frame_count<training_period*ui_obj->io_params->fps){
			//cout<<"Training Background.....\n";				
			continue;
		}
		// looking for a scene wide sudden lighting change	
		double frg_ratio=getForegroundRatio(ui_obj->low_threshold_mask->Ptr());
		//cout<<"frg_ratio="<<frg_ratio<<"\n";
		if((!ui_obj->input_source_changed)&&(frg_ratio>bgs_obj->bgs_params->bgs_toggle_params->frg_thr)){
			if(frg_thr_count==0){					
				if(temp_tracked_mask){
					delete(temp_tracked_mask);
					temp_tracked_mask=NULL;
				}						
				tracked_mask_img=cvCreateImage(ui_obj->proc_size,IPL_DEPTH_8U,1);

				/*cout<<"About to copy inside frg_thr_count.\n";	
				cout<<"proc_size.width="<<ui_obj->proc_size.width<<"\t proc_size.height="<<ui_obj->proc_size.height<<"\n";
				cout<<"tracked_mask_img.width="<<tracked_mask_img->width<<"\t tracked_mask_img.height="<<tracked_mask_img->height<<"\n";
				cout<<"static_mask.width="<<tracking_obj->static_mask.Ptr()->width<<"\t static_mask.height="<<tracking_obj->static_mask.Ptr()->height<<"\n";*/

				cvCopy(tracking_obj->static_mask.Ptr(),tracked_mask_img);
				cout<<"Done copying.\n";
				temp_tracked_mask=new BwImage(tracked_mask_img);
			}
			frg_thr_count++;
			if(frg_thr_count>=bgs_obj->bgs_params->bgs_toggle_params->max_frg_thr_count){
				cout<<"Foreground ratio has crossed the threshold at "<<frg_ratio<<".\n";
				cout<<"Resetting the background.....\n";
				//_getch();
				bgs_obj->bgs->resetBackground(*(ui_obj->frame_data_resized),temp_tracked_mask);				
				frg_thr_count=0;
			}
		}else{
			frg_thr_count=0;
		}

		bkg_img=bgs_obj->bgs->Background();

		if((ui_obj->output_size.width!=ui_obj->proc_size.width)||(ui_obj->output_size.height!=ui_obj->proc_size.height)){
			cvResize(bkg_img->Ptr(),ui_obj->output_img_rgb);
		}else{			
			cvCopy(bkg_img->Ptr(),ui_obj->output_img_rgb);
			//ui_obj->output_img_rgb=bkg_img->Ptr();
		}
		cvShowImage(window_names->bgs,ui_obj->output_img_rgb);
		//cvWriteFrame(ui_obj->bkg_video_writer,ui_obj->output_img_rgb);				

		//cvCopy(low_threshold_mask.Ptr(),temp_gray_img);

		//cvShowImage( "Unprocessed Mask", low_threshold_mask.Ptr() );
		//cvWriteFrame(unproc_frg_mask_video_writer, low_threshold_mask.Ptr());

		covertGrayscaleToRGB(ui_obj->low_threshold_mask->Ptr(),ui_obj->proc_img_rgb);
		//cvShowImage( "Static Mask", static_threshold_mask.Ptr());

		//------------------------------- done performing background subtraction-----------------------------------------//


		//-----------------------------------performing foreground analysis---------------------------------------------//

		if(SHOW_STATUS)
			cout<<"Starting foreground_analysis\n";

		if(frg_proc_obj->params->perform_foreground_analysis){

			frg_proc_obj->removeFalseForeground(bkg_img->Ptr(),ui_obj->frame_data_resized->Ptr(),*(ui_obj->low_threshold_mask));

			/*combineImages(temp_gray_img,low_threshold_mask.Ptr(),combined_image_x2_gray,HORIZONTAL_JOIN);
			//cvShowImage("Processed Mask",low_threshold_mask.Ptr());	
			cvShowImage("Foreground Masks",combined_image_x2_gray);
			cvWriteFrame(frg_mask_video_writer,combined_image_x2_gray);
			*/

			if((ui_obj->output_size.width!=ui_obj->proc_size.width)||(ui_obj->output_size.height!=ui_obj->proc_size.height)){
				cvResize(ui_obj->low_threshold_mask->Ptr(),ui_obj->output_img_gray);
			}else{
				cvCopy(ui_obj->low_threshold_mask->Ptr(),ui_obj->output_img_gray);
				//ui_obj->output_img_gray=ui_obj->low_threshold_mask->Ptr();
			}
			cvShowImage(window_names->frg_analysis,ui_obj->output_img_gray);			

			/*cvShowImage( "frg_grad_x", frg_proc_obj->frg_grad->grad_x.Ptr() );
			cvShowImage( "bkg_grad_x", frg_proc_obj->bkg_grad->grad_x.Ptr() );
			cvShowImage( "bkg_gray", frg_proc_obj->bkg_gray.Ptr() );
			cvShowImage( "frg_gray", frg_proc_obj->frg_gray.Ptr() );*/
		}	
		if(SHOW_STATUS)
			cout<<"Done foreground_analysis\n";

		//getImageHistogram(frg_proc_obj->frg_grad->grad_x.Ptr(),ui_obj->proc_size,255);
		//getImageHistogram(frg_proc_obj->frg_grad->grad_y.Ptr(),ui_obj->proc_size,255);

		//cvShowImage("Before Morph",ui_obj->low_threshold_mask->Ptr());


		if(frg_proc_obj->morph_params->performMorphologicalOp){

			if(frg_proc_obj->morph_params->performOpening){
				//morphological opening operation to reduce foreground fragments and noise
				cvErode(ui_obj->low_threshold_mask->Ptr(),ui_obj->low_threshold_mask->Ptr(),NULL,frg_proc_obj->morph_params->no_of_iterations);
				cvDilate(ui_obj->low_threshold_mask->Ptr(),ui_obj->low_threshold_mask->Ptr(),NULL,frg_proc_obj->morph_params->no_of_iterations);					
			}
			if(frg_proc_obj->morph_params->performClosing){
				//morphological closing operation to reduce foreground fragments and noise
				cvDilate(ui_obj->low_threshold_mask->Ptr(),ui_obj->low_threshold_mask->Ptr(),NULL,frg_proc_obj->morph_params->no_of_iterations);
				cvErode(ui_obj->low_threshold_mask->Ptr(),ui_obj->low_threshold_mask->Ptr(),NULL,frg_proc_obj->morph_params->no_of_iterations);				
			}
		}
		if(SHOW_STATUS)
			cout<<"Done with morphological operations.\n";
		//cvShowImage("After Morph",ui_obj->low_threshold_mask->Ptr());

		//---------------------------------done performing foreground analysis-------------------------------------------//

		/*waitForKeyPress(1,ui_obj);
		continue;*/

		if((ui_obj->output_size.width!=ui_obj->proc_size.width)||(ui_obj->output_size.height!=ui_obj->proc_size.height)){
			cvResize(ui_obj->low_threshold_mask->Ptr(),ui_obj->output_img_gray);
		}else{
			cvCopy(ui_obj->low_threshold_mask->Ptr(),ui_obj->output_img_gray);
			//ui_obj->output_img_gray=ui_obj->low_threshold_mask->Ptr();

		}
		cvShowImage(window_names->morph_analysis,ui_obj->output_img_gray);
		//cvWriteFrame(ui_obj->proc_frg_mask_video_writer,ui_obj->output_img_gray);

		if(SHOW_STATUS)
			cout<<"About to convert GS to RGB.\n";
		covertGrayscaleToRGB(ui_obj->output_img_gray,ui_obj->output_img_rgb_2);
		if(SHOW_STATUS)
			cout<<"Done converting GS to RGB.\n";

		if(SHOW_STATUS)
			cout<<"About to combine images.\n";
		combineImages(ui_obj->output_img_rgb,ui_obj->output_img_rgb_2,ui_obj->combined_image_x2_2,HORIZONTAL_JOIN);
		if(SHOW_STATUS)
			cout<<"Done combining images.\n";

		/*if(!CV_IS_IMAGE(ui_obj->low_threshold_mask->Ptr()))
		cout<<"\n Not an image !\n";
		cvShowImage("Nazi",ui_obj->low_threshold_mask->Ptr());
		cvWaitKey(0);*/
		//-----------------------------------performing blob extraction---------------------------------------------//
		detection_obj->getBlobs(ui_obj->low_threshold_mask->Ptr(), ui_obj->frame_data_resized->Ptr(),ui_obj->io_params->show_all_blobs);

		if(SHOW_STATUS)
			cout<<"Done with blob detection.\n";

		if((ui_obj->output_size.width!=ui_obj->proc_size.width)||(ui_obj->output_size.height!=ui_obj->proc_size.height)){
			cvResize(detection_obj->blob_image,ui_obj->output_img_rgb);
		}else{
			cvCopy(detection_obj->blob_image,ui_obj->output_img_rgb);
			//ui_obj->output_img_rgb=detection_obj->blob_image;
		}
		cvShowImage(window_names->blob_detection,ui_obj->output_img_rgb);

		/*waitForKeyPress(1,ui_obj);
		continue;*/
		//---------------------------------done performing blob extraction------------------------------------------//


		//-----------------------------------performing blob tracking---------------------------------------------//

		//tracking_obj->updateBlobDistances(detection_obj->blobs,BOUNDING_BOX_DIST);

		IplImage *frg_grad_x=frg_proc_obj->frg_grad->grad_x.Ptr();
		IplImage *frg_grad_y=frg_proc_obj->frg_grad->grad_y.Ptr();

		/*cvShowImage("grad_x",frg_grad_x);
		cvShowImage("grad_y",frg_grad_y);
		cvWaitKey(0);*/

		if(!pause_frames){
			if(tracking_obj->matching_params->use_gradient_diff){
				tracking_obj->updateTrackedBlobs(ui_obj->frame_data_resized->Ptr(),detection_obj->blobs,detection_obj->no_of_blobs,*(ui_obj->low_threshold_mask),frg_grad_x,frg_grad_y);
			}else{
				tracking_obj->updateTrackedBlobs(ui_obj->frame_data_resized->Ptr(),detection_obj->blobs,detection_obj->no_of_blobs,*(ui_obj->low_threshold_mask));
			}
			if(SHOW_STATUS)
				cout<<"Done with updateTrackedBlobs.\n";

			/*waitForKeyPress(1,ui_obj);
			continue;*/

			//tracking_obj->displayBlobsAndDistances(frame_data,detection_obj->blobs,frg_color,track_color);
			//Stracking_obj->showCorrespondenceImage(detection_obj->blobs,line_thickness,line_type,shift);

			bool found_ack=false;
			if(tracking_obj->matching_params->use_gradient_diff){
				found_ack=tracking_obj->processTrackedBlobs(ui_obj->frame_data_resized->Ptr(),detection_obj->blobs,detection_obj->no_of_blobs,frg_grad_x,frg_grad_y);	
			}else{
				found_ack=tracking_obj->processTrackedBlobs(ui_obj->frame_data_resized->Ptr(),detection_obj->blobs,detection_obj->no_of_blobs);
			}
			if(SHOW_STATUS)
				cout<<"Done with processTrackedBlobs.\n";

			/*waitForKeyPress(1,ui_obj);
			continue;*/

			if(found_ack)
				removeMarkedObjects(ui_obj);

			tracking_obj->updateTrackedStaticMask();
			if(SHOW_STATUS)
				cout<<"Done with updateTrackedStaticMask.\n";
			if((ui_obj->output_size.width!=ui_obj->proc_size.width)||(ui_obj->output_size.height!=ui_obj->proc_size.height)){
				cvResize(tracking_obj->tracked_mask.Ptr(),ui_obj->output_img_gray);
			}else{
				cvCopy(tracking_obj->tracked_mask.Ptr(),ui_obj->output_img_gray);
				//ui_obj->output_img_gray=tracking_obj->tracked_mask.Ptr();
			}
			cvShowImage(window_names->blob_matching,ui_obj->output_img_gray);
			cvWriteFrame(ui_obj->static_mask_video_writer,ui_obj->output_img_gray);
		}
		/*if(found_abandoned){
		tracking_obj->showAbandonedBlobs(detection_obj->bounding_box_image,CV_RGB(255,255,255));
		}*/

		/*cvResize( detection_obj->bounding_box_image,resized_image_rgb);
		cvShowImage("Bounding Boxes Resized", resized_image_rgb);

		temp_img=combineImages(detection_obj->bounding_box_image,bkg_img->Ptr(),HORIZONTAL_JOIN);
		cvShowImage( "Combined Image", temp_img);*/

		//cvWaitKey(0);	
		//cout<<"\n\n"<<abandonment_obj->params->abandonment_method<<"\n\n";

		//-----------------------------------done performing blob tracking---------------------------------------------//


		//-----------------------------------performing abandonment analysis---------------------------------------------//

		bool abnd_status=false;
		if(abandonment_obj->params->abandonment_method==ABND_REGION_GROWING){

			/*abandonment_obj->showBoxes(tracking_obj->tracked_blobs,line_thickness,line_type,shift);
			cvResize(abandonment_obj->temp_rgb_img.Ptr(),ui_obj->output_img_rgb);
			cvShowImage(window_names->abandonment_analysis,ui_obj->output_img_rgb);
			cvWaitKey(0);*/

			//abandonment_obj->drawErodedContour2(tracking_obj->tracked_blobs);

			abnd_status=abandonment_obj->detectRemovedObjectsUsingRegionGrowing(tracking_obj->tracked_blobs,*(ui_obj->frame_data_resized),*bkg_img,ui_obj->proc_size);
			//cvResize(abandonment_obj->boundary_image,ui_obj->output_img_rgb);
			cvResize(abandonment_obj->region_growing_frg,ui_obj->output_img_gray);			
			covertGrayscaleToRGB(ui_obj->output_img_gray,ui_obj->output_img_rgb);
			cvShowImage(window_names->abandonment_analysis,ui_obj->output_img_rgb);

			cvResize(abandonment_obj->region_growing_bkg,ui_obj->output_img_gray);
			covertGrayscaleToRGB(ui_obj->output_img_gray,ui_obj->output_img_rgb_2);

			combineImages(ui_obj->output_img_rgb,ui_obj->output_img_rgb_2,ui_obj->combined_image_x2_3,VERTICAL_JOIN);

		}else if(abandonment_obj->params->abandonment_method==ABND_GRADIENT_EDGE_DETECTION){

			abnd_status=abandonment_obj->detectRemovedObjectsUsingGradientEdgeDetection(tracking_obj->tracked_blobs,frg_proc_obj->frg_grad,frg_proc_obj->bkg_grad);
			cvResize(abandonment_obj->frg_grad_combined,ui_obj->output_img_gray);
			cvShowImage(window_names->abandonment_analysis,ui_obj->output_img_gray);
			covertGrayscaleToRGB(ui_obj->output_img_gray,ui_obj->output_img_rgb);

			cvResize(abandonment_obj->bkg_grad_combined,ui_obj->output_img_gray);
			covertGrayscaleToRGB(ui_obj->output_img_gray,ui_obj->output_img_rgb_2);

			combineImages(ui_obj->output_img_rgb,ui_obj->output_img_rgb_2,ui_obj->combined_image_x2_3,VERTICAL_JOIN);

		}else if(abandonment_obj->params->abandonment_method==ABND_CANNY_EDGE_DETECTION){			

			abnd_status=abandonment_obj->detectRemovedObjectsUsingCannyEdgeDetection(tracking_obj->tracked_blobs,frg_proc_obj->frg_gray.Ptr(),frg_proc_obj->bkg_gray.Ptr());			
			cvResize(abandonment_obj->canny_frg,ui_obj->output_img_gray);
			cvShowImage(window_names->abandonment_analysis,ui_obj->output_img_gray);
			covertGrayscaleToRGB(ui_obj->output_img_gray,ui_obj->output_img_rgb);

			cvResize(abandonment_obj->canny_bkg,ui_obj->output_img_gray);
			covertGrayscaleToRGB(ui_obj->output_img_gray,ui_obj->output_img_rgb_2);

			combineImages(ui_obj->output_img_rgb,ui_obj->output_img_rgb_2,ui_obj->combined_image_x2_3,VERTICAL_JOIN);	
		}

		if(abnd_status)
			removeMarkedObjects(ui_obj);

		//abandonment_obj->showGradientImagesForAbandonedBlobs(tracking_obj->tracked_blobs,frg_proc_obj,ui_obj->proc_img_rgb,ui_obj->proc_img_gray);
		//abandonment_obj->clearContourImage();

		//abandonment_obj->drawOriginalContour(&(detection_obj->blobs),outline_color);
		//abandonment_obj->drawErodedContour(&(detection_obj->blobs),CV_RGB(255,0,0),erosion_width);

		/*abandonment_obj->testRegionGrowing(*(ui_obj->frame_data_resized),ui_obj->proc_size,&(detection_obj->blobs));
		cvShowImage( "Blob Outlines Foreground", abandonment_obj->contour_image.Ptr());				

		abandonment_obj->testRegionGrowing(*bkg_img,ui_obj->proc_size,&(detection_obj->blobs));
		cvShowImage( "Blob Outlines Background", abandonment_obj->contour_image.Ptr());			
		cvWaitKey(0);*/

		if(SHOW_STATUS)
			cout<<"Done with abandonment_analysis.\n";
		//-----------------------------------done performing abandonment analysis---------------------------------------------//


		//-----------------------------------performing blob filtering---------------------------------------------//

		if((filter_obj->filter_params->enable_filtering)&&(filter_obj->candidate_removed_objects.size()>0)){

			bool filter_status=filter_obj->filterRemovedOjects(tracking_obj->tracked_blobs,bkg_img->Ptr(),ui_obj->proc_params->proc_resize_factor);
			if(filter_status)
				removeMarkedObjects(ui_obj);

			filter_obj->refreshSelectionImage(ui_obj->frame_data_original);
			cvShowImage(window_names->blob_filtering,filter_obj->temp_selection_image);

			if(SHOW_STATUS)
				cout<<"Done with blob filtering.\n";
		}else{
			cvShowImage(window_names->blob_filtering,ui_obj->frame_data_original);
		}
		//-----------------------------------done performing blob filtering---------------------------------------------//

		tracking_obj->updateBlobImage(detection_obj->bounding_box_image,line_thickness,line_type,shift);

		if(SHOW_STATUS)
			cout<<"Done with updateBlobImage.\n";


		//---------------------------------displaying and writing output images-------------------------------------------//

		if((ui_obj->output_size.width!=ui_obj->proc_size.width)||(ui_obj->output_size.height!=ui_obj->proc_size.height)){
			cvResize(tracking_obj->tracked_blobs_image,ui_obj->output_img_rgb);
		}else{
			cvCopy(tracking_obj->tracked_blobs_image,ui_obj->output_img_rgb);
			//ui_obj->output_img_rgb=tracking_obj->tracked_blobs_image;
		}

		cvShowImage(window_names->blob_tracking,ui_obj->output_img_rgb);
		cvWriteFrame(ui_obj->bounding_box_video_writer,ui_obj->output_img_rgb);	


		/*if(ui_obj->proc_params->change_proc_resolution){
		cvResize( ui_obj->frame_data_resized->Ptr(),ui_obj->output_img_rgb);
		}else{
		cvCopy(ui_obj->frame_data_resized->Ptr(),ui_obj->output_img_rgb);			
		}*/


		if((ui_obj->output_size.width!=ui_obj->proc_size.width)||(ui_obj->output_size.height!=ui_obj->proc_size.height)){
			cvResize(ui_obj->proc_img_rgb,ui_obj->output_img_rgb_2);
		}else{
			cvCopy(ui_obj->proc_img_rgb,ui_obj->output_img_rgb_2);
			//ui_obj->output_img_rgb_2=ui_obj->proc_img_rgb;
		}

		combineImages(ui_obj->output_img_rgb,ui_obj->output_img_rgb_2,ui_obj->combined_image_x2_1,HORIZONTAL_JOIN);	

		combineImages(ui_obj->combined_image_x2_1,ui_obj->combined_image_x2_2,ui_obj->combined_image_x4,VERTICAL_JOIN);
		combineImages(ui_obj->combined_image_x4,ui_obj->combined_image_x2_3,ui_obj->combined_image_x6,HORIZONTAL_JOIN);
		cvPutText(ui_obj->combined_image_x6,fps_text,cvPoint(fps_x,fps_y), &fps_font,fps_col);
		cvShowImage(window_names->proc, ui_obj->combined_image_x6);
		cvWriteFrame(ui_obj->combined_video_writer,ui_obj->combined_image_x6);

		if(SHOW_STATUS)
			cout<<"Done with combineImages.\n";		

		//image_blobs=getBlobs2(low_threshold_mask.Ptr(), frame_data.Ptr());

		//cvShowImage( "high_threshold_mask", high_threshold_mask.Ptr() );

		//cvShowImage( "Components", dst );
		//cvShowImage( "Blobs", image_blobs );
		//cvShowImage( "static_threshold_mask", static_threshold_mask.Ptr());

		//---------------------------------done displaying and writing output images-------------------------------------------//

		//-------------------------looking for parameter changes that require the state variables or windows to be reset------------------//

		if(show_help){
			cvShowImage("Keys",key_image);
		}else{
			cvDestroyWindow("Keys");
		}		

		if(bgs_obj->bgs_method_changed){

			bgs_obj->bgs_method_changed=false;	

			if(SHOW_STATUS)
				cout<<"\nRunning initBGSMethod\n";

			bgs_obj->initBGSMethod();

			if(SHOW_STATUS)
				cout<<"\nRunning InitModel\n";

			bgs_obj->bgs->InitModel(*bkg_img);

			if(bgs_obj->bgs_params->bgs_toggle_params->bgs_method==BGS_ZIVKOVIC_GMM){	

				bgs_obj->bgs->Subtract2(*bkg_img,*(ui_obj->low_threshold_mask),*(ui_obj->high_threshold_mask),
					tracking_obj->tracked_mask);

			}else if(bgs_obj->bgs_params->bgs_toggle_params->bgs_method==BGS_GRIMSON_GMM){

				bgs_obj->bgs->Subtract2(*bkg_img,*(ui_obj->low_threshold_mask),*(ui_obj->high_threshold_mask),
					*(ui_obj->static_threshold_mask),tracking_obj->tracked_mask,static_threshold);
			}

			cvDestroyWindow(window_names->bgs);
			bgs_obj->initWindow(window_names->bgs);						
		}

		if(frg_proc_obj->shadow_method_updated){

			cout<<"\nResetting foreground analysis window.\n";
			frg_proc_obj->shadow_method_updated=false;
			cvDestroyWindow(window_names->frg_analysis);
			frg_proc_obj->initWindow(window_names->frg_analysis);			
		}

		if(abandonment_obj->abandonment_updated){

			cout<<"\nResetting abandonment analysis window.\n";
			abandonment_obj->abandonment_updated=false;
			cvDestroyWindow(window_names->abandonment_analysis);
			abandonment_obj->initWindow(window_names->abandonment_analysis);
		}

		if(filter_obj->match_updated){
			cout<<"\nResetting blob filtering window.\n";
			filter_obj->match_updated=false;
			cvDestroyWindow(window_names->blob_filtering);
			filter_obj->initWindow(window_names->blob_filtering);
		}

		waitForKeyPress(1,ui_obj);	
		fps_obj->updateFPS(frame_count);
	}

	cvDestroyAllWindows();

	cout<<"\nPerforming cleanup before exiting..........\n";

	/*ui_obj->clearInputInterface();
	ui_obj->clearOutputInterface();
	ui_obj->clearProcInterface();
	if(SHOW_STATUS)
	cout<<"Done clearing ui_obj.\n";*/	

	destroySystemObjects();

	if(SHOW_STATUS)
		cout<<"Done destroySystemObjects.\n";

	//if(!ui_obj->io_params->capture_from_camera)
	if(temp_tracked_mask)
		delete(temp_tracked_mask);

	delete(ui_obj);
	if(SHOW_STATUS)
		cout<<"Done deleting ui_obj.\n";

	delete(param_obj);	
	delete(fps_obj);

	window_names->clearWindowNames();
	delete(window_names);

	if(SHOW_STATUS)
		cout<<"Done deleting objects.\n";
}

inline void processIOChanges(UserInterface *ui_obj,int &frame_count,int min_blob_size_full_res){	

	if(ui_obj->io_updated){	

		ui_obj->io_updated=false;
		if(ui_obj->io_source_updated){
			cvDestroyWindow(window_names->io);
			ui_obj->initIOWindow(window_names->io);
			ui_obj->io_source_updated=false;
		}
		ui_obj->updateProcParams(0);
		ui_obj->initInterface(1,1,1);

		//destroySystemObjects();
		initStateVariables(ui_obj,1,1);
		ui_obj->input_source_changed=true;
		frame_count=-1;	
	}

	if(ui_obj->resize_for_disp_updated){
		ui_obj->resize_for_disp_updated=false;
		ui_obj->initOutputInterface();
	}

	if(ui_obj->proc_updated){	

		ui_obj->proc_updated=false;	

		ui_obj->initInterface(0,1,1);
		//destroySystemObjects();			

		if(ui_obj->proc_params->change_proc_resolution){
			double resize_factor=ui_obj->proc_params->proc_resize_factor;
			detection_obj->params->min_blob_size=(int)((double)min_blob_size_full_res/(resize_factor*resize_factor));				
			cout<<"\nMinimum blob size changed to "<<detection_obj->params->min_blob_size<<"\n";
		}else{
			detection_obj->params->min_blob_size=min_blob_size_full_res;
		}
		cvSetTrackbarPos("min_blob_size",window_names->blob_detection,detection_obj->params->min_blob_size);			

		initStateVariables(ui_obj,1,0);
		ui_obj->input_source_changed=true;
		frame_count=-1;	
	}
}

// return the fraction of total pixels that are in the foreground
double getForegroundRatio(IplImage *img){

	if(img->nChannels!=1){
		cout<<"Image is not gray scale.\n";
		return 0;
	}
	int frg_count=0;
	for(int r=0;r<img->height;r++){
		for(int c=0;c<img->width;c++){
			int pixel_pos=r*img->widthStep+c;
			unsigned char pixel_val=(unsigned char)img->imageData[pixel_pos];
			if(pixel_val==FOREGROUND_COLOR)
				frg_count++;
		}
	}
	int total_count=img->height*img->width;
	double frg_ratio=(double)frg_count/(double)total_count;

	return frg_ratio;
}
// initialize the state variables for all the system modules
void initStateVariables(UserInterface *ui_obj,int pre_process_flag,int filter_flag){

	if(pre_process_flag){
		pre_process_obj->initStateVariables(ui_obj->proc_size);
		if(SHOW_STATUS)
			cout<<"Done with pre_process_obj initStateVariables\n";
	}

	bgs_obj->initStateVariables(ui_obj->proc_size);
	if(SHOW_STATUS)
		cout<<"Done with bgs_obj initStateVariables\n";

	frg_proc_obj->initStateVariables(ui_obj->proc_size);
	if(SHOW_STATUS)
		cout<<"Done with frg_proc_obj initStateVariables\n";

	detection_obj->initStateVariables(ui_obj->proc_size);
	if(SHOW_STATUS)
		cout<<"Done with detection_obj initStateVariables\n";

	tracking_obj->initStateVariables(ui_obj->proc_size);
	if(SHOW_STATUS)
		cout<<"Done with tracking_obj initStateVariables\n";

	abandonment_obj->initStateVariables(ui_obj->proc_size);
	if(SHOW_STATUS)
		cout<<"Done with abandonment_obj initStateVariables\n";

	if(filter_flag){
		filter_obj->initStateVariables(ui_obj->original_size);
		if(SHOW_STATUS)
			cout<<"Done with filter_obj initStateVariables\n";
	}
}

// initializing the objects for the system modules
void initSystemObjects(UserInterface *ui_obj,SystemParameters *param_obj,int pre_process_flag,int filter_flag){	

	if(pre_process_flag){
		if(pre_process_obj){
			delete(pre_process_obj);
			cvDestroyWindow(window_names->pre_processing);
		}
		pre_process_obj=new PreProcessing(ui_obj->proc_size,param_obj->pre_process_params_init);		
		pre_process_obj->initWindow(window_names->pre_processing);

		if(SHOW_STATUS)
			cout<<"Done with pre_process_obj creation\n";
	}

	if(bgs_obj){
		delete(bgs_obj);
		cvDestroyWindow(window_names->bgs);
	}
	bgs_obj=new BgsModels(ui_obj->proc_size,param_obj->bgs_params_init);	
	bgs_obj->initWindow(window_names->bgs);

	if(SHOW_STATUS)
		cout<<"Done with bgs_obj creation\n";

	if(frg_proc_obj){
		delete(frg_proc_obj);
		cvDestroyWindow(window_names->frg_analysis);
		cvDestroyWindow(window_names->morph_analysis);
	}
	frg_proc_obj=new ForegroundProc(ui_obj->proc_size,param_obj->frg_params_init,param_obj->frg_morph_params_init);		
	frg_proc_obj->initWindow(window_names->frg_analysis);	
	frg_proc_obj->initMorphWindow(window_names->morph_analysis);

	if(SHOW_STATUS)
		cout<<"Done with frg_proc_obj creation\n";

	if(detection_obj){
		delete(detection_obj);
		cvDestroyWindow(window_names->blob_detection);
	}
	detection_obj=new BlobDetection(ui_obj->proc_size,param_obj->blob_detect_params_init);	
	detection_obj->initWindow(window_names->blob_detection);	

	if(SHOW_STATUS)
		cout<<"Done with detection_obj creation\n";

	if(tracking_obj){
		delete(tracking_obj);
		cvDestroyWindow(window_names->blob_tracking);
		cvDestroyWindow(window_names->blob_matching);
	}
	tracking_obj=new BlobTracking(ui_obj->proc_size,param_obj->blob_matching_params_init,param_obj->blob_tracking_params_init);
	tracking_obj->initWindowTrack(window_names->blob_tracking);
	tracking_obj->initWindowMatch(window_names->blob_matching);	

	if(SHOW_STATUS)
		cout<<"Done with tracking_obj creation\n";

	if(abandonment_obj){
		delete(abandonment_obj);
		cvDestroyWindow(window_names->abandonment_analysis);
	}
	abandonment_obj=new AbandonmentAnalysis(ui_obj->proc_size,param_obj->abandonment_params_init);
	abandonment_obj->initWindow(window_names->abandonment_analysis);

	if(SHOW_STATUS)
		cout<<"Done with abandonment_obj creation\n";

	if(filter_flag){
		if(filter_obj){
			delete(filter_obj);
			cvDestroyWindow(window_names->blob_filtering);
		}
		filter_obj=new BlobFilter(ui_obj->original_size,param_obj->filter_params_init);
		filter_obj->initWindow(window_names->blob_filtering);

		if(SHOW_STATUS)
			cout<<"Done with filter_obj creation\n";
	}
}

void destroySystemObjects(){	

	delete(pre_process_obj);
	delete(bgs_obj);
	delete(frg_proc_obj);
	delete(detection_obj);
	delete(tracking_obj);
	delete(abandonment_obj);
	delete(filter_obj);

}

// push all the objects of the specified type into the background
void removeAllObjects(UserInterface *ui_obj,int object_type){

	bool found_obj=false;
	for(int i=0;i<tracking_obj->tracked_blob_count;i++){
		CBlob *current_blob=tracking_obj->tracked_blobs->GetBlob(i);
		if(object_type==OBJ_ALL){
			bgs_obj->bgs->pushIntoBackground(current_blob->GetBoundingBox(),*(ui_obj->frame_data_resized));
			found_obj=true;

		}else if(object_type==OBJ_STATIC){
			if(current_blob->is_abandoned||current_blob->is_removed)
				continue;
			if(current_blob->is_static){
				bgs_obj->bgs->pushIntoBackground(current_blob->GetBoundingBox(),*(ui_obj->frame_data_resized));	
				found_obj=true;
			}
		}else if(object_type==OBJ_ABANDONED){			
			if(current_blob->is_removed)
				continue;
			if(current_blob->is_abandoned){
				bgs_obj->bgs->pushIntoBackground(current_blob->GetBoundingBox(),*(ui_obj->frame_data_resized));	
				found_obj=true;
			}
		}else if(object_type==OBJ_REMOVED){

			if(current_blob->is_removed){
				bgs_obj->bgs->pushIntoBackground(current_blob->GetBoundingBox(),*(ui_obj->frame_data_resized));
				found_obj=true;
			}
		}
	}
	if(found_obj)
		cout<<"\nAll objects of the selected type have been removed.\n";
	else 
		cout<<"\nNo objects of the selected type to remove.\n";
}
// allow the user to select an object in the tracking system by clicking on it and push it into the background
void selectAndRemoveObject(UserInterface *ui_obj,char *selection_window_name,int object_type){

	int obj_count=tracking_obj->getSelectionImage(ui_obj->frame_data_resized->Ptr(),ui_obj->selection_image,object_type,ui_obj->proc_params->change_proc_resolution);

	if(obj_count==0){
		if(object_type==OBJ_ALL)
			cout<<"\nNo objects found\n";
		else if(object_type==OBJ_STATIC)
			cout<<"\nNo static objects found\n";
		else if(object_type==OBJ_ABANDONED)
			cout<<"\nNo abandoned objects found\n";
		else if(object_type==OBJ_REMOVED)
			cout<<"\nNo removed objects found\n";

	}else if(ui_obj->selectObject(selection_window_name,ui_obj->selection_image)){

		if(ui_obj->proc_params->change_proc_resolution){
			ui_obj->mouse_click_point.x=(int)((double)ui_obj->mouse_click_point.x/ui_obj->proc_params->proc_resize_factor);
			ui_obj->mouse_click_point.y=(int)((double)ui_obj->mouse_click_point.y/ui_obj->proc_params->proc_resize_factor);
		}

		CBlob *selected_blob=tracking_obj->getBlobContainingPoint(ui_obj->mouse_click_point,object_type);
		if(!selected_blob){
			cout<<"\nThe selected point does not lie within any valid object.\n";
		}else{						
			bgs_obj->bgs->pushIntoBackground(selected_blob->GetBoundingBox(),*(ui_obj->frame_data_resized));
			cout<<"\nThe selected object has been removed.\n";
		}
	}
}

// allow the user to select one or more objects through and remove them from the filtering system
void selectAndRemoveCandidateObject(UserInterface *ui_obj,char *selection_window_name){

	if(filter_obj->candidate_removed_objects.size()==0){
		cout<<"No objects to remove.\n";
		return;
	}

	cvCopy(ui_obj->frame_data_original,filter_obj->temp_selection_image);
	filter_obj->showObjectsFromEnd(0);
	ui_obj->selectObject(selection_window_name,filter_obj->temp_selection_image);

	int obj_id=filter_obj->getObjectContainingPoint(ui_obj->mouse_click_point);
	if(obj_id<0){
		cout<<"\nThe selected point does not lie within any valid object.\n";
	}else{						
		filter_obj->removeCandidateObject(obj_id);
		cout<<"\nThe selected object has been removed.\n";
	}
}
// remove all objects marked as acknowledged from the tracking system and push them into the background
void removeMarkedObjects(UserInterface *ui_obj){
	for(int i=0;i<tracking_obj->tracked_blob_count;i++){
		CBlob *current_blob=tracking_obj->tracked_blobs->GetBlob(i);
		if(current_blob->is_ack)
			bgs_obj->bgs->pushIntoBackground(current_blob->GetBoundingBox(),*(ui_obj->frame_data_resized));
	}
}
// allow the user to select one or more objects through their bounding boxes and push them into the background
void selectAndPushObjectToBackground(UserInterface *ui_obj){

	int added_object_count=0;
	bool received_key_press=false;
	int pressed_key=0;
	bool remove_objects=false;
	int obj_min_x,obj_min_y,obj_max_x,obj_max_y,obj_width,obj_height;
	IplImage *temp_selection_image=cvCreateImage(ui_obj->original_size,IPL_DEPTH_8U,3);
	vector<obj_struct*> added_objects;

	if(ui_obj->proc_params->change_proc_resolution)
		cvResize(ui_obj->frame_data_resized->Ptr(),temp_selection_image);
	else
		cvCopy(ui_obj->frame_data_resized->Ptr(),temp_selection_image);

	cvNamedWindow(filter_obj->selection_window,1);	
	cvSetMouseCallback(filter_obj->selection_window,BlobFilter::getClickedPoint,(void*)filter_obj);
	//int pressed_key=cvWaitKey(wait_status);
	/*if(pressed_key!=27)
	use_obj=1;*/
	while(!received_key_press){

		cvShowImage(filter_obj->selection_window,temp_selection_image);
		int obj_add_status=filter_obj->getObject(temp_selection_image);
		if(obj_add_status==ADD_OBJ){			
			cvRectangle(temp_selection_image, filter_obj->new_obj->min_point, filter_obj->new_obj->max_point,filter_obj->outline_params->obj_col,filter_obj->outline_params->line_thickness,filter_obj->outline_params->line_type, filter_obj->outline_params->shift);	
			added_objects.push_back(filter_obj->new_obj);
			added_object_count++;				
		}else if(obj_add_status==CANCEL_OP){	
			cvDestroyWindow(filter_obj->selection_window);
			return;
		}else if(obj_add_status==OP_COMPLETE){			
			break;
		}
		pressed_key=cvWaitKey(1);
		if(pressed_key>0)
			received_key_press=true;
	}

	for(int i=0;i<added_object_count;i++){
		obj_struct *obj=added_objects[i];

		obj_min_x=obj->min_point.x;
		obj_min_y=obj->min_point.y;
		obj_max_x=obj->max_point.x;
		obj_max_y=obj->max_point.y;

		if(ui_obj->proc_params->change_proc_resolution){
			obj_min_x=(int)((double)obj_min_x/ui_obj->proc_params->proc_resize_factor);
			obj_min_y=(int)((double)obj_min_y/ui_obj->proc_params->proc_resize_factor);
			obj_max_x=(int)((double)obj_max_x/ui_obj->proc_params->proc_resize_factor);
			obj_max_y=(int)((double)obj_max_y/ui_obj->proc_params->proc_resize_factor);
		}

		obj_width=obj_max_x-obj_min_x;
		obj_height=obj_max_y-obj_min_y;
		bgs_obj->bgs->pushIntoBackground(cvRect(obj_min_x,obj_min_y,obj_width,obj_height),*(ui_obj->frame_data_resized));
		delete(obj);
	}
	added_objects.clear();

	cvDestroyWindow(filter_obj->selection_window);
	if(SHOW_FILTER_STATUS)
		cout<<"Done selectAndPushObjectToBackground\n";
}