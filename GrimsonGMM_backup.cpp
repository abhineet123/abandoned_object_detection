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

				//update mean and weight of the distribution
				float k = m_params.Alpha()/weight;
				weight = fOneMinAlpha*weight + m_params.Alpha();

				if(update_model){
					m_modes[pos].weight = weight;
					m_modes[pos].muR = muR - k*(dR);
					m_modes[pos].muG = muG - k*(dG);
					m_modes[pos].muB = muB - k*(dB);
					//update the variance while maintaining it within limit
					float variance_new = var + k*(dist-var);
					m_modes[pos].variance = variance_new < min_variance ? min_variance : variance_new > max_variance ? max_variance : variance_new;
				}

            } else {
                weight = fOneMinAlpha*weight;
                if (weight < min_weight) {
                    weight=min_weight;
                    numModes--;
                }

                m_modes[pos].weight = weight;
            }
        } else {
            weight = fOneMinAlpha*weight;
            if (weight < min_weight) {
                weight=min_weight;
                numModes--;
            }
            m_modes[pos].weight = weight;
        }
        if(iModes==STATIC_DIST_INDEX) {
            if(weight>=static_threshold)
                static_mask_value=FOREGROUND;
            else
                static_mask_value=BACKGROUND;
        }

        total_weight += weight;
    }

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


    low_threshold=bBackgroundLow?BACKGROUND:FOREGROUND;
    high_threshold=bBackgroundHigh?BACKGROUND:FOREGROUND;
}


void GrimsonGMM::Subtract2(int frame_num, const RgbImage& data,BwImage& low_threshold_mask,
						   BwImage& high_threshold_mask,BwImage& static_threshold_mask,BwImage& update_mask,double static_threshold) {
    unsigned char low_threshold, high_threshold,static_mask_value;
    long posPixel;
	bool update_model;

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
