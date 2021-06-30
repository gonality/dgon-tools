# If Makefiles are supported on your system, then in theory you should be able to compile all
# programs by simply going to the appropriate directory and calling "make". You will need to have
# a C++ compiler installed, and you may need to adjust the CXXFLAGS given below (not sure if these
# are compiler-specific).

CXXFLAGS += --std=c++11 -Wall -Wextra -pedantic -ggdb -O2
CPP_TARGETS=convert_from_graph6 convert_to_graph6 find_gonality subdivision_conjecture

# default target:
all: ${CPP_TARGETS}

convert_from_graph6: convert_from_graph6.cpp graphs.h graph6.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $@.cpp -o $@

convert_to_graph6: convert_to_graph6.cpp graphs.h subdivisions.h graph6.h graph_io.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $@.cpp -o $@

find_gonality: find_gonality.cpp divisors.h graphs.h subdivisions.h graph_io.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $@.cpp -o $@

subdivision_conjecture: subdivision_conjecture.cpp divisors.h graphs.h subdivisions.h graph6.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $@.cpp -o $@


# Create phony target for clean (see [1]).
#    [1]: https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html#Phony-Targets
.PHONY: clean

# Suppress error messages (see [2]).
#    [2]: https://www.gnu.org/software/make/manual/html_node/Errors.html#Errors
clean:
	-rm -f ${CPP_TARGETS}

