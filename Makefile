#********************************************************************************
# Makefile for the libsnark library.
#********************************************************************************
#* @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
#*             Shaul Kfir, Eran Tromer, Madars Virza.
#* This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
#* @copyright  MIT license (see LICENSE file)
#*******************************************************************************/

CXXFLAGS += -O2 -Wall -Wextra -Wno-unused-parameter -Wno-comment -march=native -mtune=native -std=c++11 -fopenmp

DEPSRC=depsrc
DEPINST=depinst
LDLIBS += -L $(DEPINST)/lib -Wl,-rpath $(DEPINST)/lib -L . -lgmpxx -lgmp -lzm
CXXFLAGS += -I $(DEPINST)/include -I src -I . -DUSE_ASM
DEFAULT_CURVE=BN128
GTESTDIR=/usr/src/gtest

SRCS= \
	src/algebra/curves/alt_bn128/alt_bn128_g1.cpp \
	src/algebra/curves/alt_bn128/alt_bn128_g2.cpp \
	src/algebra/curves/alt_bn128/alt_bn128_init.cpp \
	src/algebra/curves/alt_bn128/alt_bn128_pairing.cpp \
	src/algebra/curves/alt_bn128/alt_bn128_pp.cpp \
	src/common/profiling.cpp \
	src/common/utils.cpp \
	src/gadgetlib2/examples/simple_example.cpp \
	src/gadgetlib2/infrastructure.cpp \
	src/gadgetlib2/adapters.cpp \
	src/gadgetlib2/constraint.cpp \
	src/gadgetlib2/gadget.cpp \
	src/gadgetlib2/integration.cpp \
	src/gadgetlib2/pp.cpp \
	src/gadgetlib2/protoboard.cpp \
	src/gadgetlib2/variable.cpp \
        src/algebra/curves/bn128/bn128_g1.cpp \
        src/algebra/curves/bn128/bn128_g2.cpp \
        src/algebra/curves/bn128/bn128_gt.cpp \
        src/algebra/curves/bn128/bn128_init.cpp \
        src/algebra/curves/bn128/bn128_pairing.cpp \
        src/algebra/curves/bn128/bn128_pp.cpp \
        src/algebra/curves/edwards/edwards_g1.cpp \
        src/algebra/curves/edwards/edwards_g2.cpp \
        src/algebra/curves/edwards/edwards_init.cpp \
        src/algebra/curves/edwards/edwards_pairing.cpp \
        src/algebra/curves/edwards/edwards_pp.cpp

EXECUTABLES= \
	src/r1cs_ppzksnark/examples/profile_r1cs_ppzksnark \
	src/gadgetlib2/examples/tutorial \
	src/gadgetlib2/tests/gadgetlib2_test

DOCS= README.html

OBJS=$(patsubst %.cpp,%.o,$(SRCS))


# Recompile GTest, if we can (e.g., Ubuntu). Otherwise use precompiled one (e.g., Fedora).
# See https://code.google.com/p/googletest/wiki/FAQ#Why_is_it_not_recommended_to_install_a_pre-compiled_copy_of_Goog .
COMPILE_GTEST=$(shell test -d $(GTESTDIR) && echo 1)   # Found GTest sourcecode?
ifeq ($(COMPILE_GTEST),1)
else
	LDFLAGS += -lgtest
endif


# For documentation of the following options, see README.md .

ifeq ($(MINDEPS),1)
	CXXFLAGS += -DMINDEPS
else
	LDLIBS += -lprocps
endif

ifeq ($(LOWMEM),1)
	CXXFLAGS += -DLOWMEM
endif

ifeq ($(STATIC),1)
	CXXFLAGS += -static -DSTATIC
endif

ifeq ($(PROFILE_OP_COUNTS),1)
	CXXFLAGS += -static -DPROFILE_OP_COUNTS
endif

ifeq ($(MULTICORE),1)
	CXXFLAGS += -DMULTICORE
endif

ifeq ($(CPPDEBUG),1)
        CXXFLAGS += -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
        DEBUG = 1
endif

ifeq ($(DEBUG),1)
        CXXFLAGS += -DDEBUG -ggdb3
endif

ifeq ($(PERFORMANCE),1)
        CXXFLAGS += -flto -fuse-linker-plugin
        CXXFLAGS += -march=native -mtune=native
        CXXFLAGS += -DNDEBUG
        LDFLAGS += -flto
endif

ifeq ($(CURVE),)
        CXXFLAGS += -DCURVE_$(DEFAULT_CURVE)
else
        CXXFLAGS += -DCURVE_$(CURVE)
endif

all: $(EXECUTABLES) $(DOCS)

doc: $(DOCS)

# In order to detect changes to #include dependencies. -MMD below generates a .d file for .cpp file. Include the .d file.
-include $(SRCS:.cpp=.d)

$(OBJS): %.o: %.cpp
	$(CXX) -o $@ $< -c -MMD $(CXXFLAGS)

ifeq ($(COMPILE_GTEST),1)
OBJS += $(DEPINST)/gtest-all.o

$(DEPINST)/gtest-all.o: $(GTESTDIR)/src/gtest-all.cc
	$(CXX) -o $@ $< $(CXXFLAGS) -I $(GTESTDIR) -c
endif

src/gadgetlib2/tests/gadgetlib2_test: src/gadgetlib2/tests/adapters_UTEST.cpp src/gadgetlib2/tests/constraint_UTEST.cpp src/gadgetlib2/tests/gadget_UTEST.cpp src/gadgetlib2/tests/protoboard_UTEST.cpp src/gadgetlib2/tests/variable_UTEST.cpp

$(EXECUTABLES): %: %.o $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

$(DOCS): %.html: %.md
	markdown_py -f $@ $^ -x toc -x extra --noisy
#	TODO: Would be nice to enable "-x smartypants" but Ubuntu 12.04 doesn't support that.
#	TODO: switch to redcarpet, to produce same output as GitHub's processing of README.md. But what about TOC?

doxy:
	doxygen doxygen.conf

# Clean generated files, except locally-compiled dependencies
clean:
	$(RM) \
		$(OBJS) \
		$(EXECUTABLES) \
		$(DOCS) \
		${patsubst %,%.o,${EXECUTABLES}} \
		${patsubst %.cpp,%.d,${SRCS}} \
	rm -fr doxygen/

# Clean all, including locally-compiled dependencies
clean-all: clean
	rm -fr $(DEPSRC) $(DEPINST)

.PHONY: all clean clean-all doc doxy
