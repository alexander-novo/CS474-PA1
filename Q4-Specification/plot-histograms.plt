# Q4-Specification/plot-histograms.plt
# A gnuplot plotting file to plot the three histograms of data from specify with the -p switch
if (!exists("outfile")) outfile='plot.eps'

if (!exists("imageName") || !exists("histoName")) {
	set title "Comparison of histograms of input image, input histogram, and output image"
} else {
	set title "Comparison of histograms of " . imageName . ", " . histoName . ", and specified image" noenhanced
}

set terminal postscript eps enhanced color size 6,3
set output outfile
set style data histogram
set style histogram cluster gap 1
set style fill solid
set boxwidth 0.9

unset xtics

# Colors chosen using ColorBrewer 2.0 qualitative scheme "Dark2"
# https://colorbrewer2.org/#type=qualitative&scheme=Dark2&n=3
plot infile using 2:xtic(1) ti imageName noenhanced linecolor rgb "#1b9e77",\
	'' u 3 ti histoName noenhanced linecolor rgb "#d95f02",\
	'' u 4 ti col linecolor rgb "#7570b3"