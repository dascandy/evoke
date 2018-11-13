
BOOST_INCLUDE_DIR = /opt/local/include
BOOST_LIB_DIR = /opt/local/lib
CXX=g++

o/%.o: %.cpp
        @mkdir -p $(dir $@)
        $(CXX) -std=c++17 -O3 -c -o $@ $< -Wall -Wextra -Wpedantic -I$(BOOST_INCLUDE_DIR) -Ifw/include -Iproject/include -Itoolsets/include -I. -Iview/include -Ievoke/include

bin/evoke: bin/evoke_make
	bin/evoke_make
	@echo Build complete. Please copy bin/evoke to wherever you want to install it.

bin/evoke_make: $(patsubst %.cpp,o/%.o,$(shell find ./ -name *.cpp))
        @mkdir -p $(dir $@)
        $(CXX) -O3 -std=c++17 -pthread -o $@ $^ -L$(BOOST_LIB_DIR) -lboost_filesystem -lboost_system -Wl,-rpath $(BOOST_LIB_DIR)

clean:
        @rm -f $(patsubst %.cpp,o/%.o,$(shell find ./ -name *.cpp))
        @rm -f bin/evoke_make
