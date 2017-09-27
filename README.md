To compile:

Make sure ncurses is properly installed before trying to compile. Also, both Graph.cpp and Simulation.cpp contain macros at the start that can control certain debug features. Once all relevant macros have been defined, run this:

g++ -std=c++11 Simulation.cpp -lncurses -o Simulation


To run:

Program takes the filename of an adjacency matrix (Simulation.csv) that describes nodes and connections in graph as well as the filename of a table that shows vertex, SoC, and Load in that order (Parameters.csv). This command will run the simulation:

./Simulation Simulation.csv Parameters.csv
