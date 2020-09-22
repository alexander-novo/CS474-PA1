CPPFLAGS = -std=c++17
CC       = g++
OBJDIR   = obj
DEPDIR   = $(OBJDIR)/.deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
SOURCES  = Common/image.cpp Q1-Sampling/main.cpp Q2-Quantization/main.cpp Q3-Equalization/main.cpp Q4-Specification/main.cpp
TARGETS  = Q1-Sampling/sample Q2-Quantization/quantize Q3-Equalization/equalize Q4-Specification/specify
OBJDIRS  = $(addprefix $(OBJDIR)/, $(dir $(SOURCES)))
DEPDIRS  = $(addprefix $(DEPDIR)/, $(dir $(SOURCES)))
DEPFILES = $(SOURCES:%.cpp=$(DEPDIR)/%.d)

.PHONY: all clean

all: $(TARGETS)

Q1-Sampling/sample: $(OBJDIR)/Q1-Sampling/main.o $(OBJDIR)/Common/image.o
	$(CC) $(CPPFLAGS) $^ -o $@

Q2-Quantization/quantize: $(OBJDIR)/Q2-Quantization/main.o $(OBJDIR)/Common/image.o
	$(CC) $(CPPFLAGS) $^ -o $@

Q3-Equalization/equalize: $(OBJDIR)/Q3-Equalization/main.o $(OBJDIR)/Common/image.o
	$(CC) $(CPPFLAGS) $^ -o $@

Q4-Specification/specify: $(OBJDIR)/Q4-Specification/main.o $(OBJDIR)/Common/image.o
	$(CC) $(CPPFLAGS) $^ -o $@

clean:
	rm -r $(OBJDIR)
	rm $(TARGETS)

# Auto-Build .cpp files into .o
$(OBJDIR)/%.o: %.cpp
$(OBJDIR)/%.o: %.cpp $(DEPDIR)/%.d | $(DEPDIRS) $(OBJDIRS)
	$(CC) $(DEPFLAGS) $(CPPFLAGS) -c $< -o $@

$(DEPDIRS) $(OBJDIRS): ; @mkdir -p $@
$(DEPFILES):
include $(wildcard $(DEPFILES))