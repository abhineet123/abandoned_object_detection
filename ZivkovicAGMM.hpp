/****************************************************************************
Implementation of the Gaussian mixture model (GMM) background
		 subtraction algorithm developed by Z. Zivkovic.
******************************************************************************/

#ifndef ZIVKOVIC_AGMM_H
#define ZIVKOVIC_AGMM_H

#include "Bgs.hpp"

namespace Algorithms {
namespace BackgroundSubtraction {

// --- User adjustable parameters used by the Grimson GMM BGS algorithm ---
class ZivkovicParams : public BgsParams {
public:
    float &LowThreshold() {
        return m_low_threshold;
    }
    float &HighThreshold() {
        return m_high_threshold;
    }

    float &Alpha() {
        return m_alpha;
    }
    int &MaxModes() {
        return m_max_modes;
    }

private:
    // Threshold on the squared dist. to decide when a sample is close to an existing
    // components. If it is not close to any, a new component will be generated.
    // Smaller threshold values lead to more generated components and higher threshold values
    // lead to a small number of components but they can grow too large.
    //
    // It is usual easiest to think of these thresholds as being the number of variances (not standard deviations)
    // away from the mean of a pixel before it is considered to be from the foreground.
    float m_low_threshold;
    float m_high_threshold;

    // alpha - speed of update - if the time interval you want to average over is T
    // set alpha=1/T.
    float m_alpha;

    // Maximum number of modes (Gaussian components) that will be used per pixel
    int m_max_modes;
};

// --- Zivkovic AGMM BGS algorithm ---
class ZivkovicAGMM : public Bgs {
private:
    struct GMM {
        float sigma;
        float muR;
        float muG;
        float muB;
        float weight;
    };

public:
    ZivkovicAGMM();
    ~ZivkovicAGMM();

    void Initalize(const BgsParams& param);

	void UpdateParams(const BgsParams& param);

	void PrintParams();

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

    RgbImage* Background() {
        return &m_background;
    }

private:
    void SubtractPixel(long posPixel, const RgbPixel& pixel, unsigned char* pModesUsed,
                       unsigned char& lowThreshold, unsigned char& highThreshold);
    void SubtractPixel2(long posPixel, const RgbPixel& pixel, unsigned char* pModesUsed,
                        unsigned char& lowThreshold, unsigned char& highThreshold,bool update_model);

	void copyModel(ZivkovicAGMM::GMM *src,ZivkovicAGMM::GMM *dst,int no_of_elements);

    // User adjustable parameters
    ZivkovicParams m_params;

    // Threshold when the component becomes significant enough to be included into
    // the background model. It is the TB = 1-cf from the paper. So I use cf=0.1 => TB=0.9
    // For alpha=0.001 it means that the mode should exist for approximately 105 frames before
    // it is considered foreground
    float m_bg_threshold; //1-cf from the paper

    // Initial variance for the newly generated components.
    // It will will influence the speed of adaptation. A good guess should be made.
    // A simple way is to estimate the typical standard deviation from the images.
    float m_variance;

    // This is related to the number of samples needed to accept that a component
    // actually exists.
    float m_complexity_prior;

    //data
    int m_num_bands;	//only RGB now ==3

    // dynamic array for the mixture of Gaussians
    GMM* m_modes;

	GMM* m_modes_backup;

    RgbImage m_background;

    //number of Gaussian components per pixel
    unsigned char* m_modes_per_pixel;
};

};
};

#endif






