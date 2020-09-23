// Common/histogram_tools.cpp
#include "histogram_tools.h"

#include <algorithm>
#include <iostream>

void Histogram::print(unsigned* histogram, unsigned bins, unsigned width, unsigned height) {
	// An adjusted histogram, which has been binned
	unsigned* binnedHistogram = new unsigned[width];
	// Maximum number of original bins represented by each new bin
	// Each bin is this size, except maybe the last bin (which may be smaller)
	unsigned binSize = 1 + (bins - 1) / width;
	// The maximum number of observations in all bins
	unsigned maxBin = 0;

	// Calculate new binnedHistogram and maxBin
#pragma omp parallel for reduction(max : maxBin)
	for (unsigned i = 0; i < width; i++) {
		binnedHistogram[i] = 0;
		for (unsigned j = binSize * i; j < binSize * (i + 1) && j < bins; j++) {
			binnedHistogram[i] += histogram[j];
		}
		maxBin = std::max(binnedHistogram[i], maxBin);
	}

	// The maximum number of observations each tick can represent
	// May represent as few as 1, if present on the top of a histogram bar
	unsigned tickSize = 1 + (maxBin - 1) / height;

	for (unsigned i = 1; i <= height; i++) {
		unsigned threshold = (height - i) * tickSize;
		for (unsigned j = 0; j < width; j++) {
			if (binnedHistogram[j] > threshold)
				std::cout << '*';
			else
				std::cout << ' ';
		}
		std::cout << '\n';
	}

	delete[] binnedHistogram;
}
