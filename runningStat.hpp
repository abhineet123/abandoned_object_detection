#ifndef _RUNNING_STAT_
#define _RUNNING_STAT_

#define NCHANNELS 3


#include<vector>

using namespace std;

class Pixel_t{

public:

	vector<double> val;

	Pixel_t(){
		val.resize(NCHANNELS);
	}

};

class RunningStatVector {

public:
	RunningStatVector(){
		no_of_elements = 0;

		final_mean=new Pixel_t();
		final_var=new Pixel_t();
		final_std=new Pixel_t();
	}

	void clearAll() {
		no_of_elements = 0;
	}

	void addElement(Pixel_t *x) {
		no_of_elements++;

		if (no_of_elements == 1) {
			for(int ch=0;ch<NCHANNELS;ch++){
				old_mean.val[ch]=x->val[ch];
				new_mean.val[ch]=x->val[ch];
				old_var.val[ch]=0.0;
			}		
		} else {
			for(int ch=0;ch<NCHANNELS;ch++){

				new_mean.val[ch] = old_mean.val[ch] + (x->val[ch] - old_mean.val[ch])/(double)no_of_elements;
				new_var.val[ch] = old_var.val[ch] + (x->val[ch] - old_mean.val[ch])*(x->val[ch] - new_mean.val[ch]);

				// set up for next iteration
				old_mean.val[ch] = new_mean.val[ch];
				old_var.val[ch] = new_var.val[ch];
			}
		}
	}

	int getElementCount()  {
		return no_of_elements;
	}

	void updateState() {

		for(int ch=0;ch<NCHANNELS;ch++){
			if(no_of_elements > 0){
				final_mean->val[ch]=new_mean.val[ch];
			}else{
				final_mean->val[ch]=0.0;			
			}

			if(no_of_elements > 1){
				final_var->val[ch]=new_var.val[ch]/(double)(no_of_elements - 1);	
			}else{
				final_var->val[ch]=0.0;
			}
			final_std->val[ch]=sqrt(final_var->val[ch]);

		}		
	}

	Pixel_t *final_mean,*final_var,*final_std;

private:
	int no_of_elements;
	Pixel_t old_mean, new_mean, old_var, new_var;
};

class RunningStatScalar {

public:
	RunningStatScalar(){
		cout<<"\n\n here we are in RunningStatScalar::RunningStatScalar\n\n";
		new_mean=0.0;
		no_of_elements = 0;
	}

	void clearAll() {
		no_of_elements = 0;
	}

	void addElement(double x) {
		cout<<"\n\n here we are in RunningStatScalar::addElement\n\n";
		no_of_elements++;

		if (no_of_elements == 1) {			
			old_mean=x;
			new_mean=x;
			old_var=0.0;

		} else {	
			new_mean=old_mean + (x- old_mean)/(double)no_of_elements;
			new_var=old_var+ (x-old_mean)*(x-new_mean);

			// set up for next iteration
			old_mean=new_mean;
			old_var=new_var;			
		}
		final_mean=new_mean;
		cout<<"\nfinal_mean="<<final_mean<<"\n";
	}

	int getElementCount()  {
		return no_of_elements;
	}

	double mean(){
		if(no_of_elements > 0){
			final_mean=new_mean;
		}else{
			final_mean=0.0;			
		}
		return final_mean;
	}

	double var(){
		if(no_of_elements > 1){
			final_var=new_var/(double)(no_of_elements - 1);	
		}else{
			final_var=0.0;
		}		
		return final_var;
	}	

private:
	int no_of_elements;
	double old_mean, new_mean, old_var, new_var;
	double final_mean,final_var;
};

#endif

