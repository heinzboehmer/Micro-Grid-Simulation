#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <iostream>

template<class T>
struct vertex;

template<class T> //Struct that represents adjacent vertices, contains pointer to adjacent vertex and distance from original vertex
struct adjVertex {
  vertex<T> *v;
  float distance; //Used for closest neighbor model
};

template<class T> //Struct that represents vertex, contains name, SoC, load, and level of vertex, as well as vector containing all vertices adjacent to it
struct vertex {
  T name;
  int SoC = -1;
  int load = -1;
  bool needPower = false;
  std::string level;
  std::vector<adjVertex<T>> adj;
};

template<class T>
class Graph {
  public:
    int count = 0; // used to give each city a number/ID
    Graph();
    ~Graph();
    void addEdge(T v1, T v2, float distance); //Adds edge to graph from v1 to v2 with associated distance
    void addVertex(T name); //Adds new vertex to graph
    void displayEdges(); //Displays all vertices and other vertices they are connected to
    void userInputParameters(); //Asks user for parameters of each vertex and sets them
    void setParameters(char *parametersFilename); //Grabs parameters of each vertex fom file and sets them
    void displayParameters(); //Displays parameters of all vertices
    void equalizeChargeNearestNeighbor(); //Simulates nodes sharing power with nearest neighbor approach
    void equalizeChargeLoad(); //Simulates nodes sharing power with load approach
    void printClosestVertex(T name); //Prints name and distance of closest vertex
    void identifyLevels(); //Goes through all vertices and identifies levels based on distances of edges and number of vertices connected

  protected:
  private:
    std::string *output; //Declares string to store output
    std::vector<vertex<T>> vertices; //Declares vector to store vertices
    std::string progressBar(int SoC, int load); //Creates string that represents a progress bar relating to input value
    vertex<T> *findVertex(T name); //Finds vertex based on name
    vertex<T> *findMaxExtraChargeVertex(vertex<T> *v, bool balancingMGCCS); //Finds adjacent vertex with most extra charge
    vertex<T> *findMaxChargeNeedVertex(vertex<T> *v, bool balancingMGCCS); //Finds adjacent vertex with most charge need
    adjVertex<T> findClosestVertex(vertex<T> *v); //Finds closest vertex to inputed vertex
    void createOutput(); //Creates string for output
    void setUpNcurses(); //Sets up ncurses
    void refreshNcurses(); //Refreshes nscurses
    void closeNcurses(); //Closes ncurses
    void recalculateUpperLevelParameters(); //Recalculates parameters for MGCCs and SGCC

};

#include "Graph.cpp"

#endif // GRAPH_H
