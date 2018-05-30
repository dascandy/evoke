
obj/%.o: %.cpp
	g++ -std=c++17 -c -o $@ $< -Wall -Wextra -Wpedantic  -Icompile -Ifw -Iload -Iproject -Itoolsets -I. -g

evoke: $(patsubst %.cpp,obj/%.o,$(shell find ./ -name *.cpp))
	g++ -o $@ $^ -lboost_filesystem -lboost_system -pthread  -g
