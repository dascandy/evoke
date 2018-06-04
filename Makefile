
obj/%.o: %.cpp
	g++ -std=c++17 -c -o $@ $< -Wall -Wextra -Wpedantic -I/opt/local/include -Icompile -Ifw -Iload -Iproject -Itoolsets -I. -g -fsanitize=address  -Iview

evoke: $(patsubst %.cpp,obj/%.o,$(shell find ./ -name *.cpp))
	g++ -o $@ $^ -L/opt/local/lib -lboost_filesystem -lboost_system -pthread  -g -fsanitize=address 
