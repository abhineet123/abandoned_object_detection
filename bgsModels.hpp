#ifndef _BGS_MODELS_
#define _BGS_MODELS_

#include "GrimsonGMM.hpp"
#include "ZivkovicAGMM.hpp"
#include "AdaptiveMedianBGS.hpp"
#include "Eigenbackground.hpp"
#include "WrenGA.hpp"

#define MAX_BGS_ID 3

#define BGS_GRIMSON_GMM 0
#define BGS_ZIVKOVIC_GMM 1
#define BGS_ADAPTIVE_MEDIAN 2
#define BGS_RUNNING_GAUSSIAN 3
#define BGS_EIGEN_BKG 4

#define MAX_NAME 100

using namespace std;
using namespace cv;
using namespace Algorithms::BackgroundSubtraction;

struct gmm_struct{

	int max_std_10;
	int alpha_1000;

	double max_std;
	double alpha;
	int max_modes;
};

struct running_gaussian_struct{

	int max_std_10;
	int alpha_1000;

	double max_std;
	double alpha;
	int learning_frames;
};

struct adaptive_median_struct{

	int low_threshold;
	int sampling_rate;
	int learning_frames;
};

struct bgs_toggle_struct{

	int bgs_method;
	int frg_thr_percent;
	double frg_thr;
	int max_frg_thr_count;
};

struct bgs_struct{

	gmm_struct *grimson_params_local;
	gmm_struct *zivkovic_params_local;
	adaptive_median_struct *median_params_local;
	running_gaussian_struct *rg_params_local;

	bgs_toggle_struct *bgs_toggle_params;
};



class BgsModels{
public:

	BgsModels(CvSize frame_size,bgs_struct *bgs_params_init);
	~BgsModels();
	void clearBGSModels();

	void initBGSToggleParams(bgs_toggle_struct *bgs_toggle_params_init);
	void initBGSParams(bgs_struct *bgs_params_init);
	void initStateVariables(CvSize frame_size);	
	void initBGSModels();
	void initBGSMethod();	
	void initWindow(char *window_name);
	static void updateBGSMethod(int);
	static void updateBGSToggleParams(int);
	static void printBGSTogglesParams();

	void initGrimsonGMMParams(gmm_struct *bgs_grimson_params_init);
	static void updateGrimsonGMMParamsLocal(int);
	static void updateGrimsonGMMParamsGlobal();
	static void printGrimsonGMMParams();

	void initZivkovicGMMParams(gmm_struct *bgs_zivkovic_params_init);
	static void updateZivkovicGMMParamsLocal(int);
	static void updateZivkovicGMMParamsGlobal();
	static void printZivkovicGMMParams();

	void initAdaptiveMedianParams(adaptive_median_struct *bgs_median_params_init);
	static void updateAdaptiveMedianParamsLocal(int);
	static void updateAdaptiveMedianParamsGlobal();
	static void printAdaptiveMedianParams();

	void initRunningGaussianParams(running_gaussian_struct *bgs_rg_params_init);
	static void updateRunningGaussianParamsLocal(int);
	static void updateRunningGaussianParamsGlobal();
	static void printRunningGaussianParams();

	static void showBGSHelpWindow(int mouse_event,int x,int y,int flags,void* param);

	BgsParams *params;
	Bgs *bgs;

	static GrimsonParams *grimson_params;
	static GrimsonGMM *grimson_bgs;	

	static ZivkovicParams *zivkovic_params;
	static ZivkovicAGMM *zivkovic_bgs;
	
	static AdaptiveMedianParams *median_params;
	static AdaptiveMedianBGS *median_bgs;

	static WrenParams *rg_params;
	static WrenGA *rg_bgs;	

	static bgs_struct *bgs_params;
	
	static bool bgs_method_changed;

	static char *bgs_help_window;
	static IplImage *bgs_help_image;
};

#endif