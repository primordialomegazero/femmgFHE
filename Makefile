# FEmmG-FHE Makefile
CXX = g++
CXXFLAGS = -std=c++17 -O3 -march=native
INCLUDES = -I./openfhe-development/src/pke/include \
           -I./openfhe-development/src/core/include \
           -I./openfhe-development/src/binfhe/include \
           -I./openfhe-development/build/src/core \
           -I./src
LIBS = -L./openfhe-development/build/lib \
       -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe \
       -Wl,-rpath,./openfhe-development/build/lib \
       -lstdc++ -lpthread -lm

# Active tests (from tests/active/)
TESTS = test_phi_complete test_phi_clean_cycle test_phi_ckks_trace \
        test_phi_ckks_ultra test_phi_vs_bootstrap test_phi_stress \
        test_phi_prescale test_phi_32768 test_phi_bootfree_v2 \
        test_phi_deep_lite test_phi_gauntlet test_phi_unlimited \
        test_phi_sprint test_phi_asymmetric_clean test_phi_pure_mulx \
        test_phi_binet test_snc_verify test_phi_final

.PHONY: all clean

all: $(TESTS)

define TEST_TEMPLATE
$(1): tests/active/$(1).cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o bin/$(1) tests/active/$(1).cpp $(LIBS)
endef

$(foreach t,$(TESTS),$(eval $(call TEST_TEMPLATE,$(t))))

clean:
	rm -f bin/*
	rm -f gauntlet_log.txt
	rm -f core*

distclean: clean
	rm -rf openfhe-development/build/*
