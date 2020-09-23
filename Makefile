CPPFLAGS = -std=c++17 -fopenmp
CC       = g++
OBJDIR   = obj
DEPDIR   = $(OBJDIR)/.deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
SOURCES  = Common/image.cpp Q1-Sampling/main.cpp Q2-Quantization/main.cpp Q3-Equalization/main.cpp Q4-Specification/main.cpp
TARGETS  = Q1-Sampling/sample Q2-Quantization/quantize Q3-Equalization/equalize Q4-Specification/specify
OBJDIRS  = $(addprefix $(OBJDIR)/, $(dir $(SOURCES)))
DEPDIRS  = $(addprefix $(DEPDIR)/, $(dir $(SOURCES)))
DEPFILES = $(SOURCES:%.cpp=$(DEPDIR)/%.d)

.PHONY: all clean report

all: $(TARGETS)

Q1-Sampling/sample: $(OBJDIR)/Q1-Sampling/main.o $(OBJDIR)/Common/image.o
	$(CC) $(CPPFLAGS) $^ -o $@

Q2-Quantization/quantize: $(OBJDIR)/Q2-Quantization/main.o $(OBJDIR)/Common/image.o
	$(CC) $(CPPFLAGS) $^ -o $@

Q3-Equalization/equalize: $(OBJDIR)/Q3-Equalization/main.o $(OBJDIR)/Common/image.o
	$(CC) $(CPPFLAGS) $^ -o $@

Q4-Specification/specify: $(OBJDIR)/Q4-Specification/main.o $(OBJDIR)/Common/image.o
	$(CC) $(CPPFLAGS) $^ -o $@

out/%-equal.pgm out/%-histograms.dat: Q3-Equalization/equalize Images/%.pgm | out
	Q3-Equalization/equalize Images/$*.pgm out/$*-equal.pgm -p out/$*-histograms.dat

out/%-histogram-plot.eps: out/%-histograms.dat Q3-Equalization/plot-histograms.plt
	gnuplot -e "infile='out/$*-histograms.dat'" -e "outfile='out/$*-histogram-plot.eps'" -e "imageName='$*.pgm'" Q3-Equalization/plot-histograms.plt

report: out/boat-histogram-plot.eps Images/boat.png out/boat-equal.png out/f_16-histogram-plot.eps Images/f_16.png out/f_16-equal.png 

clean:
	rm -rf $(OBJDIR)
	rm -f $(TARGETS)
	rm -rf out
	rm -f Images/*.png

%.png: %.pgm
	pnmtopng $< > $@

# Auto-Build .cpp files into .o
$(OBJDIR)/%.o: %.cpp
$(OBJDIR)/%.o: %.cpp $(DEPDIR)/%.d | $(DEPDIRS) $(OBJDIRS)
	$(CC) $(DEPFLAGS) $(CPPFLAGS) -c $< -o $@

$(DEPDIRS) $(OBJDIRS) out: ; @mkdir -p $@
$(DEPFILES):
include $(wildcard $(DEPFILES))