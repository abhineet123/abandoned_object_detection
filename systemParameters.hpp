#ifndef _SYS_PARAMS_
#define _SYS_PARAMS_

#include <fstream>
#include <cstring>
#include <stdlib.h> 
#include <conio.h> 
#include <cv.h>

#include "userInterface.hpp"
#include "preProcessing.hpp"
#include "bgsModels.hpp"
#include "blobDetection.hpp"
#include "foregroundProcessing.hpp"
#include "blobTracking.hpp"
#include "abandonmentAnalysis.hpp"
#include "blobFilter.hpp"

#define MAX_LINE_SIZE 500
#define MAX_VAL_SIZE 20
#define SHOW_STATUS_SYS 0

using namespace std;
using namespace cv;

class SystemParameters{

public:

	SystemParameters();
	~SystemParameters();

	void readInitialParams(char *input_file_name);

	bool readInitialUIParams(ifstream &fin);
	bool readInitialPreProcessingParams(ifstream &fin);
	bool readInitialBGSParams(ifstream &fin);
	bool readInitialFrgParams(ifstream &fin);
	bool readInitialBlobDetectionParams(ifstream &fin);
	bool readInitialBlobMatchingParams(ifstream &fin);
	bool readInitialBlobTrackingParams(ifstream &fin);
	bool readInitialAbandonmentParams(ifstream &fin);
	bool readInitialBlobFilteringParams(ifstream &fin);

	int getParamStartIndex();
	double getParamVal(ifstream &fin);
	bool getParamString(int param_start_index);

	proc_struct *ui_proc_params_init;
	io_struct *ui_io_params_init;

	pre_process_struct *pre_process_params_init;

	bgs_struct *bgs_params_init;	

	frg_struct *frg_params_init;	
	morph_struct *frg_morph_params_init;

	blob_detection_struct *blob_detect_params_init;

	match_struct *blob_matching_params_init;
	track_struct *blob_tracking_params_init;

	abandonment_struct *abandonment_params_init;

	filter_struct *filter_params_init;

	char *input_buffer;
	char *param_str;

	int buffer_length;
};

#endif