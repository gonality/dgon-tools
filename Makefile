# If Makefiles are supported on your system, then in theory you should be able to compile all
# programs by simply going to the appropriate directory and calling "make". You will need to have
# a C++ compiler installed, and you may need to adjust the CXXFLAGS given below (not sure if these
# are compiler-specific).

CXXFLAGS += -Wall -Wextra -ggdb -O3
CPP_TARGETS=convert_from_graph6 convert_to_graph6 find_gonality subdivision_conjecture

# default target:
all: ${CPP_TARGETS}

convert_from_graph6: graphs.h graph6.h

convert_to_graph6: graphs.h subdivisions.h graph6.h graph_io.h

find_gonality: divisors.h graphs.h subdivisions.h graph_io.h

subdivision_conjecture: divisors.h graphs.h subdivisions.h graph6.h


# Create phony target for clean (see [1]).
#    [1]: https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html#Phony-Targets
.PHONY: clean

# Suppress error messages (see [2]).
#    [2]: https://www.gnu.org/software/make/manual/html_node/Errors.html#Errors
clean:
	-rm -f ${CPP_TARGETS}

