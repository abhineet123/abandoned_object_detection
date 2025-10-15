#ifndef _UI_
#define _UI_

#include <cv.h>
#include <io.h>
#include<stdio.h>
#include<conio.h>

#include "Image.hpp"
#include "combineImages.hpp"

#define SMALL 0
#define MEDIUM 1
#define LARGE 2
#define MAX_FILE_NAME_LENGTH 100
#define MAX_FOLDER_NAME_LENGTH 200
#define MAX_FILE_PATH_LENGTH 500
#define SHOW_STATUS_BGS 0
#define PETS2006 0
#define PETS2007 1
#define AVSS 2
#define CUSTOM 3
#define REWIND_BUFFER_SIZE 1000

#define PETS2006_SKIP_SEC 0
#define PETS2007_SKIP_SEC 0
#define AVSS_SKIP_SEC 11
#define CUSTOM_SKIP_SEC 3
#define CAMERA_SKIP_SEC 8

#define MAX_NAME 100

using namespace std;

struct proc_struct{

	int change_proc_resolution;
	int proc_resize_factor_10;
	double proc_resize_factor;
	int disp_resize_factor_10;
	double disp_resize_factor;
	int change_disp_resolution;

	int proc_width;
	int proc_height;
};

struct io_struct{

	int capture_from_camera;
	int video_file_source;
	int set_id;
	int view_id;
	int size_id;
	int exit_program;

	int original_width;
	int original_height;
	int fps;

	int show_all_blobs;
};

class UserInterface{

public:

	UserInterface(io_struct *ui_io_params_init,proc_struct *ui_proc_params_init);	
	~UserInterface();
	void clearStateVariables();

	void initProcParams(proc_struct *ui_proc_params_init);
	static void updateProcParams(int);
	void initProcWindow(char *window_name);
	static void updateDispResize(int);
	

	void initIOParams(io_struct *ui_io_params_init);
	static void updateIOParams(int);
	static void updateInputSource(int);
	void initIOWindow(char *window_name);

	void clearInputInterface();
	void initInputInterface();

	void clearOutputInterface();
	void initOutputInterface();

	void clearProcInterface();
	void initProcInterface();

	void initInterface(int input_flag,int output_flag,int proc_flag);
	static void updateProcParamsGlobal();

	int selectObject(char *window_name,IplImage *current_selection_image);
	static void getClickedPoint(int mouse_event,int x,int y,int flags,void* param);

	static void showIOHelpWindow(int mouse_event,int x,int y,int flags,void* param);
	static void showProcHelpWindow(int mouse_event,int x,int y,int flags,void* param);

	bool checkFolderExistence(char *folder_path);

	CvCapture* video_reader;

	CvVideoWriter* bounding_box_video_writer;
	//CvVideoWriter* bkg_video_writer;
	//CvVideoWriter* unproc_frg_mask_video_writer;
	//CvVideoWriter* proc_frg_mask_video_writer;
	CvVideoWriter* combined_video_writer ;
	CvVideoWriter* static_mask_video_writer;

	IplImage *combined_image_x2_1;
	IplImage *combined_image_x2_2;
	IplImage *combined_image_x2_3;
	IplImage *combined_image_x2_gray;	
	IplImage *combined_image_x4;
	IplImage *combined_image_x6;


	IplImage *output_img_rgb;
	IplImage *output_img_rgb_2;
	IplImage *output_img_gray;

	CvSize image_size_x2;
	CvSize image_size_x4;
	CvSize image_size_x6;

	bool proc_images_allocated;
	bool output_images_allocated;

	char *input_file_name;
	char *input_folder_path;
	char *input_file_path;

	char *output_file_name;
	char *output_folder_path;
	char *output_file_path;	

	CvSize original_size;
	CvSize output_size;

	IplImage *proc_img_rgb;
	IplImage *proc_img_gray;

	IplImage *frame_img;
	IplImage *low_threshold_img;
	IplImage *high_threshold_img;
	IplImage *static_threshold_img;
	IplImage *frame_data_original;

	RgbImage *frame_data_resized;	
	BwImage *low_threshold_mask;
	BwImage *high_threshold_mask;
	BwImage *static_threshold_mask;

	static bool io_updated;
	static bool io_source_updated;
	static bool proc_updated;
	static bool resize_for_disp_updated;

	static CvSize proc_size;
	static proc_struct *proc_params;
	static io_struct *io_params;

	CvPoint mouse_click_point;

	bool point_selected;
	IplImage *selection_image;
	int wait_status; 

	static char *io_help_window;
	static char *proc_help_window;

	static IplImage *io_help_image;
	static IplImage *proc_help_image;

	IplImage* rewind_buffer[REWIND_BUFFER_SIZE];
	int buffer_start_index;
	int buffer_end_index;
	int buffer_current_index;
	bool rewind_buffer_allocated;

	int skipped_secs;
	bool input_source_changed;

	

};

#endif