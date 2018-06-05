
o/%.o: %.cpp
	g++ -std=c++17 -c -o $@ $< -Wall -Wextra -Wpedantic -I/opt/local/include -Ifw/include -Iproject/include -Itoolsets/include -I. -g -fsanitize=address  -Iview/include -Ix/include

evoke: $(patsubst %.cpp,o/%.o,$(shell find ./ -name *.cpp))
	g++ -o $@ $^ -L/opt/local/lib -lboost_filesystem -lboost_system -pthread  -g -fsanitize=address 
