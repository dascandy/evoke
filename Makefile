


obj/%.o: src/%.cpp
	g++ -c -o $@ $< -Wall -Wextra -Wpedantic 

x: $(patsubst src/%.cpp,obj/%.o,$(shell ls src/*.cpp))
	g++ -o $@ $^ -lboost_filesystem -lboost_system
