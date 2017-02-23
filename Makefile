


obj/%.o: src/%.cpp
	g++ -c -o $@ $<

x: $(patsubst src/%.cpp,obj/%.o,$(shell ls src/*.cpp))
	g++ -o $@ $^ -lboost_filesystem -lboost_system
