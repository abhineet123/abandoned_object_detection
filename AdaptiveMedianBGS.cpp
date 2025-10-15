#include <iostream>
#include <stdlib.h>
#include <cmath>
#include "AdaptiveMedianBGS.hpp"

using namespace Algorithms::BackgroundSubtraction;

void AdaptiveMedianBGS::Initalize(const BgsParams& param) {
    m_params = (AdaptiveMedianParams&)param;

    m_median = cvCreateImage(cvSize(m_params.Width(), m_params.Height()), IPL_DEPTH_8U, 3);
    cvSet(m_median.Ptr(), CV_RGB(BACKGROUND,BACKGROUND,BACKGROUND));
}

void AdaptiveMedianBGS::UpdateParams(const BgsParams& param) {
	m_params = (AdaptiveMedianParams&)param;	
}

void AdaptiveMedianBGS::PrintParams(){
	std::cout<<"\n";
	std::cout<<"low threshold="<<m_params.LowThreshold()<<"\t";
	std::cout<<"high threshold="<<m_params.HighThreshold()<<"\t";
	std::cout<<"sampling rate="<<m_params.SamplingRate()<<"\t";
	std::cout<<"learning frames="<<m_params.LearningFrames()<<"\n";
}

RgbImage* AdaptiveMedianBGS::Background() {
    return &m_median;
}

void AdaptiveMedianBGS::InitModel(const RgbImage& data) {
    // initialize the background model
	frame_count=0;
    for (unsigned int r = 0; r < m_params.Height(); ++r) {
        for(unsigned int c = 0; c < m_params.Width(); ++c) {
            m_median(r,c) = data(r,c);
        }
    }
}

void AdaptiveMedianBGS::Update(const RgbImage& data,  const BwImage& update_mask) {
	if(frame_count % m_params.SamplingRate() != 1) {
		return;
	}
	// update background model
	for (unsigned int r = 0; r < m_params.Height(); ++r) {
		for(unsigned int c = 0; c < m_params.Width(); ++c) {
			// perform conditional updating only if we are passed the learning phase
			if(update_mask(r,c) == BACKGROUND || frame_count < m_params.LearningFrames()) {
				for(int ch = 0; ch < NUM_CHANNELS; ++ch) {
					if(data(r,c,ch) > m_median(r,c,ch)) {
						m_median(r,c,ch)++;
					} else if(data(r,c,ch) < m_median(r,c,ch)) {
						m_median(r,c,ch)--;
					}
				}
			}
		}
	} 
}

void AdaptiveMedianBGS::SubtractPixel(int r, int c, const RgbPixel& pixel,
                                      unsigned char& low_threshold, unsigned char& high_threshold) {
    // perform background subtraction
    low_threshold = high_threshold = FOREGROUND;

    int diffR = abs(pixel(0) - m_median(r,c,0));
    int diffG = abs(pixel(1) - m_median(r,c,1));
    int diffB = abs(pixel(2) - m_median(r,c,2));

    if(diffR <= m_params.LowThreshold() && diffG <= m_params.LowThreshold() &&  diffB <= m_params.LowThreshold()) {
        low_threshold = BACKGROUND;
    }

    if(diffR <= m_params.HighThreshold() && diffG <= m_params.HighThreshold() &&  diffB <= m_params.HighThreshold()) {
        high_threshold = BACKGROUND;
    }
}

void AdaptiveMedianBGS::Subtract2(const RgbImage& data,BwImage& low_threshold_mask, 
							 BwImage& high_threshold_mask, BwImage& static_threshold_mask,BwImage& update_mask,double static_threshold){

}

void AdaptiveMedianBGS::Subtract2(const RgbImage& data,BwImage& low_threshold_mask,
								  BwImage& high_threshold_mask,BwImage& update_mask){

}

///////////////////////////////////////////////////////////////////////////////
//Input:
//  data - a pointer to the image data
//Output:
//  output - a pointer to the data of a gray value image
//					(the memory should already be reserved)
//					values: 255-foreground, 0-background
///////////////////////////////////////////////////////////////////////////////
void AdaptiveMedianBGS::Subtract(const RgbImage& data,
                                 BwImage& low_threshold_mask, BwImage& high_threshold_mask) {

    unsigned char low_threshold, high_threshold;

	frame_count++;

    // update each pixel of the image
    for(unsigned int r = 0; r < m_params.Height(); ++r) {
        for(unsigned int c = 0; c < m_params.Width(); ++c) {
            // perform background subtraction
            SubtractPixel(r, c, data(r,c), low_threshold, high_threshold);

            // setup silhouette mask
            low_threshold_mask(r,c) = low_threshold;
            high_threshold_mask(r,c) = high_threshold;
        }
    }
}

void AdaptiveMedianBGS::pushIntoBackground(const CvRect bounding_box,RgbImage &current_frame){

	CvPoint min_point,max_point;
	min_point.x=bounding_box.x;
	min_point.y=bounding_box.y;

	max_point.x=min_point.x+bounding_box.width;
	max_point.y=min_point.y+bounding_box.height;

	for(int r=min_point.y;r<=max_point.y;r++){
		for(int c=min_point.x;c<=max_point.x;c++){		
			m_median(r,c,0)=current_frame(r,c,0);
			m_median(r,c,1)=current_frame(r,c,1);
			m_median(r,c,2)=current_frame(r,c,2);
		}
	}

}

void AdaptiveMedianBGS::resetBackground(RgbImage &current_frame,BwImage *reset_mask){

	int img_width=current_frame.width();
	int img_height=current_frame.height();

	if((img_width!=m_median.width())||(img_height!=m_median.height())){
		cout<<"Current frame dimensions do not match those of the background image.\n";
		_getch();
		return;
	}

	for(int r=0;r<img_height;r++){
		for(int c=0;c<img_width;c++){

			if((reset_mask!=NULL)&&((*reset_mask)(r,c)==FOREGROUND))
				continue;

			m_median(r,c,0)=current_frame(r,c,0);
			m_median(r,c,1)=current_frame(r,c,1);
			m_median(r,c,2)=current_frame(r,c,2);
		}
	}
}

