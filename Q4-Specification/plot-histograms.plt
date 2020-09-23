# A gnuplot plotting file to plot the three histograms of data from specify with the -p switch
if (!exists("outfile")) outfile='plot.eps'

if (!exists("imageName")) {
	set title "Comparison of histograms of input image, input histogram, and output image"
} else {
	set title "Comparison of histograms of " . imageName . ", " . histoName . ", and specified image"
}

set terminal postscript eps enhanced color
set output outfile
set style data histogram
set style histogram cluster gap 1
set style fill solid
set boxwidth 0.9

unset xtics

plot infile using 2:xtic(1) ti col linecolor rgb "#1b9e77", '' u 3 ti col linecolor rgb "#d95f02", '' u 4 ti col linecolor rgb "#7570b3"