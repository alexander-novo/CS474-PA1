#ifndef IMAGE_H
#define IMAGE_H

#include <iostream>
// a simple example - you would need to add more funtions

class Image {
public:
	typedef unsigned char pixelT;
	struct ImageHeader {
		enum Type {
			COLOR,
			GRAY,
		} type;

		unsigned M, N, Q;

		static ImageHeader read(std::istream &in);
	};

	Image();
	Image(unsigned, unsigned, unsigned);
	Image(const Image &);
	Image(Image &&);
	~Image();

	static Image read(std::istream &in);

	friend std::ostream &operator<<(std::ostream &out, const Image &im);

	pixelT *operator[](unsigned i);
	const pixelT *operator[](unsigned i) const;

	pixelT *const &pixels  = pixelValue;
	const unsigned &rows   = M;
	const unsigned &cols   = N;
	const unsigned &maxVal = Q;

private:
	Image(unsigned, unsigned, unsigned, pixelT *);
	unsigned M, N, Q;
	pixelT *pixelValue;
};

std::ostream &operator<<(std::ostream &out, const Image::ImageHeader &head);

#endif
