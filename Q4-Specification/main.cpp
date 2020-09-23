#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <regex>
#include <vector>

#include "../Common/histogram_tools.h"
#include "../Common/image.h"

// Struct for inputting arguments from command line
struct Arguments {
	char *inputImagePath, *outImagePath, *histogramPath;
	Image inputImage;
	std::ifstream histogramFile;
	std::ofstream outFile;
	unsigned histogramWidth = 64, histogramHeight = 10;
	bool plot = false;
	std::ofstream plotFile;
};

int specify(Arguments& arg);
void printHistogram(unsigned* histogram, const Arguments& arg);
bool verifyArguments(int argc, char** argv, Arguments& arg, int& err);
void printHelp();

int main(int argc, char** argv) {
	int err;
	Arguments arg;

	if (!verifyArguments(argc, argv, arg, err)) { return err; }

	return specify(arg);
}

int specify(Arguments& arg) {
	unsigned* histogram    = new unsigned[arg.inputImage.maxVal + 1];
	unsigned* newHistogram = new unsigned[arg.inputImage.maxVal + 1];
	unsigned* cdf          = new unsigned[arg.inputImage.maxVal + 1];
	std::mutex* locks      = new std::mutex[arg.inputImage.maxVal + 1];
	std::vector<unsigned> targetHistogram;
	unsigned targetPixels = 0;  // Number of pixels in the target histogram

	// Initialise histogram bins to be empty
#pragma omp parallel for
	for (unsigned i = 0; i <= arg.inputImage.maxVal; i++) {
		histogram[i] = newHistogram[i] = 0;
	}

	// Create input image histogram
#pragma omp parallel for
	for (unsigned i = 0; i < arg.inputImage.rows * arg.inputImage.cols; i++) {
		unsigned bin = arg.inputImage.pixels[i];
		locks[bin].lock();
		histogram[bin]++;
		locks[bin].unlock();
	}

	// Start with enough space to hold our input image. If we need more, we can get
	// more, but we're probably working with similarly-valued images.
	targetHistogram.reserve(arg.inputImage.maxVal + 1);

	// Read in target histogram
	std::string line;
	std::regex rHistogram("^([[:digit:]]+)[[:space:]]+([[:digit:]]+).*");
	std::smatch matches;
	while (arg.histogramFile) {
		std::getline(arg.histogramFile, line);
		if (!std::regex_match(line, matches, rHistogram)) continue;

		if (stoul(matches[1].str()) != targetHistogram.size()) {
			std::cout << "Error in reading histogram file \"" << arg.histogramPath
			          << "\":\n"
			          << "Bucket \"" << stoul(matches[1].str())
			          << "\" was expected to be \"" << targetHistogram.size()
			          << "\".";
		}

		targetHistogram.push_back(stoul(matches[2].str()));
		targetPixels += targetHistogram.back();
	}
	arg.histogramFile.close();

	// Calculate CDFs
	std::vector<unsigned> targetCDF(targetHistogram.size());
	cdf[0]       = histogram[0];
	targetCDF[0] = targetHistogram[0];

	for (unsigned i = 1; i <= arg.inputImage.maxVal; i++) {
		cdf[i] = cdf[i - 1] + histogram[i];
	}
	for (unsigned i = 1; i < targetHistogram.size(); i++) {
		targetCDF[i] = targetCDF[i - 1] + targetHistogram[i];
	}

// Tranform input image with its CDF and inverse CDF of target histogram
#pragma region CDF transformation
	// Separate cases for if the images have different dimensions/maxVal, to make
	// calculation easier
	if (arg.inputImage.maxVal == targetHistogram.size() - 1) {
		if (arg.inputImage.rows * arg.inputImage.cols == targetPixels) {
#pragma omp parallel for
			for (unsigned i = 0; i < arg.inputImage.rows * arg.inputImage.cols; i++) {
				Image::pixelT& pixelVal = arg.inputImage.pixels[i];

				unsigned inversePixel = cdf[pixelVal];
				pixelVal              = targetCDF.rend() -
				           std::lower_bound(targetCDF.rbegin(), targetCDF.rend(),
				                            inversePixel, std::greater<unsigned>());
			}
		} else {
#pragma omp parallel for
			for (unsigned i = 0; i < arg.inputImage.rows * arg.inputImage.cols; i++) {
				Image::pixelT& pixelVal = arg.inputImage.pixels[i];

				unsigned inversePixel = cdf[pixelVal] * targetPixels /
				                        (arg.inputImage.rows * arg.inputImage.cols);
				pixelVal = targetCDF.rend() -
				           std::lower_bound(targetCDF.rbegin(), targetCDF.rend(),
				                            inversePixel, std::greater<unsigned>());
			}
		}
	} else if (arg.inputImage.rows * arg.inputImage.cols == targetPixels) {
#pragma omp parallel for
		for (unsigned i = 0; i < arg.inputImage.rows * arg.inputImage.cols; i++) {
			Image::pixelT& pixelVal = arg.inputImage.pixels[i];

			unsigned inversePixel =
			    cdf[pixelVal] * arg.inputImage.maxVal / (targetHistogram.size() - 1);
			pixelVal = targetCDF.rend() -
			           std::lower_bound(targetCDF.rbegin(), targetCDF.rend(),
			                            inversePixel, std::greater<unsigned>());
		}
	} else {
		// In this case, we need to do math with ull because of the multiplications
		// overflowing The result after division should fit within an unsigned, though
#pragma omp parallel for
		for (unsigned i = 0; i < arg.inputImage.rows * arg.inputImage.cols; i++) {
			Image::pixelT& pixelVal = arg.inputImage.pixels[i];

			unsigned inversePixel = ((unsigned long long) cdf[pixelVal]) *
			                        arg.inputImage.maxVal * targetPixels /
			                        (arg.inputImage.rows * arg.inputImage.cols *
			                         (targetHistogram.size() - 1));
			pixelVal = targetCDF.rend() -
			           std::lower_bound(targetCDF.rbegin(), targetCDF.rend(),
			                            inversePixel, std::greater<unsigned>());
		}
	}
#pragma endregion CDF transformation

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

	std::cout << "\nInput histogram \"" << arg.histogramPath << "\":\n";
	Histogram::print(&targetHistogram.front(), targetHistogram.size(),
	                 arg.histogramWidth, arg.histogramHeight);

	std::cout << "\nHistogram of output image \"" << arg.outImagePath << "\":\n";
	Histogram::print(newHistogram, arg.inputImage.maxVal + 1, arg.histogramWidth,
	                 arg.histogramHeight);

	// Print histogram data for plot file
	if (arg.plot) {
		arg.plotFile << "Source Input-Image Input-Histogram Specified\n";
		unsigned i;
		for (i = 0; i <= arg.inputImage.maxVal && i < targetHistogram.size(); i++) {
			arg.plotFile << i << "    " << histogram[i] << "    "
			             << targetHistogram[i] << "    " << newHistogram[i] << '\n';
		}
		arg.plotFile.close();
	}

	delete[] histogram;
	delete[] newHistogram;
	delete[] cdf;
	delete[] locks;

	return 0;
}

bool verifyArguments(int argc, char** argv, Arguments& arg, int& err) {
	if (argc < 2 ||
	    (argc < 4 && strcmp(argv[1], "-h") && strcmp(argv[1], "--help"))) {
		std::cout << "Missing operand.\n";
		err = 1;
		printHelp();
		return false;
	}

	if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
		printHelp();
		return false;
	}

	// Find optional argument switches
	for (unsigned i = 4; i < argc; i++) {
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

	arg.histogramPath = argv[2];
	arg.histogramFile.open(argv[2]);
	if (!arg.histogramFile) {
		std::cout << "Could not open \"" << argv[2] << "\"\n";
		err = 2;
		return false;
	}

	arg.outImagePath = argv[3];
	arg.outFile.open(argv[3]);
	if (!arg.outFile) {
		std::cout << "Could not open \"" << argv[3] << "\"\n";
		err = 2;
		return false;
	}

	return true;
}

void printHelp() {
	std::cout
	    << "Usage: specify <image> <histogram> <output> [options]   (1)\n"
	    << "   or: specify -h                                       (2)\n\n"
	    << "(1) Take an image file as input, change its histogram to the\n"
	    << "    specified histogram, and write new image to output file.\n"
	    << "    Displays the original histogram and the new equalized histogram.\n"
	    << "    Histogram files can be obtained by running 'equalize' with\n"
	    << "    the -p flag set (or 'specify' with the -p falg set).\n"
	    << "(2) Print this help menu\n\n"
	    << "Options:\n"
	    << "  -width <width>    Number of visual histogram bins\n"
	    << "  -height <height>  Height of visual histogram (in lines)\n"
	    << "  -p <file>         Send histogram plotting data to a file for gnuplot\n";
}