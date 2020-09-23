// Q3-Equalization/main.cpp
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>

#include "../Common/histogram_tools.h"
#include "../Common/image.h"

// Struct for inputting arguments from command line
struct Arguments {
	char *inputImagePath, *outImagePath;
	Image inputImage;
	std::ofstream outFile;
	unsigned histogramWidth = 64, histogramHeight = 10;
	bool plot = false;
	std::ofstream plotFile;
};

void equalize(Arguments& arg);
bool verifyArguments(int argc, char** argv, Arguments& arg, int& err);
void printHelp();

int main(int argc, char** argv) {
	int err;
	Arguments arg;

	if (!verifyArguments(argc, argv, arg, err)) { return err; }

	equalize(arg);

	return 0;
}

void equalize(Arguments& arg) {
	unsigned* histogram    = new unsigned[arg.inputImage.maxVal + 1];
	unsigned* newHistogram = new unsigned[arg.inputImage.maxVal + 1];
	unsigned* cdf          = new unsigned[arg.inputImage.maxVal + 1];
	std::mutex* locks      = new std::mutex[arg.inputImage.maxVal + 1];
	// Initialise histogram bins to be empty
#pragma omp parallel for
	for (unsigned i = 0; i <= arg.inputImage.maxVal; i++) {
		histogram[i] = newHistogram[i] = 0;
	}

	// Create histogram
#pragma omp parallel for
	for (unsigned i = 0; i < arg.inputImage.rows * arg.inputImage.cols; i++) {
		unsigned bin = arg.inputImage.pixels[i];
		locks[bin].lock();
		histogram[bin]++;
		locks[bin].unlock();
	}

	// Calculate CDF
	cdf[0] = histogram[0];
	for (unsigned i = 1; i <= arg.inputImage.maxVal; i++) {
		cdf[i] = cdf[i - 1] + histogram[i];
	}

	// Tranform image with the CDF
#pragma omp parallel for
	for (unsigned i = 0; i < arg.inputImage.rows * arg.inputImage.cols; i++) {
		Image::pixelT& pixelVal = arg.inputImage.pixels[i];
		pixelVal                = cdf[pixelVal] * arg.inputImage.maxVal /
		           (arg.inputImage.rows * arg.inputImage.cols);
	}

	// Write new transformed image out
	arg.outFile << arg.inputImage;
	arg.outFile.close();

	// Calculate histogram of new image
#pragma omp parallel for
	for (unsigned i = 0; i < arg.inputImage.rows * arg.inputImage.cols; i++) {
		unsigned bin = arg.inputImage.pixels[i];
		locks[bin].lock();
		newHistogram[bin]++;
		locks[bin].unlock();
	}

	// Print histograms
	std::cout << "\nHistogram of input image \"" << arg.inputImagePath << "\":\n";
	Histogram::print(histogram, arg.inputImage.maxVal + 1, arg.histogramWidth,
	                 arg.histogramHeight);

	std::cout << "\nHistogram of output image \"" << arg.outImagePath << "\":\n";
	Histogram::print(newHistogram, arg.inputImage.maxVal + 1, arg.histogramWidth,
	                 arg.histogramHeight);

	// Print histogram data for plot file
	if (arg.plot) {
		arg.plotFile << "Image Input Equalized\n";
		for (unsigned i = 0; i <= arg.inputImage.maxVal; i++) {
			arg.plotFile << i << "    " << histogram[i] << "    " << newHistogram[i]
			             << '\n';
		}
		arg.plotFile.close();
	}

	delete[] histogram;
	delete[] newHistogram;
	delete[] cdf;
	delete[] locks;
}

bool verifyArguments(int argc, char** argv, Arguments& arg, int& err) {
	// If there are not the minimum number of arguments, print help and leave
	if (argc < 2 ||
	    (argc < 3 && strcmp(argv[1], "-h") && strcmp(argv[1], "--help"))) {
		std::cout << "Missing operand.\n";
		err = 1;
		printHelp();
		return false;
	}

	// If the user asks for the help menu, print help and leave
	if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
		printHelp();
		return false;
	}

	// Find optional argument switches
	for (unsigned i = 3; i < argc; i++) {
		if (!strcmp(argv[i], "-width")) {
			if (i + 1 >= argc) {
				std::cout << "Missing width";
				err = 1;
				printHelp();
				return false;
			}

			arg.histogramWidth = strtoul(argv[i + 1], nullptr, 10);
			if (arg.histogramWidth == 0) {
				std::cout << "Width \"" << argv[i + 1]
				          << "\" could not be recognised as a positive integer.";
				err = 2;
				return false;
			}

			i++;
		} else if (!strcmp(argv[i], "-height")) {
			if (i + 1 >= argc) {
				std::cout << "Missing height";
				err = 1;
				break;
			}

			arg.histogramHeight = strtoul(argv[i + 1], nullptr, 10);
			if (arg.histogramHeight == 0) {
				std::cout << "Height \"" << argv[i + 1]
				          << "\" could not be recognised as a positive integer.";
				err = 2;
				return false;
			}

			i++;
		} else if (!strcmp(argv[i], "-p")) {
			if (i + 1 >= argc) {
				std::cout << "Missing plot output file";
				err = 1;
				break;
			}

			arg.plot = true;
			arg.plotFile.open(argv[i + 1]);

			if (!arg.plotFile) {
				std::cout << "Plot file \"" << argv[i + 1]
				          << "\" could not be opened";
				err = 2;
				return false;
			}

			i++;
		}
	}

	// Required arguments
	arg.inputImagePath = argv[1];
	std::ifstream inFile(argv[1]);
	try {
		arg.inputImage = Image::read(inFile);
	} catch (std::exception& e) {
		std::cout << "Image \"" << argv[1] << "\"failed to be read: \"" << e.what()
		          << "\"\n";
		err = 2;
		return false;
	}

	arg.outImagePath = argv[2];
	arg.outFile.open(argv[2]);
	if (!arg.outFile) {
		std::cout << "Could not open \"" << argv[2] << "\"\n";
		err = 2;
		return false;
	}

	return true;
}

void printHelp() {
	std::cout
	    << "Usage: equalize <image> <output> [options]   (1)\n"
	    << "   or: equalize -h                           (2)\n\n"
	    << "(1) Take an image file as input, equalize its histogram,\n"
	    << "    and write new image to output file. Displays the original\n"
	    << "    histogram and the new equalized histogram.\n"
	    << "(2) Print this help menu\n\n"
	    << "Options:\n"
	    << "  -width <width>    Number of visual histogram bins\n"
	    << "  -height <height>  Height of visual histogram (in lines)\n"
	    << "  -p <file>         Send histogram plotting data to a file for gnuplot\n";
}