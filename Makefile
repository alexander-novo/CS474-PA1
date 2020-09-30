CXXFLAGS     = -std=c++17 -fopenmp
OBJDIR       = obj
DEPDIR       = $(OBJDIR)/.deps
# Flags which, when added to gcc/g++, will auto-generate dependency files
DEPFLAGS     = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

# Source files - add more to auto-compile into .o files
SOURCES      = Common/image.cpp Common/histogram_tools.cpp Q1-Sampling/main.cpp Q2-Quantization/main.cpp Q3-Equalization/main.cpp Q4-Specification/main.cpp
# Executable targets - add more to auto-make in default 'all' target
EXEC         = Q1-Sampling/sample Q2-Quantization/quantize Q3-Equalization/equalize Q4-Specification/specify
# Targets required for the homework, spearated by question
REQUIRED_1   = out/lenna-2-sampled.pgm out/lenna-4-sampled.pgm out/lenna-8-sampled.pgm out/peppers-2-sampled.pgm out/peppers-4-sampled.pgm out/peppers-8-sampled.pgm
REQUIRED_2   =
REQUIRED_3   = out/boat-equal.pgm out/f_16-equal.pgm
REQUIRED_4   = out/boat-sf-specify.pgm out/f_16-peppers-specify.pgm
REQUIRED_OUT = $(REQUIRED_1) $(REQUIRED_2) $(REQUIRED_3) $(REQUIRED_4)

OBJDIRS      = $(addprefix $(OBJDIR)/, $(dir $(SOURCES)))
DEPDIRS      = $(addprefix $(DEPDIR)/, $(dir $(SOURCES)))
DEPFILES     = $(SOURCES:%.cpp=$(DEPDIR)/%.d)

.PHONY: all clean report

# By default, make all executable targets and the images required for the homework
all: $(EXEC) $(REQUIRED_OUT)

# Executable Targets
Q1-Sampling/sample: $(OBJDIR)/Q1-Sampling/main.o $(OBJDIR)/Common/image.o
	$(CXX) $(CXXFLAGS) $^ -o $@

Q2-Quantization/quantize: $(OBJDIR)/Q2-Quantization/main.o $(OBJDIR)/Common/image.o
	$(CXX) $(CXXFLAGS) $^ -o $@

Q3-Equalization/equalize: $(OBJDIR)/Q3-Equalization/main.o $(OBJDIR)/Common/image.o $(OBJDIR)/Common/histogram_tools.o
	$(CXX) $(CXXFLAGS) $^ -o $@

Q4-Specification/specify: $(OBJDIR)/Q4-Specification/main.o $(OBJDIR)/Common/image.o $(OBJDIR)/Common/histogram_tools.o
	$(CXX) $(CXXFLAGS) $^ -o $@

### Question 1 Outputs ###
.SECONDEXPANSION:
out/%-sampled.pgm: Q1-Sampling/sample Images/$$(word 1,$$(subst -, ,$$*)).pgm | out
	Q1-Sampling/sample Images/$(word 1,$(subst -, ,$*)).pgm $@ $(word 2,$(subst -, ,$*))

### Question 3 Outputs ###
# Generate equalized images and histogram plotting data from equalize
out/%-equal.pgm out/%-histograms.dat: Q3-Equalization/equalize Images/%.pgm | out
	Q3-Equalization/equalize Images/$*.pgm out/$*-equal.pgm -p out/$*-histograms.dat

# Generate histogram plot from histogram plotting data
out/%-histogram-plot.eps: out/%-histograms.dat Q3-Equalization/plot-histograms.plt
	gnuplot -e "infile='$<'"\
	        -e "outfile='$@'"\
	        -e "imageName='$*.pgm'"\
	        Q3-Equalization/plot-histograms.plt

### Question 4 Outputs ###
# Generate specified images and histogram comparison plotting data from specify
# Outputs depend on *two* input images, so outputs encode the inputs with a 
# '-' separated list of input images, e.g. out/boat-sf-specify.pgm depends on
# Images/boat.pgm and out/sf-histograms.dat. .SECONDEXPANSION facilitates this,
# allowingus to do some commands on the stem %, such as separating into a list
# of words, for prerequisites.
.SECONDEXPANSION:
out/%-specify.pgm out/%-compare-histograms.dat: Q4-Specification/specify Images/$$(word 1,$$(subst -, ,$$*)).pgm out/$$(word 2,$$(subst -, ,$$*))-histograms.dat
	Q4-Specification/specify Images/$(word 1,$(subst -, ,$*)).pgm\
	                         out/$(word 2,$(subst -, ,$*))-histograms.dat\
	                         out/$*-specify.pgm\
	                         -p out/$*-compare-histograms.dat

.SECONDEXPANSION:
out/%-specified-plot.eps: out/%-compare-histograms.dat Q4-Specification/plot-histograms.plt
	gnuplot -e "infile='$<'"\
	        -e "outfile='$@'"\
	        -e "imageName='$(word 1,$(subst -, ,$*)).pgm'"\
	        -e "histoName='$(word 2,$(subst -, ,$*)).pgm'"\
	        Q4-Specification/plot-histograms.plt

# Figures needed for the report
report: out/boat-histogram-plot.eps out/f_16-histogram-plot.eps out/boat-sf-specified-plot.eps
report: Images/boat.png out/boat-equal.png Images/f_16.png out/f_16-equal.png out/boat-sf-specify.png 

clean:
	rm -rf $(OBJDIR)
	rm -f $(EXEC)
	rm -rf out
	rm -f Images/*.png

# Generate .png images from .pgm images. Needed for report, since pdfLaTeX doesn't support .pgm images
%.png: %.pgm
	pnmtopng $< > $@

# Auto-Build .cpp files into .o
$(OBJDIR)/%.o: %.cpp
$(OBJDIR)/%.o: %.cpp $(DEPDIR)/%.d | $(DEPDIRS) $(OBJDIRS)
	$(CXX) $(DEPFLAGS) $(CXXFLAGS) -c $< -o $@

# Make generated directories
$(DEPDIRS) $(OBJDIRS) out: ; @mkdir -p $@
$(DEPFILES):
include $(wildcard $(DEPFILES))