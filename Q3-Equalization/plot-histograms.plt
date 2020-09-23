if (!exists("outfile")) outfile='plot.eps'

if (!exists("imageName")) {
	set title "Comparison of histograms of input and output (equalized) images"
} else {
	set title "Comparison of histograms of " . imageName . " and equalized image"
}

# set term png size 1280,960
set terminal postscript eps enhanced color
set output outfile
set style data histogram
set style histogram cluster gap 1
set style fill solid
set boxwidth 0.9

unset xtics

plot infile using 2:xtic(1) ti col, '' u 3 ti col