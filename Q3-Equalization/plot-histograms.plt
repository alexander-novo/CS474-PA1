# A gnuplot plotting file to plot the two histograms of data from equalize with the -p switch
if (!exists("outfile")) outfile='plot.eps'

if (!exists("imageName")) {
	set title "Comparison of histograms of input and output (equalized) images"
} else {
	set title "Comparison of histograms of " . imageName . " and equalized image" noenhanced
}

set terminal postscript eps enhanced color size 6,3
set output outfile
set style data histogram
set style histogram cluster gap 1
set style fill solid
set boxwidth 0.9

unset xtics

plot infile using 2:xtic(1) ti col linecolor rgb "#1b9e77", '' u 3 ti col linecolor rgb "#d95f02"