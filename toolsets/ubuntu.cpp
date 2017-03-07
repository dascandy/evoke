



obj/%.o: %.cpp
        g++ -c -o $@ $< -Wall -Wextra -Wpedantic  -Icompile -Ifw -Iload -Iproject -Itoolsets

x_: $(patsubst %.cpp,obj/%.o,$(shell find ./ -name *.cpp))
        g++ -o $@ $^ -lboost_filesystem -lboost_system

