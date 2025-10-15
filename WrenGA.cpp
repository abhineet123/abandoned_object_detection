/****************************************************************************
*
* WrenGA.h
*
* Purpose: Implementation of the running Gaussian average background
*		  		 subtraction algorithm described in:
*	  			 "Pfinder: real-time tracking of the human body"
* 						by C. Wren et al (1997)
*
* Author: Donovan Parks, September 2007
*
* Please note that this is not an implementation of Pfinder. It implements
* a simple background subtraction algorithm where each pixel is represented
* by a single Gaussian and update using a simple weighting function.
******************************************************************************/

#include "WrenGA.hpp"

using namespace Algorithms::BackgroundSubtraction;

WrenGA::WrenGA() {
    m_gaussian = NULL;
}

WrenGA::~WrenGA() {
    if(m_gaussian != NULL)
        delete[] m_gaussian;
}

void WrenGA::Initalize(const BgsParams& param) {
    m_params = (WrenParams&)param;

    m_variance = 36.0f;

    // GMM for each pixel
    m_gaussian = new GAUSSIAN[m_params.Size()];
    for(unsigned int i = 0; i < m_params.Size(); ++i) {
        for(int ch = 0; ch < NUM_CHANNELS; ++ch) {
            m_gaussian[i].mu[ch] = 0;
            m_gaussian[i].var[ch] = 0;
        }
    }

    m_background = cvCreateImage(cvSize(m_params.Width(), m_params.Height()), IPL_DEPTH_8U, 3);
}

void WrenGA::InitModel(const RgbImage& data) {
    int pos = 0;
	frame_count=0;

    for(unsigned int r = 0; r < m_params.Height(); ++r) {
        for(unsigned int c = 0; c < m_params.Width(); ++c) {
            for(int ch = 0; ch < NUM_CHANNELS; ++ch) {
                m_gaussian[pos].mu[ch] = data(r,c,ch);
                m_gaussian[pos].var[ch] = m_variance;
            }

            pos++;
        }
    }
}

void WrenGA::UpdateParams(const BgsParams& param) {
    m_params = (WrenParams&)param;
}

void WrenGA::PrintParams() {
    std::cout<<"\n";
    std::cout<<"low threshold="<<m_params.LowThreshold()<<"\t";
    std::cout<<"high threshold="<<m_params.HighThreshold()<<"\t";
    std::cout<<"alpha="<<m_params.Alpha()<<"\t";
    std::cout<<"learning frames="<<m_params.LearningFrames()<<"\n";
}

void WrenGA::Update(const RgbImage& data,  const BwImage& update_mask) {
    int pos = 0;

    for(unsigned int r = 0; r < m_params.Height(); ++r) {
        for(unsigned int c = 0; c < m_params.Width(); ++c) {
            // conditional updating is applicable only if the learning phase has been passed
            if(update_mask(r,c) == BACKGROUND || frame_count < m_params.LearningFrames()) {
                float dR = m_gaussian[pos].mu[0] - data(r,c,0);
                float dG = m_gaussian[pos].mu[1] - data(r,c,1);
                float dB = m_gaussian[pos].mu[2] - data(r,c,2);

                float dist = (dR*dR + dG*dG + dB*dB);

                m_gaussian[pos].mu[0] -= m_params.Alpha()*(dR);
                m_gaussian[pos].mu[1] -= m_params.Alpha()*(dG);
                m_gaussian[pos].mu[2] -= m_params.Alpha()*(dB);

                float sigmanew = m_gaussian[pos].var[0] + m_params.Alpha()*(dist-m_gaussian[pos].var[0]);
                m_gaussian[pos].var[0] = sigmanew < 4 ? 4 : sigmanew > 5*m_variance ? 5*m_variance : sigmanew;

                m_background(r, c, 0) = (unsigned char)(m_gaussian[pos].mu[0] + 0.5);
                m_background(r, c, 1) = (unsigned char)(m_gaussian[pos].mu[1] + 0.5);
                m_background(r, c, 2) = (unsigned char)(m_gaussian[pos].mu[2] + 0.5);
            }

            pos++;
        }
    }
}

void WrenGA::SubtractPixel(int r, int c, const RgbPixel& pixel,
                           unsigned char& low_threshold,
                           unsigned char& high_threshold) {
    unsigned int pos = r*m_params.Width()+c;

    // calculate distance between model and pixel
    float mu[NUM_CHANNELS];
    float var[1];
    float delta[NUM_CHANNELS];
    float dist = 0;
    for(int ch = 0; ch < NUM_CHANNELS; ++ch) {
        mu[ch] = m_gaussian[pos].mu[ch];
        var[0] = m_gaussian[pos].var[0];
        delta[ch] = mu[ch] - pixel(ch);
        dist += delta[ch]*delta[ch];
    }

    // calculate the squared distance and see if pixel fits the B/G model
    low_threshold = BACKGROUND;
    high_threshold = BACKGROUND;

    if(dist > m_params.LowThreshold()*var[0])
        low_threshold = FOREGROUND;
    if(dist > m_params.HighThreshold()*var[0])
        high_threshold = FOREGROUND;
}

///////////////////////////////////////////////////////////////////////////////
//Input:
//  data - a pointer to the data of a RGB image of the same size
//Output:
//  output - a pointer to the data of a gray value image of the same size
//					(the memory should already be reserved)
//					values: 255-foreground, 125-shadow, 0-background
///////////////////////////////////////////////////////////////////////////////
void WrenGA::Subtract(const RgbImage& data,
                      BwImage& low_threshold_mask, BwImage& high_threshold_mask) {
    unsigned char low_threshold, high_threshold;

	frame_count++;

    // update each pixel of the image
    for(unsigned int r = 0; r < m_params.Height(); ++r) {
        for(unsigned int c = 0; c < m_params.Width(); ++c) {
            SubtractPixel(r, c, data(r,c), low_threshold, high_threshold);
            low_threshold_mask(r,c) = low_threshold;
            high_threshold_mask(r,c) = high_threshold;
        }
    }
}

void WrenGA::Subtract2(const RgbImage& data,BwImage& low_threshold_mask,
                       BwImage& high_threshold_mask, BwImage& static_threshold_mask,BwImage& update_mask,double static_threshold) {

}

void WrenGA::Subtract2(const RgbImage& data,BwImage& low_threshold_mask,
                       BwImage& high_threshold_mask,BwImage& update_mask) {

}

void WrenGA::pushIntoBackground(const CvRect bounding_box,RgbImage &current_frame){

	CvPoint min_point,max_point;
	min_point.x=bounding_box.x;
	min_point.y=bounding_box.y;

	max_point.x=min_point.x+bounding_box.width;
	max_point.y=min_point.y+bounding_box.height;	


	for(int r=min_point.y;r<=max_point.y;r++){
		for(int c=min_point.x;c<=max_point.x;c++){

			long pixel_pos=r*m_params.Width()+c;
			for(int ch=0;ch<NUM_CHANNELS;ch++){	
				m_gaussian[pixel_pos].mu[ch]=m_background(r, c, ch)=current_frame(r,c,ch);			
			}
		}
	}
}

void WrenGA::resetBackground(RgbImage &current_frame,BwImage *reset_mask){

	int img_width=current_frame.width();
	int img_height=current_frame.height();

	if((img_width!=m_background.width())||(img_height!=m_background.height())){
		cout<<"Current frame dimensions do not match those of the background image.\n";
		_getch();
		return;
	}

	for(int r=0;r<img_height;r++){
		for(int c=0;c<img_width;c++){

			if((reset_mask!=NULL)&&((*reset_mask)(r,c)==FOREGROUND))
				continue;

			long pixel_pos=r*m_params.Width()+c;
			for(int ch=0;ch<NUM_CHANNELS;ch++){	
				m_gaussian[pixel_pos].mu[ch]=m_background(r, c, ch)=current_frame(r,c,ch);			
			}
		}
	}
}


