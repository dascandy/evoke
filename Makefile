
BOOST_INCLUDE_DIR ?= /opt/local/include
BOOST_LIB_DIR ?= /opt/local/lib
CXX ?= g++

o/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) -std=c++17 -g2 -c -o $@ $< -Wall -Wextra -Wpedantic -I$(BOOST_INCLUDE_DIR) -Iutils/include -Ifw/include -Iproject/include -Itoolsets/include -I. -Iview/include -Ievoke/include -Ireporter/include -Inetwork/include

build/gcc/bin/evoke: bin/evoke_make
	bin/evoke_make
	@echo Build complete. Please copy build/gcc/bin/evoke to wherever you want to install it.

bin/evoke_make: $(patsubst %.cpp,o/%.o,$(shell find . -name *.cpp -not -regex ".*/test/.*"))
	@mkdir -p $(dir $@)
	$(CXX) -g2 -std=c++17 -pthread -o $@ $^ -L$(BOOST_LIB_DIR) -lboost_filesystem -lboost_system -Wl,-rpath -Wl,$(BOOST_LIB_DIR)

clean:
	@mkdir -p $(dir $@)
	@rm -f $(patsubst %.cpp,o/%.o,$(shell find ./ -name *.cpp))
	@rm -f bin/evoke_make
