#include "systemParameters.hpp"

SystemParameters::SystemParameters(){

	ui_proc_params_init=new proc_struct;
	ui_io_params_init=new io_struct;

	pre_process_params_init=new pre_process_struct;

	bgs_params_init=new bgs_struct;	

	frg_params_init=new frg_struct;
	frg_morph_params_init=new morph_struct;

	blob_detect_params_init=new blob_detection_struct;

	blob_matching_params_init=new match_struct;
	blob_tracking_params_init=new track_struct;

	abandonment_params_init=new abandonment_struct;

	filter_params_init=new filter_struct;

	input_buffer=new char[MAX_LINE_SIZE];
	param_str=new char[MAX_VAL_SIZE];
	buffer_length=0;
}

SystemParameters::~SystemParameters(){	

	delete(ui_proc_params_init);
	delete(ui_io_params_init);
	delete(pre_process_params_init);
	delete(bgs_params_init);
	delete(frg_params_init);
	delete(frg_morph_params_init);
	delete(blob_detect_params_init);
	delete(blob_matching_params_init);
	delete(blob_tracking_params_init);
	delete(abandonment_params_init);
	delete(filter_params_init);

	delete(input_buffer);
	delete(param_str);
}

int SystemParameters::getParamStartIndex(){
	
	bool found_comment=false;
	int i=0;
	for(i=0;i<buffer_length;i++){

		if(isspace(input_buffer[i]))
			continue;

		if(input_buffer[i]=='#'){
			if(!found_comment){
				found_comment=true;
				continue;
			}else
				break;
		}else if(found_comment)
			continue;
		else
			break;

	}
	return i+1;
}
double SystemParameters::getParamVal(ifstream &fin){

	bool obtained_valid_str=false;

	while((fin)&&(!obtained_valid_str)){

		fin.getline(input_buffer,MAX_LINE_SIZE);

		//cout<<"input_buffer="<<input_buffer<<"\n";

		buffer_length=strlen(input_buffer);
		if(buffer_length==0)
			continue;
		obtained_valid_str=getParamString(getParamStartIndex());
	}

	double param_val=atof(param_str);
	//cout<<"param_val="<<param_val<<"\n\n";

	return param_val;

}
bool SystemParameters::getParamString(int param_start_index ){

	int current_index=0;

	if(param_start_index>=buffer_length-1)
		return false;

	for(int i=param_start_index;i<buffer_length;i++){
		if((isspace(input_buffer[i]))||(isalpha(input_buffer[i])))
			continue;
		param_str[current_index]=input_buffer[i];
		current_index++;
	}
	param_str[current_index]='\0';
	//cout<<"param_str="<<param_str<<"\n";
	return true;

}

void SystemParameters::readInitialParams(char *input_file_name){

	cout<<"Reading initial values of system parameters from file "<<input_file_name<<"\n";

	ifstream fin(input_file_name,ios::in);
	if(!fin){
		cout<<"Parameter initialization file not found.\n";
		exit(0);
	}
	if(!readInitialUIParams(fin))
		cout<<"Could not read UI parameters\n";
	if(SHOW_STATUS_SYS)
	cout<<"Done reading UI parameters.\n";

	if(!readInitialPreProcessingParams(fin))
		cout<<"Could not read Pre Processing parameters\n";
	if(SHOW_STATUS_SYS)
		cout<<"Done reading Pre Processing parameters.\n";

	if(!readInitialBGSParams(fin))
		cout<<"Could not read BGS parameters\n";
	if(SHOW_STATUS_SYS)
		cout<<"Done reading BGS parameters.\n";

	if(!readInitialFrgParams(fin))
		cout<<"Could not read foreground analysis parameters\n";
	if(SHOW_STATUS_SYS)
		cout<<"Done reading Frg parameters.\n";

	if(!readInitialBlobDetectionParams(fin))
		cout<<"Could not read blob detection parameters\n";
	if(SHOW_STATUS_SYS)
		cout<<"Done reading blob detection parameters.\n";

	if(!readInitialBlobMatchingParams(fin))
		cout<<"Could not read blob matching parameters\n";
	if(SHOW_STATUS_SYS)
		cout<<"Done reading blob matching parameters.\n";

	if(!readInitialBlobTrackingParams(fin))
		cout<<"Could not read blob tracking parameters\n";
	if(SHOW_STATUS_SYS)
		cout<<"Done reading blob tracking parameters.\n";

	if(!readInitialAbandonmentParams(fin))
		cout<<"Could not read abandonment analysis parameters\n";
	if(SHOW_STATUS_SYS)
		cout<<"Done reading abandonment analysis parameters.\n";

	if(!readInitialBlobFilteringParams(fin))
		cout<<"Could not read blob filtering parameters\n";
	if(SHOW_STATUS_SYS)
		cout<<"Done reading abandonment analysis parameters.\n";

	if(SHOW_STATUS_SYS)
		cout<<"Done reading parameters.\n";

	//_getch();
}

bool SystemParameters::readInitialUIParams(ifstream &fin){		

	//********  1   ********//
	ui_io_params_init->capture_from_camera=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"capture_from_camera="<<ui_io_params_init->capture_from_camera<<"\n";

	//********  2   ********//
	ui_io_params_init->video_file_source=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"video_file_source="<<ui_io_params_init->video_file_source<<"\n";

	//********  3   ********//
	ui_io_params_init->set_id=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"set_id="<<ui_io_params_init->set_id<<"\n";

	//********  4   ********//
	ui_io_params_init->view_id=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"view_id="<<ui_io_params_init->view_id<<"\n";

	//********  5   ********//
	ui_io_params_init->size_id=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"size_id="<<ui_io_params_init->size_id<<"\n";

	//********  6   ********//
	ui_io_params_init->show_all_blobs=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"show_all_blobs="<<ui_io_params_init->show_all_blobs<<"\n";

	//********  7   ********//
	ui_proc_params_init->change_proc_resolution=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"change_proc_resolution="<<ui_proc_params_init->change_proc_resolution<<"\n";

	//********  8   ********//
	ui_proc_params_init->proc_resize_factor_10=(int)(getParamVal(fin)*10);
	if(SHOW_STATUS_SYS)
		cout<<"proc_resize_factor_10="<<ui_proc_params_init->proc_resize_factor_10<<"\n";

	//********  9   ********//
	ui_proc_params_init->change_disp_resolution=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"change_disp_resolution="<<ui_proc_params_init->change_disp_resolution<<"\n";

	//********  10   ********//
	ui_proc_params_init->disp_resize_factor_10=(int)(getParamVal(fin)*10);
	if(SHOW_STATUS_SYS)
		cout<<"disp_resize_factor_10="<<ui_proc_params_init->disp_resize_factor_10<<"\n";

	return true;
}

bool SystemParameters::readInitialPreProcessingParams(ifstream &fin){
	
	//********  1   ********//
	pre_process_params_init->contrast_enhancement_method=(int)getParamVal(fin);
	//********  2   ********//
	pre_process_params_init->noise_reduction_method=(int)getParamVal(fin);
	//********  3   ********//
	pre_process_params_init->show_histogram=(int)getParamVal(fin);
	//********  4   ********//
	pre_process_params_init->kernel_width=(int)getParamVal(fin);
	//********  5   ********//
	pre_process_params_init->kernel_height=(int)getParamVal(fin);
	//********  6   ********//
	pre_process_params_init->use_square_kernel=(int)getParamVal(fin);
	//********  7   ********//
	pre_process_params_init->cutoff_percent=(int)getParamVal(fin);	

	if(pre_process_params_init->kernel_width>0)
		pre_process_params_init->kernel_width_id=(int)((double)(pre_process_params_init->kernel_width-1)/2.0);
	else
		pre_process_params_init->kernel_width_id=pre_process_params_init->kernel_width;

	if(pre_process_params_init->kernel_height>0)
		pre_process_params_init->kernel_height_id=(int)((double)(pre_process_params_init->kernel_height-1)/2.0);
	else
		pre_process_params_init->kernel_height_id=pre_process_params_init->kernel_height;	

	return true;
}

bool SystemParameters::readInitialBGSParams(ifstream &fin){		

	bgs_params_init->bgs_toggle_params=new bgs_toggle_struct;
	//********  1   ********//
	bgs_params_init->bgs_toggle_params->bgs_method=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"bgs_method="<<bgs_params_init->bgs_toggle_params->bgs_method<<"\n";


	bgs_params_init->grimson_params_local=new gmm_struct;
	//********  2   ********//
	bgs_params_init->grimson_params_local->max_std_10=(int)(getParamVal(fin)*10);
	if(SHOW_STATUS_SYS)
		cout<<"max_std_10="<<bgs_params_init->grimson_params_local->max_std_10<<"\n";
	//********  3   ********//
	bgs_params_init->grimson_params_local->alpha_1000=(int)(getParamVal(fin)*1000);
	if(SHOW_STATUS_SYS)
		cout<<"alpha_1000="<<bgs_params_init->grimson_params_local->alpha_1000<<"\n";
	//********  4   ********//
	bgs_params_init->grimson_params_local->max_modes=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"max_modes="<<bgs_params_init->grimson_params_local->max_modes<<"\n";


	bgs_params_init->zivkovic_params_local=new gmm_struct;
	//********  5   ********//
	bgs_params_init->zivkovic_params_local->max_std_10=(int)(getParamVal(fin)*10);
	if(SHOW_STATUS_SYS)
		cout<<"max_std_10="<<bgs_params_init->grimson_params_local->max_std_10<<"\n";
	//********  6   ********//
	bgs_params_init->zivkovic_params_local->alpha_1000=(int)(getParamVal(fin)*1000);
	if(SHOW_STATUS_SYS)
		cout<<"alpha_1000="<<bgs_params_init->grimson_params_local->alpha_1000<<"\n";
	//********  7   ********//
	bgs_params_init->zivkovic_params_local->max_modes=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"max_modes="<<bgs_params_init->grimson_params_local->max_modes<<"\n";


	bgs_params_init->median_params_local=new adaptive_median_struct;
	//********  8   ********//
	bgs_params_init->median_params_local->low_threshold=(int)getParamVal(fin);	
	if(SHOW_STATUS_SYS)
		cout<<"low_threshold="<<bgs_params_init->median_params_local->low_threshold<<"\n";
	//********  9   ********//
	bgs_params_init->median_params_local->sampling_rate=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"sampling_rate="<<bgs_params_init->median_params_local->sampling_rate<<"\n";
	//********  10   ********//
	bgs_params_init->median_params_local->learning_frames=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"learning_frames="<<bgs_params_init->median_params_local->learning_frames<<"\n";


	bgs_params_init->rg_params_local=new running_gaussian_struct;
	//********  11   ********//
	bgs_params_init->rg_params_local->max_std_10=(int)(getParamVal(fin)*10);
	if(SHOW_STATUS_SYS)
		cout<<"max_std_10="<<bgs_params_init->rg_params_local->max_std_10<<"\n";
	//********  12   ********//
	bgs_params_init->rg_params_local->alpha_1000=(int)(getParamVal(fin)*1000);
	if(SHOW_STATUS_SYS)
		cout<<"alpha_1000="<<bgs_params_init->rg_params_local->alpha_1000<<"\n";
	//********  13   ********//
	bgs_params_init->rg_params_local->learning_frames=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"learning_frames="<<bgs_params_init->rg_params_local->learning_frames<<"\n";

	//********  14   ********//
	bgs_params_init->bgs_toggle_params->frg_thr_percent=(int)(getParamVal(fin)*100);
	if(SHOW_STATUS_SYS)
		cout<<"frg_thr_percent="<<bgs_params_init->bgs_toggle_params->frg_thr_percent<<"\n";
	//********  15   ********//
		bgs_params_init->bgs_toggle_params->max_frg_thr_count=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"max_frg_thr_count="<<bgs_params_init->bgs_toggle_params->max_frg_thr_count<<"\n";

	return true;
}

bool SystemParameters::readInitialFrgParams(ifstream &fin){		

	//********  1   ********//
	frg_params_init->perform_foreground_analysis=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"perform_foreground_analysis="<<frg_params_init->perform_foreground_analysis<<"\n";

	//********  2   ********//
	frg_params_init->shadow_detection_method=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"shadow_detection_method="<<frg_params_init->shadow_detection_method<<"\n";

	//********  3   ********//
	frg_params_init->shadow_refinement=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"shadow_refinement="<<frg_params_init->shadow_refinement<<"\n";

	//********  4   ********//
	frg_params_init->frg_similarity_threshold_percent=(int)(getParamVal(fin)*100);
	if(SHOW_STATUS_SYS)
		cout<<"shadow_refinement="<<frg_params_init->shadow_refinement<<"\n";

	//********  5   ********//
	frg_params_init->simple_ncc_threshold_percent=(int)(getParamVal(fin)*100);
	if(SHOW_STATUS_SYS)
		cout<<"simple_ncc_threshold_percent="<<frg_params_init->simple_ncc_threshold_percent<<"\n";
	
	//********  6   ********//
	frg_params_init->complex_ncc_threshold_percent=(int)(getParamVal(fin)*100);
	if(SHOW_STATUS_SYS)
		cout<<"complex_ncc_threshold_percent="<<frg_params_init->complex_ncc_threshold_percent<<"\n";	

	//********  7   ********//
	frg_params_init->min_intensity_ratio_percent=(int)(getParamVal(fin)*100);
	if(SHOW_STATUS_SYS)
		cout<<"min_intensity_ratio_percent="<<frg_params_init->min_intensity_ratio_percent<<"\n";

	//********  8   ********//
	frg_params_init->intensity_ratio_std_threshold_percent=(int)(getParamVal(fin)*100);	
	if(SHOW_STATUS_SYS)
		cout<<"intensity_ratio_std_threshold_percent="<<frg_params_init->intensity_ratio_std_threshold_percent<<"\n";

	//********  9   ********//
	frg_params_init->min_frame_intensity=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"min_frame_intensity="<<frg_params_init->min_frame_intensity<<"\n";

	//********  10   ********//
	frg_morph_params_init->performMorphologicalOp=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"performMorphologicalOp="<<frg_morph_params_init->performMorphologicalOp<<"\n";

	//********  11   ********//
	frg_morph_params_init->performClosing=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"performClosing="<<frg_morph_params_init->performClosing<<"\n";

	//********  12   ********//
	frg_morph_params_init->performOpening=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"performOpening="<<frg_morph_params_init->performOpening<<"\n";

	//********  13   ********//
	frg_morph_params_init->no_of_iterations=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"no_of_iterations="<<frg_morph_params_init->no_of_iterations<<"\n";

	return true;
}


bool SystemParameters::readInitialBlobDetectionParams(ifstream &fin){		

	//********  1   ********//
	blob_detect_params_init->min_blob_size=(int)getParamVal(fin);

	return true;

}

bool SystemParameters::readInitialBlobMatchingParams(ifstream &fin){

	//********  1   ********//
	blob_matching_params_init->occ_threshold_percent=(int)(getParamVal(fin)*100);
	if(SHOW_STATUS_SYS)
		cout<<"occ_threshold_percent="<<blob_matching_params_init->occ_threshold_percent<<"\n";

	//********  2   ********//
	blob_matching_params_init->size_threshold_percent=(int)(getParamVal(fin)*100);
	if(SHOW_STATUS_SYS)
		cout<<"size_threshold_percent="<<blob_matching_params_init->size_threshold_percent<<"\n";

	//********  3   ********//
	blob_matching_params_init->dist_threshold_percent=(int)(getParamVal(fin)*100);
	if(SHOW_STATUS_SYS)
		cout<<"dist_threshold_percent="<<blob_matching_params_init->dist_threshold_percent<<"\n";

	//********  4   ********//
	blob_matching_params_init->appearance_threshold_percent=(int)(getParamVal(fin)*100);
	if(SHOW_STATUS_SYS)
		cout<<"appearance_threshold_percent="<<blob_matching_params_init->appearance_threshold_percent<<"\n";

	//********  5   ********//
	blob_matching_params_init->moving_avg_alpha_percent=(int)(getParamVal(fin)*100);
	if(SHOW_STATUS_SYS)
		cout<<"moving_avg_alpha_percent="<<blob_matching_params_init->moving_avg_alpha_percent<<"\n";

	//********  6   ********//
	blob_matching_params_init->blob_distance_measure=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"blob_distance_measure="<<blob_matching_params_init->blob_distance_measure<<"\n";

	//********  7   ********//
	blob_matching_params_init->blob_area_measure=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"blob_area_measure="<<blob_matching_params_init->blob_area_measure<<"\n";

	//********  8   ********//
	blob_matching_params_init->blob_area_type=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"blob_area_type="<<blob_matching_params_init->blob_area_type<<"\n";

	//********  9   ********//
	blob_matching_params_init->use_gradient_diff=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"use_gradient_diff="<<blob_matching_params_init->use_gradient_diff<<"\n";

	return true;

}

bool SystemParameters::readInitialBlobTrackingParams(ifstream &fin){
	

	//********  1   ********//
	blob_tracking_params_init->min_hit_count_for_abandoned=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"min_hit_count_for_abandoned="<<blob_tracking_params_init->min_hit_count_for_abandoned<<"\n";
	
	//********  2   ********//
	blob_tracking_params_init->min_hit_count_for_occ=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"min_hit_count_for_occ="<<blob_tracking_params_init->min_hit_count_for_occ<<"\n";
	
	//********  3   ********//
	blob_tracking_params_init->min_hit_count_for_static=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"min_hit_count_for_static="<<blob_tracking_params_init->min_hit_count_for_static<<"\n";
	
	//********  4   ********//
	blob_tracking_params_init->max_miss_count=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"max_miss_count="<<blob_tracking_params_init->max_miss_count<<"\n";

	//********  5   ********//
	blob_tracking_params_init->max_occ_count=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"max_occ_count="<<blob_tracking_params_init->max_occ_count<<"\n";
	
	//********  6   ********//
	blob_tracking_params_init->max_mean_diff_10=(int)(getParamVal(fin)*10);
	if(SHOW_STATUS_SYS)
		cout<<"max_mean_diff_10="<<blob_tracking_params_init->max_mean_diff_10<<"\n";

	//********  7   ********//
	blob_tracking_params_init->max_removed_count=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"max_removed_count="<<blob_tracking_params_init->max_removed_count<<"\n";

	//********  8   ********//
	blob_tracking_params_init->max_abandoned_count=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"max_abandoned_count="<<blob_tracking_params_init->max_abandoned_count<<"\n";

	//********  9   ********//
	blob_tracking_params_init->static_factor_occ=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"static_factor_occ="<<blob_tracking_params_init->static_factor_occ<<"\n";

	//********  10   ********//
	blob_tracking_params_init->static_factor_miss=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"static_factor_miss="<<blob_tracking_params_init->static_factor_miss<<"\n";

	return true;

}


bool SystemParameters::readInitialAbandonmentParams(ifstream &fin){	

	//********  1   ********//
	abandonment_params_init->abandonment_method=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"abandonment_method="<<abandonment_params_init->abandonment_method<<"\n";

	//********  2   ********//
	abandonment_params_init->similarity_method=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"similarity_method="<<abandonment_params_init->similarity_method<<"\n";

	//********  3   ********//
	abandonment_params_init->erosion_width=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"erosion_width="<<abandonment_params_init->erosion_width<<"\n";
	
	//********  4   ********//
	abandonment_params_init->edge_diff_threshold_percent=(int)(getParamVal(fin)*100);
	if(SHOW_STATUS_SYS)
		cout<<"edge_diff_threshold_percent="<<abandonment_params_init->edge_diff_threshold_percent<<"\n";

	//********  5   ********//
	abandonment_params_init->region_growing_threshold_percent=(int)(getParamVal(fin)*100);
	if(SHOW_STATUS_SYS)
		cout<<"region_growing_threshold_percent="<<abandonment_params_init->region_growing_threshold_percent<<"\n";

	//********  6   ********//
	abandonment_params_init->region_diff_threshold_percent=(int)(getParamVal(fin)*100);	
	if(SHOW_STATUS_SYS)
		cout<<"region_diff_threshold_percent="<<abandonment_params_init->region_diff_threshold_percent<<"\n";

	//********  7   ********//
	abandonment_params_init->max_squared_diff=(int)(getParamVal(fin));
	if(SHOW_STATUS_SYS)
		cout<<"max_squared_diff="<<abandonment_params_init->max_squared_diff<<"\n";

	//********  8   ********//
	abandonment_params_init->detect_still_person=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"detect_still_person="<<abandonment_params_init->detect_still_person<<"\n";

	//********  9   ********//
	abandonment_params_init->max_avg_diff_10=(int)(getParamVal(fin)*10);
	if(SHOW_STATUS_SYS)
		cout<<"max_avg_diff_10="<<abandonment_params_init->max_avg_diff_10<<"\n";

	//********  10   ********//
	abandonment_params_init->canny_low_thr=(int)(getParamVal(fin));
	if(SHOW_STATUS_SYS)
		cout<<"canny_low_thr="<<abandonment_params_init->canny_low_thr<<"\n";

	//********  11   ********//
	abandonment_params_init->canny_ratio=(int)(getParamVal(fin));
	if(SHOW_STATUS_SYS)
		cout<<"canny_ratio="<<abandonment_params_init->canny_ratio<<"\n";

	//********  12   ********//
	abandonment_params_init->gradient_min_thr=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"gradient_min_thr="<<abandonment_params_init->gradient_min_thr<<"\n";

	return true;

}

bool SystemParameters::readInitialBlobFilteringParams(ifstream &fin){

	//********  1   ********//
	filter_params_init->enable_filtering=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"enable_filtering="<<filter_params_init->enable_filtering<<"\n";

	//********  2   ********//
	filter_params_init->size_thr_percent=(int)(getParamVal(fin)*100);
	if(SHOW_STATUS_SYS)
		cout<<"size_thr_percent="<<filter_params_init->size_thr_percent<<"\n";

	//********  3   ********//
	filter_params_init->dist_thr_percent=(int)(getParamVal(fin)*100);
	if(SHOW_STATUS_SYS)
		cout<<"dist_thr_percent="<<filter_params_init->dist_thr_percent<<"\n";

	//********  4   ********//
	filter_params_init->appearance_thr_percent=(int)(getParamVal(fin)*100);
	if(SHOW_STATUS_SYS)
		cout<<"appearance_thr_percent="<<filter_params_init->appearance_thr_percent<<"\n";	

	//********  5   ********//
	filter_params_init->dist_method=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"blob_distance_measure="<<filter_params_init->dist_method<<"\n";

	//********  6   ********//
	filter_params_init->match_location=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"match_location="<<filter_params_init->match_location<<"\n";

	//********  7   ********//
	filter_params_init->match_size=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"match_size="<<filter_params_init->match_size<<"\n";

	//********  8   ********//
	filter_params_init->match_appearance=(int)getParamVal(fin);
	if(SHOW_STATUS_SYS)
		cout<<"match_appearance="<<filter_params_init->match_appearance<<"\n";

	return true;
}














