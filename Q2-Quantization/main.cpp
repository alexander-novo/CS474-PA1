#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "../Common/image.h"

/*
  Quantizes an image based on the quantization level
  @Param: image - the input image that will be quantized
  @Param: quantization_level - the number of gray level values to use
  @Return: void
*/
void quantize_image(Image& image, int quantization_level){

  int offset = 256 / quantization_level;
  std::vector<int> newPixelValues;

  // calculate new values for pixels
  for (int i = 0; i < quantization_level; i++)
      newPixelValues.push_back(i * offset);


	for(int i=0; i<image.cols; i++) {
   		for(int j=0; j<image.rows; j++) {

   			// current pixel
   			int pixelValue = image[i][j];

        // index for new pixel value
   			int index = pixelValue / offset;

        // update image pixel
        image[i][j] = newPixelValues[index];
    	}
    }
}

int main(int argc, char** argv) {
	int M, N, Q;
 	bool type;
 	int val;
 	int quantization_level;
 	std::istringstream ss(argv[3]);

 	if(ss >> quantization_level) {
 		if(quantization_level > 256){
 			std::cout << "Error: Quantization level should be less than 256" << std::endl;
 			return 1;

 		}
 	}

 	std::ifstream inFile(argv[1]);

 	Image image = Image::read(inFile);

	std::cout << "Question 2: Quantization." << std::endl;

	quantize_image(image, quantization_level);

	// Save output image
	std::ofstream outFile;
	outFile.open(argv[2]);
	outFile << image;
	outFile.close();

	return 0;
}