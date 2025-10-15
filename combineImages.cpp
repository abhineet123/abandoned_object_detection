#include "combineImages.hpp"

using namespace cv;

IplImage* initializeCombinedImage(CvSize img1_size,CvSize img2_size,int nchannels,int combine_method){
	int combined_width=0,combined_height=0;

	if(combine_method==HORIZONTAL_JOIN){
		
		combined_width=img1_size.width+img2_size.width;
		combined_height=img1_size.height;

	}else if(combine_method==VERTICAL_JOIN){

		combined_width=img1_size.width;
		combined_height=img1_size.height+img2_size.height;

	}else{

		std::cout<<"Invalid join method specified.\n";
		return NULL;
	}

	IplImage *combined_img=cvCreateImage(cvSize(combined_width,combined_height),IPL_DEPTH_8U,nchannels);

	return combined_img;

}
void combineImages(IplImage* img1,IplImage* img2,IplImage* combined_img,int combine_method){

	if(img1->nChannels!=img2->nChannels){
		std::cout<<"Images to be combined do not have the same depth.\n";
		std::cout<<"img1_depth="<<img1->nChannels<<" img2_depth="<<img2->nChannels<<"\n";
		return;
	}
	if(img1->nChannels!=combined_img->nChannels){
		std::cout<<"Destination image does not have the same depth as the input images.\n";
		return;
	}

	int img1_width=img1->width,img1_height=img1->height;
	int img2_width=img2->width,img2_height=img2->height;
	int nchannels=img1->nChannels;

	int combined_width,combined_height;
	int old_location,new_location;

	if(combine_method==HORIZONTAL_JOIN){

		if(img1_height!=img2_height){
			std::cout<<"Images to be combined do not have the same height.\n";
			std::cout<<"img1_height="<<img1_height<<" img2_height="<<img2_height<<"\n";
			return;
		}
		combined_width=img1_width+img2_width;
		combined_height=img1_height;

	}else if(combine_method==VERTICAL_JOIN){

		if(img1_width!=img2_width){
			std::cout<<"Images to be combined do not have the same width.\n";
			std::cout<<"img1_width="<<img1_width<<" img2_width="<<img2_width<<"\n";
			return;
		}
		combined_width=img1_width;
		combined_height=img1_height+img2_height;

	}else{

		std::cout<<"Invalid join method specified.\n";
		return;
	}

	if((combined_img->height!=combined_height)||(combined_img->width!=combined_width)){
		std::cout<<"Destination image has incorrect dimensions.\n";
		return;
	}

	//copying image 1
	//std::cout<<"Beginning image 1 copying\n";
	for(int r=0;r<img1_height;r++){

		for(int c=0;c<img1_width;c++){

			old_location=r*img1->widthStep+c*img1->nChannels;
			new_location=r*combined_img->widthStep+c*combined_img->nChannels;

			for(int ch=0;ch<nchannels;ch++){				
				combined_img->imageData[new_location+ch]=img1->imageData[old_location+ch];
			}
		}
	}
	//std::cout<<"Done image 1 copying\n";

	//copying image 2
	//std::cout<<"Beginning image 2 copying\n";
	for(int r=0;r<img2_height;r++){

		for(int c=0;c<img2_width;c++){

			old_location=r*img2->widthStep+c*img2->nChannels;

			if(combine_method==HORIZONTAL_JOIN)
				new_location=r*combined_img->widthStep+(c+img1_width)*combined_img->nChannels;
			else if(combine_method==VERTICAL_JOIN)
				new_location=(r+img1_height)*combined_img->widthStep+c*combined_img->nChannels;
			else{
				std::cout<<"Invalid join method specified.\n";
				return;
			}

			for(int ch=0;ch<nchannels;ch++){				
				combined_img->imageData[new_location+ch]=img2->imageData[old_location+ch];
			}
		}
	}
	//std::cout<<"Done image 2 copying\n";
}

void covertGrayscaleToRGB(IplImage *img_gray,IplImage *img_rgb){

	if((img_gray->height!=img_rgb->height)||(img_gray->width!=img_rgb->width)){
		std::cout<<"Images have incompatible dimensions.\n";
		std::cout<<"img_gray: width="<<img_gray->width<<"\t height ="<<img_gray->height<<"\t depth="<<img_gray->nChannels<<"\n";
		std::cout<<"img_rgb: width="<<img_rgb->width<<"\t height ="<<img_rgb->height<<"\t depth="<<img_rgb->nChannels<<"\n";
		return;
	}
	if((img_gray->nChannels!=1)||(img_rgb->nChannels!=3)){
		std::cout<<"Specified images have incorrect depths.\n";
		std::cout<<"img_gray: width="<<img_gray->width<<"\t height ="<<img_gray->height<<"\t depth="<<img_gray->nChannels<<"\n";
		std::cout<<"img_rgb: width="<<img_rgb->width<<"\t height ="<<img_rgb->height<<"\t depth="<<img_rgb->nChannels<<"\n";
		return;
	}
	unsigned char current_value;
	int current_location;

	for(int r=0;r<img_rgb->height;r++){

		for(int c=0;c<img_rgb->width;c++){
			
			current_value=(unsigned char)img_gray->imageData[r*img_gray->widthStep+c];
			current_location=r*img_rgb->widthStep+c*img_rgb->nChannels;

			for(int ch=0;ch<img_rgb->nChannels;ch++){
				img_rgb->imageData[current_location+ch]=current_value;
			}
		}
	}
}
void getIntensityRange(IplImage *img_gray,CvSize image_size,int &min_val,int &max_val){
	min_val=INF;
	max_val=-INF;
	for(int r=0;r<image_size.height;r++){
		int location=r*img_gray->widthStep;
		for(int c=0;c<image_size.width;c++){

			int current_intensity=img_gray->imageData[location+c]+128;
			if(current_intensity<min_val)
				min_val=current_intensity;

			if(current_intensity>max_val)
				max_val=current_intensity;
		}
	}
}
