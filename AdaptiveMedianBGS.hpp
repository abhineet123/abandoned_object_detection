/****************************************************************************
*
* AdaptiveMedianBGS.hpp
*
* Purpose: Implementation of the simple adaptive median background
*		  		 subtraction algorithm described in:
*	  			 "Segmentation and tracking of piglets in images"
* 						by McFarlane and Schofield
*
* Author: Donovan Parks, September 2007

Example:
		Algorithms::BackgroundSubtraction::AdaptiveMedianParams params;
		params.SetFrameSize(width, height);
		params.LowThreshold() = 40;
		params.HighThreshold() = 2*params.LowThreshold();
		params.SamplingRate() = 7;
		params.LearningFrames() = 30;

		Algorithms::BackgroundSubtraction::AdaptiveMedianBGS bgs;
		bgs.Initalize(params);
******************************************************************************/

#include "Bgs.hpp"

namespace Algorithms {
namespace BackgroundSubtraction {

// --- Parameters used by the Adaptive Median BGS algorithm ---
class AdaptiveMedianParams : public BgsParams {
public:
    unsigned char &LowThreshold() {
        return m_low_threshold;
    }
    unsigned char &HighThreshold() {
        return m_high_threshold;
    }

    int &SamplingRate() {
        return m_samplingRate;
    }
    int &LearningFrames() {
        return m_learning_frames;
    }

private:
    unsigned char m_low_threshold;
    unsigned char m_high_threshold;

    int m_samplingRate;
    int m_learning_frames;	
};


// --- Adaptive Median BGS algorithm ---
class AdaptiveMedianBGS : public Bgs {
public:
    virtual ~AdaptiveMedianBGS() {}

	void UpdateParams(const BgsParams& param);

	void PrintParams();

    void Initalize(const BgsParams& param);

    void InitModel(const RgbImage& data);

    void Subtract(const RgbImage& data,
		BwImage& low_threshold_mask, BwImage& high_threshold_mask);

	void Subtract2(const RgbImage& data,BwImage& low_threshold_mask, 
		BwImage& high_threshold_mask, BwImage& static_threshold_mask,BwImage& update_mask,double static_threshold);
	
	void Subtract2(const RgbImage& data,BwImage& low_threshold_mask,
		BwImage& high_threshold_mask,BwImage& update_mask);
	
	void Update(const RgbImage& data,  const BwImage& update_mask);

	void pushIntoBackground(const CvRect bounding_box,RgbImage &current_frame);
	void resetBackground(RgbImage &current_frame,BwImage *reset_mask);

    RgbImage* Background();

private:
    void SubtractPixel(int r, int c, const RgbPixel& pixel,
                       unsigned char& low_threshold, unsigned char& high_threshold);

    AdaptiveMedianParams m_params;

	int frame_count;

    RgbImage m_median;
};

};
};
