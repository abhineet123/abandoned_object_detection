#ifndef _UPDATE_PARAMS_
#define _UPDATE_PARAMS_

#include "GrimsonGMM.hpp"
#include "ZivkovicAGMM.hpp"
#include "AdaptiveMedianBGS.hpp"
#include "Eigenbackground.hpp"
#include "WrenGA.hpp"

using namespace Algorithms::BackgroundSubtraction;


bool bgs_method_changed=false;
void updateBGSMethod(int);

int max_std_10,bgs_alpha_1000,max_modes;
double max_std,bgs_alpha;
bool bgs_updated;
void updateParamsGMM(int);


int median_low_threshold,median_sampling_rate,median_learning_frames;
void updateParamsAdaptiveMedian(int);


int perform_foreground_analysis;
double shadow_similarity_threshold,frg_similarity_threshold;
int shadow_similarity_threshold_percent,frg_similarity_threshold_percent,min_frame_intensity;
int min_intensity_ratio_percent,intensity_ratio_std_threshold_percent;
double min_intensity_ratio,intensity_ratio_std_threshold;
bool frg_updated;
void updateParamsForeground(int);


double static_threshold;
int static_threshold_percent;
void updateParamsStatic(int);


void updateParamsBlobs(int);


double occ_threshold,size_threshold,dist_threshold_frac,appearance_threshold;
int occ_threshold_percent,size_threshold_percent,dist_threshold_percent,appearance_threshold_percent;
bool match_updated;
void  updateParamsMatch(int);


bool track_updated;
void updateParamsTrack(int);


int region_similarity_percent;
double region_similarity_threshold;
void updateParamsAbandonment(int);


void updateGrimsonGMMParameters(GrimsonParams *params,GrimsonGMM *bgs);


void updateZivkovicGMMParameters(ZivkovicParams *params,ZivkovicAGMM *bgs);


void updateAdaptiveMedianParameters(AdaptiveMedianParams *params,AdaptiveMedianBGS *bgs);

#endif