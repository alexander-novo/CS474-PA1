#include <iostream>
#include <fstream>
#include <sstream>

#include "../Common/image.h"

/*
	Subsamples and image based on the sampling factor
	@Param: image - the input image that will be sampled
	@Param: subsample_factor - the factor by which to sample
	@Return: void
*/
void subsample_image(Image& image, int subsample_factor){

	// iterate through image to get the sample
	for(int i=0; i<image.cols; i += subsample_factor){
   		for(int j=0; j<image.rows; j += subsample_factor) {

   			// save the sampled pixel
   			int pixelSample = image[i][j];

   			// Modify neighbor pixels to match the sampled pixel
   			for (int k = 0; k < subsample_factor; k++)
   			{
   				for (int l = 0; l < subsample_factor; l++)
   				{
   					image[i + k][j + l] = pixelSample;
   				}
   			}
    	}
    }
}

int main(int argc, char** argv) {

 	int M, N, Q;
 	bool type;
 	int val;
 	int subsample_factor;
 	std::istringstream ss(argv[3]);

 	// Get sampling factor, error checking
 	if(ss >> subsample_factor) {
 		if(256 % subsample_factor != 0 || subsample_factor > 256){
 			std::cout << "Error: Subsample factor should be power of 2 less than 256" << std::endl;
 			return 1;

 		}
 	}

 	//read image
 	std::ifstream inFile(argv[1]);

 	Image image = Image::read(inFile);

	std::cout << "Question 1: Sampling." << std::endl;

	// sample the image
	subsample_image(image, subsample_factor);

	// Save output image
	std::ofstream outFile;
	outFile.open(argv[2]);
	outFile << image;
	outFile.close();

	return 0;
}