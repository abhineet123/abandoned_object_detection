#include "userInterface.hpp"

bool UserInterface::io_updated;
bool UserInterface::io_source_updated;
bool UserInterface::proc_updated;
bool UserInterface::resize_for_disp_updated;

CvSize UserInterface::proc_size;
proc_struct* UserInterface::proc_params;
io_struct* UserInterface::io_params;

char* UserInterface::io_help_window;
char* UserInterface::proc_help_window;

IplImage* UserInterface::io_help_image;
IplImage* UserInterface::proc_help_image;

UserInterface::UserInterface(io_struct *ui_io_params_init,proc_struct *ui_proc_params_init){	

	input_file_name=new char[MAX_FILE_NAME_LENGTH];
	input_folder_path=new char[MAX_FOLDER_NAME_LENGTH];
	input_file_path=new char[MAX_FILE_PATH_LENGTH];

	output_file_name=new char[MAX_FILE_NAME_LENGTH];
	output_folder_path=new char[MAX_FOLDER_NAME_LENGTH];
	output_file_path=new char[MAX_FILE_PATH_LENGTH];	

	io_help_window=new char[MAX_NAME];
	sprintf(io_help_window,"Input/Output Help");

	proc_help_window=new char[MAX_NAME];
	sprintf(proc_help_window,"Processing Help");

	io_help_image=cvLoadImage("help/io_help.jpg");
	proc_help_image=cvLoadImage("help/proc_help.jpg");

	video_reader=NULL;

	bounding_box_video_writer=NULL;
	combined_video_writer=NULL;
	static_mask_video_writer=NULL;

	/*bkg_video_writer=NULL;
	unproc_frg_mask_video_writer=NULL;
	proc_frg_mask_video_writer=NULL;	
	frg_mask_video_writer=NULL;	*/	

	output_img_rgb=NULL;
	output_img_rgb_2=NULL;
	output_img_gray=NULL;

	combined_image_x2_1=NULL;
	combined_image_x2_2=NULL;
	combined_image_x2_gray=NULL;
	combined_image_x4=NULL;

	output_images_allocated=false;		

	frame_img=NULL;
	low_threshold_img=NULL;
	high_threshold_img=NULL;
	static_threshold_img=NULL;

	frame_data_resized=NULL;
	low_threshold_mask=NULL;
	high_threshold_mask=NULL;
	static_threshold_mask=NULL;

	proc_img_rgb=NULL;
	proc_img_gray=NULL;

	selection_image=NULL;
	frame_data_original=NULL;

	proc_images_allocated=false;

	io_updated=false;
	io_source_updated=false;
	proc_updated=false;
	resize_for_disp_updated=false;
	rewind_buffer_allocated=false;

	wait_status=0;

	io_params=new io_struct;
	initIOParams(ui_io_params_init);

	proc_params=new proc_struct;	
	initProcParams(ui_proc_params_init);	

	if(SHOW_STATUS_BGS)
		cout<<"Done with proc_params creation\n";

	initInputInterface();
	input_source_changed=true;

	updateProcParamsGlobal();
	initProcInterface();

	initOutputInterface();

	if(SHOW_STATUS_BGS)
		cout<<"Done with io_params creation\n";

	point_selected=false;	
}
UserInterface::~UserInterface(){

	clearStateVariables();

	delete(input_file_name);
	if(SHOW_STATUS_BGS)
		cout<<"deleted input_file_name\n";

	delete(input_folder_path);
	if(SHOW_STATUS_BGS)
		cout<<"deleted input_folder_path\n";

	delete(input_file_path);
	if(SHOW_STATUS_BGS)
		cout<<"deleted input_file_path\n";

	delete(output_file_name);
	if(SHOW_STATUS_BGS)
		cout<<"deleted output_file_name\n";

	/*cout<<"output_folder_path="<<output_folder_path<<"\n";
	delete(output_folder_path);
	if(SHOW_STATUS_BGS)
	cout<<"deleted output_folder_path\n";*/

	delete(output_file_path);
	if(SHOW_STATUS_BGS)
		cout<<"deleted output_file_path\n";

	delete(io_help_window);
	if(SHOW_STATUS_BGS)
		cout<<"deleted io_help_window\n";

	delete(proc_help_window);
	if(SHOW_STATUS_BGS)
		cout<<"deleted proc_help_window\n";

	/*delete(io_params);
	delete(proc_params);*/

}
void UserInterface::clearStateVariables(){

	clearInputInterface();
	clearOutputInterface();
	clearProcInterface();
}
// clear the video reader and images allocated for reading the input frames
void UserInterface::clearInputInterface(){
	if(SHOW_STATUS_BGS)
		cout<<"Start clearInputInterface\n";

	if(video_reader)
		cvReleaseCapture(&video_reader);

	if(selection_image)
		cvReleaseImage(&selection_image);

	if(frame_data_original)
		cvReleaseImage(&frame_data_original);

	if(rewind_buffer_allocated){
		for(int i=0;i<REWIND_BUFFER_SIZE;i++){
			cvReleaseImage(&rewind_buffer[i]);
		}
		rewind_buffer_allocated=false;
	}

	if(SHOW_STATUS_BGS)
		cout<<"Done clearInputInterface\n";

}
// clear the video writer and images allocated for saving and displaying the output results
void UserInterface::clearOutputInterface(){

	if(SHOW_STATUS_BGS)
		cout<<"Start clearOutputInterface\n";

	if(output_images_allocated){

		cvReleaseImage(&combined_image_x2_1);
		cvReleaseImage(&combined_image_x2_2);
		cvReleaseImage(&combined_image_x2_3);		
		cvReleaseImage(&combined_image_x2_gray);
		cvReleaseImage(&combined_image_x4);
		cvReleaseImage(&combined_image_x6);

		cvReleaseImage(&output_img_rgb);
		cvReleaseImage(&output_img_rgb_2);
		cvReleaseImage(&output_img_gray);		

		output_images_allocated=false;
	}

	if(bounding_box_video_writer)
		cvReleaseVideoWriter(&bounding_box_video_writer);

	if(combined_video_writer)
		cvReleaseVideoWriter(&combined_video_writer);

	if(static_mask_video_writer)
		cvReleaseVideoWriter(&static_mask_video_writer);

	/*if(bkg_video_writer)
	cvReleaseVideoWriter(&bkg_video_writer);

	if(unproc_frg_mask_video_writer)
	cvReleaseVideoWriter(&unproc_frg_mask_video_writer);

	if(proc_frg_mask_video_writer)
	cvReleaseVideoWriter(&proc_frg_mask_video_writer);	

	if(frg_mask_video_writer)
	cvReleaseVideoWriter(&frg_mask_video_writer);*/

	if(SHOW_STATUS_BGS)
		cout<<"Done clearOutputInterface\n";

}
// allocate and initialize the video reader and images for reading the input frames
void UserInterface::initInputInterface(){	

	if(SHOW_STATUS_BGS)
		cout<<"Start initInputInterface\n";

	clearInputInterface();

	if(!io_params->capture_from_camera) {
		if(io_params->video_file_source==PETS2006){
			cout<<"Using PETS2006 dataset.\n";
			skipped_secs=PETS2006_SKIP_SEC;	
			sprintf(input_folder_path,"input/PETS2006");
			sprintf(input_file_name,"set_%d_view_%d_size_%d",io_params->set_id,io_params->view_id,io_params->size_id);
		}else if(io_params->video_file_source==PETS2007){
			cout<<"Using PETS2007 dataset.\n";
			skipped_secs=PETS2007_SKIP_SEC;	
			sprintf(input_folder_path,"input/PETS2007");
			sprintf(input_file_name,"set_%d_view_%d_size_%d",io_params->set_id,io_params->view_id,io_params->size_id);
		}else if(io_params->video_file_source==AVSS){
			cout<<"Using AVSS dataset.\n";
			skipped_secs=AVSS_SKIP_SEC;	
			sprintf(input_folder_path,"input/AVSS");
			sprintf(input_file_name,"set_%d_size_%d",io_params->set_id,io_params->size_id);
		}else if(io_params->video_file_source==CUSTOM){
			cout<<"Using custom dataset.\n";
			skipped_secs=CUSTOM_SKIP_SEC;	
			sprintf(input_folder_path,"input/Custom");
			sprintf(input_file_name,"set_%d_size_%d",io_params->set_id,io_params->size_id);
		}else{
			cout<<"\nInvalid input dataset specified: "<<io_params->video_file_source<<"\n";
			exit(0);
			//io_params->exit_program=1;
		}

		cout<<"Getting input from video file "<<input_file_name<<"\n";

		if(!checkFolderExistence(input_folder_path)){
			cout<<"\n";
			cout<<"Input folder --- "<<input_folder_path<<" --- does not exist.\n";
			cout<<"\n";
			exit(0);
		}
		sprintf(input_file_path,"%s/%s.avi",input_folder_path,input_file_name);
		video_reader = cvCaptureFromAVI(input_file_path);
	} else {
		cout<<"\nGetting input from camera.\n";
		skipped_secs=CAMERA_SKIP_SEC;		
		video_reader = cvCaptureFromCAM(CV_CAP_ANY);
	}

	if(!video_reader) {
		IplImage *message_img;
		if(io_params->capture_from_camera)
			message_img=cvLoadImage("camera.jpg");
		else
			message_img=cvLoadImage("video_file.jpg");
		cvShowImage("Input Source Unavailable",message_img);
		cvReleaseImage(&message_img);

		cout << "\nInput Source Unavailable.Press any key to exit.\n";
		cvWaitKey(0);
		io_params->exit_program=1;
	}

	if(skipped_secs>0){
		cout<<"Skipping the first "<<skipped_secs<<" seconds.....\n";
	}

	io_params->original_width=(int)cvGetCaptureProperty(video_reader, CV_CAP_PROP_FRAME_WIDTH);
	io_params->original_height=(int)cvGetCaptureProperty(video_reader, CV_CAP_PROP_FRAME_HEIGHT);
	io_params->fps=(int)cvGetCaptureProperty(video_reader, CV_CAP_PROP_FPS);		

	original_size=cvSize(io_params->original_width, io_params->original_height);	

	selection_image=cvCreateImage(original_size,IPL_DEPTH_8U,3);
	frame_data_original=cvCreateImage(original_size,IPL_DEPTH_8U,3);

	for(int i=0;i<REWIND_BUFFER_SIZE;i++){		
		rewind_buffer[i]=cvCreateImage(original_size,IPL_DEPTH_8U,3);
	}
	buffer_start_index=0;
	buffer_end_index=buffer_current_index=-1;	
	rewind_buffer_allocated=true;	

	if(SHOW_STATUS_BGS)
		cout<<"Done initInputInterface\n";
}
// allocate and initialize the video writers and images for saving and displaying the output results
void UserInterface::initOutputInterface(){

	if(SHOW_STATUS_BGS)
		cout<<"Start initOutputInterface\n";

	clearOutputInterface();

	if(proc_params->change_disp_resolution){
		output_size=cvSize((int)(proc_size.width*proc_params->disp_resize_factor),(int)(proc_size.height*proc_params->disp_resize_factor));		
	}else{
		output_size=cvSize(original_size.width,original_size.height);		
	}
	cout<<"output_width="<<output_size.width<<"\t output_height="<<output_size.height<<"\n";

	combined_image_x2_1=initializeCombinedImage(output_size,output_size,3,HORIZONTAL_JOIN);
	combined_image_x2_2=initializeCombinedImage(output_size,output_size,3,HORIZONTAL_JOIN);
	combined_image_x2_3=initializeCombinedImage(output_size,output_size,3,VERTICAL_JOIN);
	combined_image_x2_gray=initializeCombinedImage(output_size,output_size,1,HORIZONTAL_JOIN);

	image_size_x2=cvSize(combined_image_x2_1->width,combined_image_x2_1->height);

	combined_image_x4=initializeCombinedImage(image_size_x2,image_size_x2,3,VERTICAL_JOIN);

	image_size_x4=cvSize(combined_image_x4->width, combined_image_x4->height);	

	combined_image_x6=initializeCombinedImage(image_size_x4,cvSize(combined_image_x2_3->width,combined_image_x2_3->height),3,HORIZONTAL_JOIN);

	image_size_x6=cvSize(combined_image_x6->width, combined_image_x6->height);

	output_img_rgb=cvCreateImage(output_size,IPL_DEPTH_8U,3);		
	output_img_rgb_2=cvCreateImage(output_size,IPL_DEPTH_8U,3);	
	output_img_gray=cvCreateImage(output_size,IPL_DEPTH_8U,1);

	output_images_allocated=true;	

	if(!io_params->capture_from_camera) {		
		if(io_params->video_file_source==PETS2006){
			sprintf(output_folder_path,"output/PETS2006");						
		}else if(io_params->video_file_source==PETS2007){
			sprintf(output_folder_path,"output/PETS2007");					
		}else if(io_params->video_file_source==AVSS){
			sprintf(output_folder_path,"output/AVSS");				
		}else if(io_params->video_file_source==CUSTOM){
			sprintf(output_folder_path,"output/Custom");			
		}else{
			cout<<"\nInvalid input dataset specified: "<<io_params->video_file_source<<"\n";
			exit(0);
			//io_params->exit_program=1;
		}		
	}else{
		sprintf(output_folder_path,"output/Camera");
	}
	if(!checkFolderExistence(output_folder_path)){
		cout<<"\n";
		cout<<"Output folder --- "<<output_folder_path<<" --- does not exist.\n";
		cout<<"\n";
		exit(0);
	}
	if(!proc_params->change_proc_resolution){
		sprintf(output_file_name,"%s_resize_disable",input_file_name);
	}else{
		sprintf(output_file_name,"%s_resize_%d",input_file_name,proc_params->proc_resize_factor_10);
	}

	sprintf(output_file_path,"%s/%s_bounding_boxes.avi",output_folder_path,output_file_name);
	bounding_box_video_writer = cvCreateVideoWriter(output_file_path, CV_FOURCC('D', 'I', 'V', 'X'),
		io_params->fps, output_size, 1);

	/*sprintf(output_file_name,"output/%s_background.avi",output_template);	
	bkg_video_writer = cvCreateVideoWriter(output_file_name, CV_FOURCC('D', 'I', 'V', 'X'),
	io_params->fps, original_size, 1);

	sprintf(output_file_name,"output/%s_unproc_frg_mask.avi",output_template);	
	unproc_frg_mask_video_writer = cvCreateVideoWriter(output_file_name, CV_FOURCC('D', 'I', 'V', 'X'),
	io_params->fps, original_size,0);


	sprintf(output_file_name,"output/%s_proc_frg_mask.avi",output_template);	
	proc_frg_mask_video_writer = cvCreateVideoWriter(output_file_name, CV_FOURCC('D', 'I', 'V', 'X'),
	io_params->fps, original_size, 0);*/


	sprintf(output_file_path,"%s/%s_combined.avi",output_folder_path,output_file_name);	
	combined_video_writer = cvCreateVideoWriter(output_file_path, CV_FOURCC('D', 'I', 'V', 'X'),
		io_params->fps, image_size_x6, 1);


	sprintf(output_file_path,"%s/%s_static_mask.avi",output_folder_path,output_file_name);	
	static_mask_video_writer = cvCreateVideoWriter(output_file_path, CV_FOURCC('D', 'I', 'V', 'X'),
		io_params->fps, output_size,0);	

	if(SHOW_STATUS_BGS)
		cout<<"Done initOutputInterface\n";	
}
// clear the images allocated for storing the intermediate processing results
void UserInterface::clearProcInterface(){

	if(SHOW_STATUS_BGS)
		cout<<"Start clearProcInterface\n";

	if(proc_images_allocated){

		delete(frame_data_resized);
		delete(low_threshold_mask);
		delete(high_threshold_mask);
		delete(static_threshold_mask);

		cvReleaseImage(&proc_img_rgb);
		cvReleaseImage(&proc_img_gray);

		/*cvReleaseImage(&frame_img);
		cvReleaseImage(&low_threshold_img);
		cvReleaseImage(&high_threshold_img);
		cvReleaseImage(&static_threshold_img);*/

		proc_img_rgb=NULL;
		proc_img_gray=NULL;

		frame_img=NULL;
		low_threshold_img=NULL;
		high_threshold_img=NULL;
		static_threshold_img=NULL;

		frame_data_resized=NULL;
		low_threshold_mask=NULL;
		high_threshold_mask=NULL;
		static_threshold_mask=NULL;

		proc_images_allocated=false;
	}

	if(SHOW_STATUS_BGS)
		cout<<"Done clearProcInterface\n";
}
// allocate and initialize images for storing the intermediate processing results
void UserInterface::initProcInterface(){

	if(SHOW_STATUS_BGS)
		cout<<"Start initProcInterface\n";

	cout<<"\nProcessing interface updated with change_proc_resolution="<<proc_params->change_proc_resolution<<" and resize_factor="<<proc_params->proc_resize_factor<<"\n";
	cout<<"proc_width="<<proc_params->proc_width<<"\t proc_height="<<proc_params->proc_height<<"\n";

	//_getch();	

	clearProcInterface();

	proc_img_rgb=cvCreateImage(proc_size,IPL_DEPTH_8U,3);
	proc_img_gray=cvCreateImage(proc_size,IPL_DEPTH_8U,1);

	frame_img=cvCreateImage(proc_size,IPL_DEPTH_8U,3);
	low_threshold_img=cvCreateImage(proc_size,IPL_DEPTH_8U,1);
	high_threshold_img=cvCreateImage(proc_size,IPL_DEPTH_8U,1);
	static_threshold_img=cvCreateImage(proc_size,IPL_DEPTH_8U,1);

	frame_data_resized=new RgbImage(frame_img);
	low_threshold_mask=new BwImage(low_threshold_img);
	high_threshold_mask=new BwImage(high_threshold_img);
	static_threshold_mask=new BwImage(static_threshold_img);

	proc_images_allocated=true;	

	if(SHOW_STATUS_BGS)
		cout<<"Done initProcInterface\n";	
}
// initialize the frame processing parameters
void UserInterface::initProcParams(proc_struct *ui_proc_params_init) {

	if(SHOW_STATUS_BGS)
		cout<<"Start initProcParams\n";

	proc_params->change_proc_resolution=ui_proc_params_init->change_proc_resolution;
	proc_params->proc_resize_factor_10=ui_proc_params_init->proc_resize_factor_10;
	proc_params->change_disp_resolution=ui_proc_params_init->change_disp_resolution;
	proc_params->disp_resize_factor_10=ui_proc_params_init->disp_resize_factor_10;

	if(SHOW_STATUS_BGS)
		cout<<"Done initProcParams\n";
}

void UserInterface::updateProcParamsGlobal() {

	if(SHOW_STATUS_BGS)
		cout<<"Start updateProcParamsGlobal\n";

	if(proc_params->proc_resize_factor_10<10){
		cout<<"\nInvalid processing resize factor specified. Using a factor of 1 instead.\n";
		proc_params->proc_resize_factor_10=10;		
	}
	proc_params->proc_resize_factor=(double)(proc_params->proc_resize_factor_10)/10.0;

	if(proc_params->disp_resize_factor_10<10){
		cout<<"\nInvalid display resize factor specified. Using a factor of 1 instead.\n";
		proc_params->disp_resize_factor_10=10;
	}
	proc_params->disp_resize_factor=(double)(proc_params->disp_resize_factor_10)/10.0;

	if(proc_params->change_proc_resolution){		

		proc_params->proc_width=(int)((double)io_params->original_width/proc_params->proc_resize_factor);
		proc_params->proc_height=(int)((double)io_params->original_height/proc_params->proc_resize_factor);		

	}else{

		proc_params->proc_width=io_params->original_width;
		proc_params->proc_height=io_params->original_height;
	}

	cout<<"\nCarrying out processing at a resolution of "<<proc_params->proc_width<<" X "<<proc_params->proc_height<<"\n";
	cout<<"Original resolution: "<<io_params->original_width<<" X "<<io_params->original_height<<"\n\n";

	proc_size=cvSize(proc_params->proc_width, proc_params->proc_height);

	if(SHOW_STATUS_BGS)
		cout<<"Done updateProcParamsGlobal\n";
}

void UserInterface::updateProcParams(int) {

	cout<<"\nChanging processing resolution....\n";

	if(SHOW_STATUS_BGS)
		cout<<"Start updateProcParams\n";

	updateProcParamsGlobal();

	proc_updated=true;

	if(SHOW_STATUS_BGS)
		cout<<"Done updateProcParams\n";
}
// initialize input/output parameters
void UserInterface::initIOParams(io_struct *ui_io_params_init) {

	if(SHOW_STATUS_BGS)
		cout<<"Start initIOParams\n";

	io_params->capture_from_camera=ui_io_params_init->capture_from_camera;
	io_params->video_file_source=ui_io_params_init->video_file_source;
	io_params->exit_program=0;

	io_params->set_id=ui_io_params_init->set_id;
	io_params->view_id=ui_io_params_init->view_id;
	io_params->size_id=ui_io_params_init->size_id;	
	io_params->show_all_blobs=ui_io_params_init->show_all_blobs;	

	if(SHOW_STATUS_BGS)
		cout<<"Done initIOParams\n";
}
// initialize input/output and frame processing interfaces
void UserInterface::initInterface(int input_flag,int output_flag,int proc_flag){

	if(SHOW_STATUS_BGS)
		cout<<"Start initInterface\n";

	if(input_flag){
		initInputInterface();
		updateProcParamsGlobal();
	}

	if(proc_flag){
		initProcInterface();
	}

	if(output_flag){
		initOutputInterface();
	}	

	if(SHOW_STATUS_BGS)
		cout<<"Done initInterface\n";


}
void UserInterface::updateIOParams(int) {

	if(SHOW_STATUS_BGS)
		cout<<"Start updateIOParams\n";

	//cout<<"\nChanging the input source....\n";	
	io_updated=true;

	if(SHOW_STATUS_BGS)
		cout<<"Done updateIOParams\n";
}

void UserInterface::updateInputSource(int) {

	if(SHOW_STATUS_BGS)
		cout<<"Start updateInputSource\n";

	//cout<<"\nChanging the input source....\n";

	if(!io_params->capture_from_camera){
		io_params->set_id=0;		
	}

	io_source_updated=true;
	io_updated=true;

	if(SHOW_STATUS_BGS)
		cout<<"Done updateInputSource\n";
}
void UserInterface::updateDispResize(int){
	if(proc_params->disp_resize_factor_10<10)
		proc_params->disp_resize_factor_10=10;
	proc_params->disp_resize_factor=(double)(proc_params->disp_resize_factor_10)/10.0;
	resize_for_disp_updated=true;
}
void UserInterface::showProcHelpWindow(int mouse_event,int x,int y,int flags,void* param){

	if(mouse_event==CV_EVENT_LBUTTONDOWN){
		cvNamedWindow(proc_help_window,1);
		cvShowImage(proc_help_window,proc_help_image);
	}else if(mouse_event==CV_EVENT_RBUTTONDOWN){
		cvDestroyWindow(proc_help_window);
	}
}

void UserInterface::showIOHelpWindow(int mouse_event,int x,int y,int flags,void* param){

	if(mouse_event==CV_EVENT_LBUTTONDOWN){
		cvNamedWindow(io_help_window,1);
		cvShowImage(io_help_window,io_help_image);
	}else if(mouse_event==CV_EVENT_RBUTTONDOWN){
		cvDestroyWindow(io_help_window);
	}
}

void UserInterface::initProcWindow(char *window_name){

	if(SHOW_STATUS_BGS)
		cout<<"Start initProcWindow\n";

	cvNamedWindow(window_name,1);
	cvCreateTrackbar("chngProc",window_name,&(proc_params->change_proc_resolution),1,updateProcParams);
	cvCreateTrackbar("chngDisp",window_name,&(proc_params->change_disp_resolution),1,updateDispResize);
	cvCreateTrackbar("procRes10",window_name,&(proc_params->proc_resize_factor_10),100,updateProcParams);
	cvCreateTrackbar("dispRes10",window_name,&(proc_params->disp_resize_factor_10),100,updateDispResize);

	cvSetMouseCallback(window_name,showProcHelpWindow,(void*)this);

	if(SHOW_STATUS_BGS)
		cout<<"Done initProcWindow\n";
}

void UserInterface::initIOWindow(char *window_name){

	if(SHOW_STATUS_BGS)
		cout<<"Start initIOWindow\n";

	cvNamedWindow(window_name,1);

	cvCreateTrackbar("useCamera",window_name,&(io_params->capture_from_camera),1,updateInputSource);
	cvCreateTrackbar("exit",window_name,&(io_params->exit_program),1,NULL);

	if(!io_params->capture_from_camera)
		cvCreateTrackbar("dataset",window_name,&(io_params->video_file_source),3,updateInputSource);

	if(io_params->video_file_source==PETS2006){
		cvCreateTrackbar("set",window_name,&(io_params->set_id),6,updateIOParams);
		cvCreateTrackbar("view",window_name,&(io_params->view_id),3,updateIOParams);
	}else if(io_params->video_file_source==PETS2007){
		cvCreateTrackbar("set",window_name,&(io_params->set_id),9,updateIOParams);
		cvCreateTrackbar("view",window_name,&(io_params->view_id),3,updateIOParams);
	}else if(io_params->video_file_source==AVSS){
		cvCreateTrackbar("set",window_name,&(io_params->set_id),8,updateIOParams);		
	}else{
		cvCreateTrackbar("set",window_name,&(io_params->set_id),18,updateIOParams);
	}
	cvCreateTrackbar("size",window_name,&(io_params->size_id),2,updateIOParams);
	cvCreateTrackbar("showBlobs",window_name,&(io_params->show_all_blobs),1,NULL);

	cvSetMouseCallback(window_name,showIOHelpWindow,(void*)this);

	if(SHOW_STATUS_BGS)
		cout<<"Done initIOWindow\n";
}

int UserInterface::selectObject(char *window_name,IplImage *current_selection_image){

	int use_obj=1;

	cvNamedWindow(window_name,1);
	cvShowImage(window_name,current_selection_image);
	cvSetMouseCallback(window_name,getClickedPoint,(void*)this);
	//int pressed_key=cvWaitKey(wait_status);
	/*if(pressed_key!=27)
	use_obj=1;*/
	point_selected=false;
	while(!point_selected){
		//cout<<"Inside the loop\n";
		if(cvWaitKey(1)==27){
			use_obj=0;
			break;
		}			
	}
	cvDestroyWindow(window_name);
	return use_obj;
}
// capture and return the location of the mouse left click point
void UserInterface::getClickedPoint(int mouse_event,int x,int y,int flags,void* param){

	//cout<<"Start getClickedPoint\n";

	UserInterface *ui_obj=(UserInterface*)param;
	if(mouse_event==CV_EVENT_LBUTTONDOWN){
		cout<<"Received a click.\n";
		ui_obj->mouse_click_point.x=x;
		ui_obj->mouse_click_point.y=y;		
		ui_obj->wait_status=1;
		ui_obj->point_selected=true;
	}
	//cout<<"Done getClickedPoint\n";
}

bool UserInterface::checkFolderExistence(char *folder_path){
	return  !(_access(folder_path,0));
}



