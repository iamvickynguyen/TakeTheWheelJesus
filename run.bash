echo This can take some time, there are over 30000 lines of code to compile in the libraries...
clang++ -pthread -std=c++17 -o main main.cpp

echo "Build complete"

echo "Running snake..."
./main

