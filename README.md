# UNR CS 474 Programming Assignment 1

## Building the assignment
A Makefile is provided in the root directory which makes all executables and images required for the homework by default. Built executables can be found in their corresponding source folders (`Q1-Sampling/`, `Q2-Quantization/`, etc.) and output images can be out in `out/`.

The only prerequisites for default targets (anything required for the homework) are a working `g++` on the path which supports C++ 17 and OpenMP (which is used in questions 3 and 4).

## Building the report
The Makefile does not have a target which generates the report, since different people like to use different engines to generate reports from `.tex` files. I recommend using `pdflatex` with the `--shell-escape` option enabled, which allows the `minted` package to function (more on the below).

The Makefile *does*, however, have a `report` target which will generate all of the neccesary prequisites of the report (such as any output images and diagrams needed by the report). This target has a few more pre-requisites than the default target, however. [Gnuplot](http://www.gnuplot.info/) is used to generate histogram comparisons for questions 3 and 4; and [`pnmtopng`](http://netpbm.sourceforge.net/doc/pnmtopng.html) is used to convert `.pgm` images (unreadable by TeX) into `.png` images. These can both be installed on Debian/Ubuntu by using `sudo apt install gnuplot netpbm` and can be found in prebuilt binary form for Windows on their websites (see earlier links).

The report uses the [`minted`](https://github.com/gpoore/minted) package, which depends on [Pygments](https://github.com/pygments/pygments), a Python-based syntax highlighting library. Since Pygments is Python-based, Python and Pygments itself must be installed manually. Many Python distributions come with Pygments pre-installed, but it can also be installed using [pip](https://github.com/pypa/pip) ala `pip install Pygments`. Make certain it ends up on `PATH`.

## Additional targets
In addition to files required by the homework and for the report, the Makefile includes targets for all sorts of output files:

- `out/%-sampled.pgm` - A sampled `.pgm` image where `%` designates the original sampled image and the sampling rate e.g. `make out/lenna-4-sampled.pgm`

- `out/%-equal.pgm` - A `.pgm` image with an equalized histogram based off of an original image specified by `%` e.g. `make out/f_16-equal.pgm`

- `out/%-histogram-plot.eps` - A `.eps` graphic which compares the histogram of an equalized image with its original histogram specified by `%` e.g. `make out/sf-histogram-plot.eps`. Requires gnuplot.

- `out/%-specify.pgm` - A `.pgm` image based off a specified image with a histogram specified by another image, both specified by `%` e.g. `make out/f_16-peppers-specify.pgm` (in this example the original image is `Images/f_16.pgm` and the histogram is specified by `Images/peppers.pgm`)

- `out/%-specified-plot.eps` - A `.eps` graphic which compares the histogram of an input image, and input histogram, and the histogram of the input image after specifying to the input histogram e.g. `make out/boat-sf-specified-plot.eps`. Requires gnuplot.