CXX=g++
CXXFLAGS=-std=c++23 -ggdb3 -Og -Wconversion -Wpedantic -Wall -Wextra -Werror -fsanitize=address,undefined -Wno-unused-result

SRCS=shell.cpp Command.cpp Tokenizer.cpp
OBJS=$(patsubst %.cpp,%.o,${SRCS})
BINS=shell

all: $(BINS)

$(BINS): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) -I. $(CXXFLAGS) -c $< -o $@


.PHONY: clean test

clean:
	@rm -f $(BINS) $(OBJS) a b test.txt output.txt out.trace ./test-files/cmd.txt

test:
	@chmod u+x pa2-tests.sh
	@./pa2-tests.sh
