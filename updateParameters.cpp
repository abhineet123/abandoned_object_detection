#include "updateParameters.hpp"


void updateBGSMethod(int) {
	bgs_method_changed=true;
}

void updateParamsGMM(int) {

	max_std=(double)(max_std_10)/10.0;
	bgs_alpha=(double)bgs_alpha_1000/1000.0;
	bgs_updated=true;
}

void updateParamsAdaptiveMedian(int) {
	bgs_updated=true;
}


void updateParamsForeground(int) {

	frg_similarity_threshold=(double)(frg_similarity_threshold_percent)/100.0;
	shadow_similarity_threshold=(double)(shadow_similarity_threshold_percent)/100.0;
	min_intensity_ratio=(double)(min_intensity_ratio_percent)/100.0;
	intensity_ratio_std_threshold=(double)(intensity_ratio_std_threshold_percent)/100.0;
	frg_updated=true;

}

void updateParamsStatic(int) {
	static_threshold=(double)(static_threshold_percent)/100.0;
}


void updateParamsBlobs(int) {	
}

void  updateParamsMatch(int) {

	occ_threshold=(double)(occ_threshold_percent)/100.0;
	size_threshold=(double)(size_threshold_percent)/100.0;
	dist_threshold_frac=(double)(dist_threshold_percent)/100.0;
	appearance_threshold=(double)(appearance_threshold_percent)/100.0;
	match_updated=true;


}

void updateParamsTrack(int) {
	track_updated=true;
}


void updateParamsAbandonment(int) {
	region_similarity_threshold=(double)region_similarity_percent/100.0;
}


void updateGrimsonGMMParameters(GrimsonParams *params,GrimsonGMM *bgs){

	params->LowThreshold() = max_std*max_std;
	params->HighThreshold() = 2*params->LowThreshold();
	params->Alpha() = bgs_alpha;
	params->MaxModes() = max_modes;	
	bgs->UpdateParams(*params);
}

void updateZivkovicGMMParameters(ZivkovicParams *params,ZivkovicAGMM *bgs){

	params->LowThreshold() = max_std*max_std;
	params->HighThreshold() = 2*params->LowThreshold();
	params->Alpha() = bgs_alpha;
	params->MaxModes() = max_modes;	
	bgs->UpdateParams(*params);
}


void updateAdaptiveMedianParameters(AdaptiveMedianParams *params,AdaptiveMedianBGS *bgs){

	params->LowThreshold() = median_low_threshold;
	params->HighThreshold() = 2*params->LowThreshold();
	params->SamplingRate() = median_sampling_rate;
	params->LearningFrames() = median_learning_frames;

	bgs->UpdateParams(*params);
}