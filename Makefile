CXX=g++
TEST_SRC=./test/config.cpp ./test/conversion.cpp
HELPER_SRC=./src/DspHelper.cpp
OUT=./bin/test

test: prep
	$(CXX) $(TEST_SRC) $(HELPER_SRC) -o $(OUT)

prep:
	mkdir -p ./bin