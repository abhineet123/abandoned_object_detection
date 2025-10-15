/****************************************************************************
Implementation of the Gaussian mixture model (GMM) background subtraction described in:
  			 "Adaptive background mixture models for real-time tracking"
						by Chris Stauffer and W.E.L Grimson
******************************************************************************/

#include "GrimsonGMM.hpp"

using namespace Algorithms::BackgroundSubtraction;

int compareGMM(const void* _gmm1, const void* _gmm2) {
    GMM gmm1 = *(GMM*)_gmm1;
    GMM gmm2 = *(GMM*)_gmm2;

    if(gmm1.significants < gmm2.significants)
        return 1;
    else if(gmm1.significants == gmm2.significants)
        return 0;
    else
        return -1;
}

GrimsonGMM::GrimsonGMM() {
    m_modes = NULL;
}

GrimsonGMM::~GrimsonGMM() {
    if(m_modes != NULL)
        delete[] m_modes;
}

void GrimsonGMM::Initalize(const BgsParams& param) {


    m_params = (GrimsonParams&)param;
    
    // Tbf - the threshold
    m_bg_threshold = 0.75f;	// 1-cf from the paper

    // Tgenerate - the threshold
    m_variance = 36.0f;		// sigma for the new mode

    // GMM for each pixel
    m_modes = new GMM[m_params.Size()*m_params.MaxModes()];

    // used modes per pixel
    m_modes_per_pixel = cvCreateImage(cvSize(m_params.Width(), m_params.Height()), IPL_DEPTH_8U, 1);

    m_background = cvCreateImage(cvSize(m_params.Width(), m_params.Height()), IPL_DEPTH_8U, 3);
	//background_image= cvCreateImage(cvSize(m_params.Width(), m_params.Height()), IPL_DEPTH_8U, 3);
}

void GrimsonGMM::UpdateParams(const BgsParams& param) {
	m_params = (GrimsonParams&)param;	
}

void GrimsonGMM::PrintParams(){
	std::cout<<"low threshold="<<m_params.LowThreshold()<<"\t";
	std::cout<<"high threshold="<<m_params.HighThreshold()<<"\t";
	std::cout<<"alpha="<<m_params.Alpha()<<"\t";
	std::cout<<"max modes="<<m_params.MaxModes()<<"\n";
}


RgbImage* GrimsonGMM::Background() {
    return &m_background;
}

void GrimsonGMM::InitModel(const RgbImage& data) {
    m_modes_per_pixel.Clear();

    for(unsigned int i = 0; i < m_params.Size()*m_params.MaxModes(); ++i) {
        m_modes[i].weight = 0;
        m_modes[i].variance = 0;
        m_modes[i].muR = 0;
        m_modes[i].muG = 0;
        m_modes[i].muB = 0;
        m_modes[i].significants = 0;
    }
}

void GrimsonGMM::Subtract2(const RgbImage& data,  
			   BwImage& low_threshold_mask, BwImage& high_threshold_mask,BwImage& update_mask){
				   ;

}

void GrimsonGMM::SubtractPixel2(long posPixel, const RgbPixel& pixel, unsigned char& numModes,
								unsigned char& low_threshold, unsigned char& high_threshold, 
								unsigned char& static_mask_value,double static_threshold,bool update_model) {
    long pos;
    bool bFitsPDF=false;
    bool bBackgroundLow=false;
    bool bBackgroundHigh=false;
    float max_var_factor=5.0f;
    float min_variance=4.0f, max_variance=max_var_factor*m_variance;
    float min_weight=0.0f;
    float fOneMinAlpha = 1-m_params.Alpha();
    float total_weight = 0.0f;    
    int backgroundGaussians = 1;  

    // update all distributions and check for match with current pixel
	for (int iModes=0; iModes < numModes; iModes++) {

		pos=posPixel+iModes;
		float weight = m_modes[pos].weight;

		// fit not found yet
		if (!bFitsPDF) {
			//check if it belongs to some of the modes
			//calculate distance
			float var = m_modes[pos].variance;
			float muR = m_modes[pos].muR;
			float muG = m_modes[pos].muG;
			float muB = m_modes[pos].muB;


			float dR=muR - pixel(0);
			float dG=muG - pixel(1);
			float dB=muB - pixel(2);

			// calculate the squared distance
			float dist = (dR*dR + dG*dG + dB*dB);

			if(dist < m_params.HighThreshold()*var && iModes < backgroundGaussians)
				bBackgroundHigh = true;

			// a match occurs when the pixel is within sqrt(fTg) standard deviations of the distribution
			if(dist < m_params.LowThreshold()*var) {
				bFitsPDF=true;

				// check if this Gaussian is part of the background model
				if(iModes < backgroundGaussians)
					bBackgroundLow = true;

				if(!update_model)
					break;	

				//update mean and weight of the distribution
				float k = m_params.Alpha()/weight;
				weight = fOneMinAlpha*weight + m_params.Alpha();

				m_modes[pos].weight = weight;
				m_modes[pos].muR = muR - k*(dR);
				m_modes[pos].muG = muG - k*(dG);
				m_modes[pos].muB = muB - k*(dB);
				//update the variance while maintaining it within limit
				float variance_new = var + k*(dist-var);
				m_modes[pos].variance = variance_new < min_variance ? min_variance : variance_new > max_variance ? max_variance : variance_new;


			} else {
				if(update_model){
					weight = fOneMinAlpha*weight;
					if (weight < min_weight) {
						weight=min_weight;
						numModes--;
					}

					m_modes[pos].weight = weight;
				}
			}
		} else {
			if(update_model){
				weight = fOneMinAlpha*weight;
				if (weight < min_weight) {
					weight=min_weight;
					numModes--;
				}
				m_modes[pos].weight = weight;
			}
		}
		if(iModes==STATIC_DIST_INDEX) {
			if(weight>=static_threshold)
				static_mask_value=FOREGROUND;
			else
				static_mask_value=BACKGROUND;
		}

		total_weight += weight;
	}

	low_threshold=bBackgroundLow?BACKGROUND:FOREGROUND;
	high_threshold=bBackgroundHigh?BACKGROUND:FOREGROUND;

	if(!update_model)
		return;

	// renormalize weights so they add to one    
	for (int i = 0; i < numModes; i++) {       

		pos=posPixel + i;
		m_modes[pos].weight/= total_weight;
		m_modes[pos].significants = m_modes[pos].weight/ sqrt(m_modes[pos].variance);
	}
	// Sort significance values so they are in descending order.
	qsort(&m_modes[posPixel],numModes,sizeof(GMM),compareGMM);
	//rearrangeDistributions(posPixel,numModes);

	// make new mode if needed and exit
	if (!bFitsPDF) {
		if (numModes < m_params.MaxModes()) {
			numModes++;
		} else {
			// the weakest mode will be replaced
		}

		pos = posPixel + numModes-1;

		m_modes[pos].muR = pixel.ch[0];
		m_modes[pos].muG = pixel.ch[1];
		m_modes[pos].muB = pixel.ch[2];
		m_modes[pos].variance = m_variance;
		m_modes[pos].significants = 0;			// will be set below

		if (numModes==1)
			m_modes[pos].weight = 1;
		else
			m_modes[pos].weight = m_params.Alpha();

		//renormalize weights

		total_weight = 0.0;
		for (int i = 0; i < numModes; i++) {
			pos=posPixel + i;
			total_weight += m_modes[pos].weight;
		}

		for (int i = 0; i < numModes; i++) {
			pos=posPixel + i;
			m_modes[pos].weight /=total_weight;
			m_modes[pos].significants = m_modes[pos].weight/ sqrt(m_modes[pos].variance);
		}
	}

	// Sort significance values so they are in descending order.
	qsort(&(m_modes[posPixel]),numModes,sizeof(GMM),compareGMM);
	//rearrangeDistributions(posPixel,numModes);

}

void GrimsonGMM::Subtract2(const RgbImage& data,BwImage& low_threshold_mask, 
						   BwImage& high_threshold_mask,BwImage& static_threshold_mask,BwImage& update_mask,double static_threshold) {
    unsigned char low_threshold, high_threshold,static_mask_value;
    long posPixel;
	bool update_model=true;
	
    // update each pixel of the image
    for(unsigned int r = 0; r < m_params.Height(); ++r) {
        for(unsigned int c = 0; c < m_params.Width(); ++c) {
            // update model + background subtract

            posPixel=(r*m_params.Width()+c)*m_params.MaxModes();

			if(update_mask(r,c)==FOREGROUND)
				update_model=false;
			else
				update_model=true;

            
            SubtractPixel2(posPixel, data(r,c), m_modes_per_pixel(r,c), low_threshold, 
				high_threshold,static_mask_value,static_threshold,update_model);

            low_threshold_mask(r,c) = low_threshold;
            high_threshold_mask(r,c) = high_threshold;
            static_threshold_mask(r,c)=static_mask_value;

            m_background(r,c,0) = (unsigned char)m_modes[posPixel].muR;
            m_background(r,c,1) = (unsigned char)m_modes[posPixel].muG;
            m_background(r,c,2) = (unsigned char)m_modes[posPixel].muB;
			
        }
    }
	//cvCopy(m_background.Ptr(), background_image);
}

void GrimsonGMM::rearrangeDistributions(long posPixel,unsigned char numModes) {
	float min_significant,current_significant;
	int min_index;
	GMM temp;

	for(int i=0; i<numModes; i++) {
		min_significant=m_modes[posPixel+i].significants;
		min_index=i;
		for(int j=i+1; j<numModes; j++) {
			current_significant=m_modes[posPixel+j].significants;
			if(current_significant<min_significant) {
				min_significant=current_significant;
				min_index=j;
			}
		}
		if(min_index!=i) {
			temp=m_modes[posPixel+i];
			m_modes[posPixel+i]=m_modes[posPixel+min_index];
			m_modes[posPixel+min_index]=temp;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//Input:
//  data - a pointer to the data of a RGB image of the same size
//Output:
//  output - a pointer to the data of a gray value image of the same size
//					(the memory should already be reserved)
//					values: 255-foreground, 125-shadow, 0-background
///////////////////////////////////////////////////////////////////////////////
void GrimsonGMM::Update(const RgbImage& data,  const BwImage& update_mask) {
	// it doesn't make sense to have conditional updates in the GMM framework
}


void GrimsonGMM::Subtract(const RgbImage& data,
                          BwImage& low_threshold_mask, BwImage& high_threshold_mask) {
    unsigned char low_threshold, high_threshold;
    long posPixel;

    // update each pixel of the image
    for(unsigned int r = 0; r < m_params.Height(); ++r) {
        for(unsigned int c = 0; c < m_params.Width(); ++c) {
            // update model + background subtract
            posPixel=(r*m_params.Width()+c)*m_params.MaxModes();

            SubtractPixel(posPixel, data(r,c), m_modes_per_pixel(r,c), low_threshold, high_threshold);


            low_threshold_mask(r,c) = low_threshold;
            high_threshold_mask(r,c) = high_threshold;


            m_background(r,c,0) = (unsigned char)m_modes[posPixel].muR;
            m_background(r,c,1) = (unsigned char)m_modes[posPixel].muG;
            m_background(r,c,2) = (unsigned char)m_modes[posPixel].muB;
        }
    }
	//cvShowImage( "Actual Background", m_background.Ptr() );
	//cvCopy(m_background.Ptr(), background_image);
}

void GrimsonGMM::SubtractPixel(long posPixel, const RgbPixel& pixel, unsigned char& numModes,unsigned char& low_threshold, unsigned char& high_threshold) {
    // calculate distances to the modes (+ sort???)
    // here we need to go in descending order!!!
    long pos;
    bool bFitsPDF=false;
    bool bBackgroundLow=false;
    bool bBackgroundHigh=false;

    float fOneMinAlpha = 1-m_params.Alpha();

    float total_weight = 0.0f;

    // calculate number of Gaussians to include in the background model
    int backgroundGaussians = 0;
    double sum = 0.0;
    for(int i = 0; i < numModes; ++i) {
        if(sum < m_bg_threshold) {
            backgroundGaussians++;
            sum += m_modes[posPixel+i].weight;
        } else {
            break;
        }
    }
    float weight,var,muR,muG,muB;
    float dR,dG,dB;
    float dist;
    float k,variance_new;

    // update all distributions and check for match with current pixel
    for (int iModes=0; iModes < numModes; iModes++) {
        pos=posPixel+iModes;
        weight = m_modes[pos].weight;

        // fit not found yet
        if (!bFitsPDF) {
            //check if it belongs to some of the modes
            //calculate distance
            var = m_modes[pos].variance;
            muR = m_modes[pos].muR;
            muG = m_modes[pos].muG;
            muB = m_modes[pos].muB;


            dR=muR - pixel(0);
            dG=muG - pixel(1);
            dB=muB - pixel(2);

            // calculate the squared distance
            dist = (dR*dR + dG*dG + dB*dB);

            if(dist < m_params.HighThreshold()*var && iModes < backgroundGaussians)
                bBackgroundHigh = true;

            // a match occurs when the pixel is within sqrt(fTg) standard deviations of the distribution
            if(dist < m_params.LowThreshold()*var) {
                bFitsPDF=true;

                // check if this Gaussian is part of the background model
                if(iModes < backgroundGaussians)
                    bBackgroundLow = true;

                //update distribution
                k = m_params.Alpha()/weight;
                weight = fOneMinAlpha*weight + m_params.Alpha();
                m_modes[pos].weight = weight;
                m_modes[pos].muR = muR - k*(dR);
                m_modes[pos].muG = muG - k*(dG);
                m_modes[pos].muB = muB - k*(dB);

                //limit the variance
                variance_new = var + k*(dist-var);
                m_modes[pos].variance = variance_new < 4 ? 4 : variance_new > 5*m_variance ? 5*m_variance : variance_new;
                m_modes[pos].significants = m_modes[pos].weight / sqrt(m_modes[pos].variance);
            } else {
                weight = fOneMinAlpha*weight;
                if (weight < 0.0) {
                    weight=0.0;
                    numModes--;
                }

                m_modes[pos].weight = weight;
            }
        } else {
            weight = fOneMinAlpha*weight;
            if (weight < 0.0) {
                weight=0.0;
                numModes--;
            }
            m_modes[pos].weight = weight;
        }
        total_weight += weight;
    }

    // renormalize weights so they add to one
    double inv_total_weight = 1.0 / total_weight;
    for (int iLocal = 0; iLocal < numModes; iLocal++) {
        pos=posPixel + iLocal;
        m_modes[pos].weight *= (float)inv_total_weight;
        m_modes[pos].significants = m_modes[pos].weight/ sqrt(m_modes[pos].variance);
    }

    // Sort significance values so they are in desending order.
    qsort(&m_modes[posPixel],  numModes, sizeof(GMM), compareGMM);

    // make new mode if needed and exit
    if (!bFitsPDF) {
        if (numModes < m_params.MaxModes()) {
            numModes++;
        } else {
            // the weakest mode will be replaced
        }

        pos = posPixel + numModes-1;

        m_modes[pos].muR = pixel.ch[0];
        m_modes[pos].muG = pixel.ch[1];
        m_modes[pos].muB = pixel.ch[2];
        m_modes[pos].variance = m_variance;
        m_modes[pos].significants = 0;			// will be set below

        if (numModes==1)
            m_modes[pos].weight = 1;
        else
            m_modes[pos].weight = m_params.Alpha();

        //renormalize weights
        int iLocal;
        float sum = 0.0;
        for (iLocal = 0; iLocal < numModes; iLocal++) {
            sum += m_modes[posPixel+ iLocal].weight;
        }

        double invSum = 1.0/sum;
        for (iLocal = 0; iLocal < numModes; iLocal++) {
            pos=posPixel + iLocal;
            m_modes[pos].weight *= (float)invSum;
            m_modes[pos].significants = m_modes[pos].weight/ sqrt(m_modes[pos].variance);
        }
    }

    // Sort significance values so they are in descending order.
    qsort(&(m_modes[posPixel]), numModes, sizeof(GMM), compareGMM);

    if(bBackgroundLow) {
        low_threshold = BACKGROUND;
    } else {
        low_threshold = FOREGROUND;
    }

    if(bBackgroundHigh) {
        high_threshold = BACKGROUND;
    } else {
        high_threshold = FOREGROUND;
    }
}

void GrimsonGMM::pushIntoBackground(const CvRect bounding_box,RgbImage &current_frame){

	CvPoint min_point,max_point;
	min_point.x=bounding_box.x;
	min_point.y=bounding_box.y;

	max_point.x=min_point.x+bounding_box.width;
	max_point.y=min_point.y+bounding_box.height;

	for(int r=min_point.y;r<=max_point.y;r++){
		for(int c=min_point.x;c<=max_point.x;c++){

			long pixel_pos=(r*m_params.Width()+c)*m_params.MaxModes();

			m_modes[pixel_pos].muR=(float)current_frame(r,c,0);
			m_modes[pixel_pos].muG=(float)current_frame(r,c,1);
			m_modes[pixel_pos].muB=(float)current_frame(r,c,2);

			m_background(r,c,0)=current_frame(r,c,0);
			m_background(r,c,1)=current_frame(r,c,1);
			m_background(r,c,2)=current_frame(r,c,2);
			
		}

	}
}

void GrimsonGMM::resetBackground(RgbImage &current_frame,BwImage *reset_mask){

	int img_width=current_frame.width();
	int img_height=current_frame.height();

	if((img_width!=m_background.width())||(img_height!=m_background.height())){
		cout<<"\nError in GrimsonGMM::resetBackground.\n";
		cout<<"Current frame dimensions do not match those of the background image.\n";
		cout<<"Current frame: "<<img_width<<" X "<<img_height<<"\n";
		cout<<"Background frame: "<<m_background.width()<<" X "<<m_background.height()<<"\n";
		_getch();
		return;
	}

	for(int r=0;r<img_height;r++){
		for(int c=0;c<img_width;c++){

			if((reset_mask!=NULL)&&((*reset_mask)(r,c)==FOREGROUND))
				continue;

			long pixel_pos=(r*m_params.Width()+c)*m_params.MaxModes();

			m_modes[pixel_pos].muR=(float)current_frame(r,c,0);
			m_modes[pixel_pos].muG=(float)current_frame(r,c,1);
			m_modes[pixel_pos].muB=(float)current_frame(r,c,2);

			m_background(r,c,0)=current_frame(r,c,0);
			m_background(r,c,1)=current_frame(r,c,1);
			m_background(r,c,2)=current_frame(r,c,2);
		}
	}
}
