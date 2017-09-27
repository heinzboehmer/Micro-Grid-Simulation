#include <iostream>
#include <queue>
#include <cmath>
#include <chrono>
#include <thread>
#include "Graph.h"

using namespace std;

#define MGCC_TO_SGCC_MIN_DISTANCE 10
#define LOCAL_TO_MGCC_MAX_DISTANCE 0.5
#define LOCAL_STATION_MAX_NUMBER_CONNECTIONS 1
// #define BALANCEMGCCS
// #define STEPBYSTEP

template <class T>
Graph<T>::Graph() {

  output = new string;

}

template <class T>
Graph<T>::~Graph() {

  delete output;

}

template <class T>
void Graph<T>::addEdge(T v1, T v2, float distance) { //Adds edge to graph from v1 to v2 with associated distance

  for(int i = 0; i < vertices.size(); i++){
    if(vertices[i].name == v1){ //Look for first vertex
      for(int j = 0; j < vertices.size(); j++){
        if(vertices[j].name == v2 && i != j){ //Look for second vertex
            adjVertex<T> av; //New instance of adjVertex struct
            av.v = &vertices[j];
            av.distance = distance; //Used for closest neighbor model
            vertices[i].adj.push_back(av); //Add to adjacent vertices vector
        }
      }
    }
  }

}

template <class T>
void Graph<T>::addVertex(T name) { //Adds new vertex to graph

  bool found = false;

  for(int i = 0; i < vertices.size(); i++){ //Checks if vertex already exists
    if(vertices[i].name == name){
      found = true;
    }
  }

  if(found == false){
    vertex<T> v; //New instance of vertex struct
    v.name = name;
    vertices.push_back(v); //Add to vertices vector
  }

}

template <class T>
void Graph <T>::displayEdges() { //Displays all vertices and other vertices they are connected to

  for(int i = 0; i < vertices.size(); i++){ //Loop through vertices vector
    cout << vertices[i].name << "(" << vertices[i].level << ") --> ";

    for(int j = 0; j < vertices[i].adj.size(); j++){ //Loop through adjacent vertices vector for each vertex
      cout << vertices[i].adj[j].v->name << "(" << vertices[i].adj[j].distance << ")";
      if(j != vertices[i].adj.size() - 1) cout << " and ";
    }

    cout << endl;
  }

}

template <class T>
void Graph <T>::userInputParameters() { //Asks user for parameters of each vertex and sets them

  vertex <T> *vertex;
  string stringSoC;
  string stringLoad;
  string vertexString;
  int SoC;
  int load;

  while (true){
    cout << "\nEnter vertex name: ";

    getline(cin, vertexString);

    vertex = findVertex(vertexString);

    if (vertex == NULL) cout << "Please enter a valid vertex name." << endl;
    else break;
  }

  cout << "\nEnter SoC and Load for vertex. Leave blank to keep current values\n" << endl;

  if (vertex->level == "Local") { //Only locals can have user defined parameters
    cout << "Enter SoC for " << vertex->name << ": ";

    while (true) {
      getline(cin, stringSoC);

      if (stringSoC == "") break;

      SoC = stoi(stringSoC);

      if (SoC > 100 || SoC < 0) { //Checks if SoC entered is valid
        cout << "SoC out of range (0-100). Please try again." << endl;
        cout << "Enter SoC for " << vertex->name << ": ";
        continue;
      }

      else {
        vertex->SoC = SoC;
        break;
      }
    }

    cout << "Enter load for " << vertex->name << ": ";

    while (true) {
      getline(cin, stringLoad);

      if (stringLoad == "") break;

      load = stoi(stringLoad);

      if (load > 100 || load < 0) { //Checks if SoC entered is valid
        cout << "Load out of range (0-100). Please try again." << endl;
        cout << "Enter load for " << vertex->name << ": ";
        continue;
      }

      else {
        vertex->load = load;
        break;
      }
    }
  }

  else cout << "Can't edit non-local vertex parameters." << endl;

  recalculateUpperLevelParameters();

}

template <class T>
void Graph <T>::setParameters(char *parametersFilename) { //Grabs parameters of each vertex from file and sets them

  ifstream inFile; //open file
	inFile.open(parametersFilename);

  string tmp;
  string word;
  float SoC;
  float load;
  int index;
  vertex<T> *vertex;

  while(getline(inFile, tmp, '\n')) { //Reads one line at a time until end of file
    stringstream ssLine(tmp);
    vertex = NULL;
    index = 0;

    while (getline(ssLine, word, ',')) { //Reads line word by word
      if (index == 0) {
        vertex = findVertex(word);

        if (vertex == NULL) {
          cout << "Error encountered when reading parameters file. One or more vertex names do not match adjacency matrix." << endl;
          return;
        }
      }

      else if (index == 1) {
        vertex->SoC = stof(word);
      }

      else if (index == 2) {
        vertex->load = stof(word);
      }

      else {
        cout << "Error encountered when reading parameters file. One or more fields do not match expected format. (Name,SoC,Load)" << endl;
        return;
      }

      index++;
    }
  }

  recalculateUpperLevelParameters();

}

template <class T>
void Graph <T>::displayParameters() { //Displays parameters of all vertices

  for (int i = 0; i < vertices.size(); i++) { //Concatenates values for every vertex
    if (vertices[i].SoC == -1 || vertices[i].load == -1) {
      cout << "One or more parameters for " + vertices[i].name + " have not been defined." << endl;
      return;
    }
  }

  setUpNcurses();

  createOutput();

  closeNcurses();

}

template <class T>
void Graph <T>::equalizeChargeNearestNeighbor() { //Simulates nodes sharing power

  vertex<T> *closestVertex [vertices.size()]; //Declares array with same size as vertices vector
  bool isBalanced = false;

  setUpNcurses();

  for (int i = 0; i < vertices.size(); i++) { //Creates array of closest vertex for each vertex
    if (vertices[i].SoC == -1 || vertices[i].load == -1) {
      cout << "One or more parameters for " + vertices[i].name + " have not been defined." << endl;
      return;
    }

    closestVertex[i] = findClosestVertex(&vertices[i]).v;
  }

  while (!isBalanced) { //Continues balancing until system is balanced
    isBalanced = true;
    *output = "";

    for (int i = 0; i < vertices.size(); i++) { //Loops through all vertices and decides if each should give or take power from their nearest neighbor
      if (vertices[i].SoC - closestVertex[i]->SoC > 1) { //More charge on node than on neighbor
        vertices[i].SoC -= 1;
        closestVertex[i]->SoC += 1;
        isBalanced = false;
      }

      else if (closestVertex[i]->SoC - vertices[i].SoC > 1) { //More charge on neighbor than on node
        vertices[i].SoC += 1;
        closestVertex[i]->SoC -= 1;
        isBalanced = false;
      }

      createOutput();
    }

    refreshNcurses();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  closeNcurses();

}

template <class T>
void Graph<T>::equalizeChargeLoad() {

  bool isBalanced = false;
  bool powerNeeded = false;
  bool powerAvailable = false;
  int prevSoC[vertices.size()];
  vertex <T> *maxChargeVertex = NULL;
  vertex <T> *maxNeedVertex = NULL;

  setUpNcurses();

  while (!isBalanced) { //Continues balancing until locals are balanced
    isBalanced = true;
    maxChargeVertex = NULL;

    for (int i = 0; i < vertices.size(); i++) { //Loops through all adjacent vertices to each MGCC
      if (vertices[i].level == "MGCC") { //Makes sure to only balance locals for each MGCC
        powerAvailable = false;

        for (int j = 0; j < vertices[i].adj.size(); j++) { //Loops through all adjacent vertices and sets flags
          if (vertices[i].adj[j].v->level != "SGCC") { //Skips SGCC
            vertices[i].adj[j].v->needPower = false;

            if (vertices[i].adj[j].v->load >= vertices[i].adj[j].v->SoC) vertices[i].adj[j].v->needPower = true;

            else powerAvailable = true; //Checks if vertices have enough power for balancing
          }
        }

        if (powerAvailable) { //Does not try to balance unless power is available
          for (int k = 0; k < vertices[i].adj.size(); k++) { //Loops through all adjacent vertices and balances
            maxChargeVertex = findMaxExtraChargeVertex(&vertices[i], false); //Finds new vertex with max charge, might be the same as last time

            if (vertices[i].adj[k].v->level != "SGCC" && vertices[i].adj[k].v->needPower) { //Skips SGCC and balances
              vertices[i].adj[k].v->SoC += 1;
              maxChargeVertex->SoC -= 1;
              isBalanced = false;
            }
          }
        }
      }
    }

    createOutput();

    refreshNcurses();

    #ifdef STEPBYSTEP
    printw("\n\nPress any key to step forward");
    getch(); //Wait for user input
    #endif

    #ifndef STEPBYSTEP
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    #endif
  }

  isBalanced = false;

  #ifdef BALANCEMGCCS
  while (!isBalanced) { //Continues balancing until MGCCs are balanced
    isBalanced = true;
    powerNeeded = false;
    maxChargeVertex = NULL;
    maxNeedVertex = NULL;

    for (int l = 0; l < vertices.size(); l++) { // Loops through all adjacent vertices to each SGCC
      if (vertices[l].level == "SGCC") { //Makes sure to only balance MGCCs for each SGCC
        powerAvailable = false;

        for (int m = 0; m < vertices[l].adj.size(); m++) { //Loops through all adjacent vertices and sets flags
          vertices[l].adj[m].v->needPower = false;

          if (vertices[l].adj[m].v->load > vertices[l].adj[m].v->SoC){
            vertices[l].adj[m].v->needPower = true;
            powerNeeded = true;
          }

          else powerAvailable = true; //Checks if vertices have enough power for balancing

          if (!powerNeeded) { //Makes sure vertices under each MGCC are also balanced
            for (int n = 0; n < vertices[l].adj[m].v->adj.size(); n++) {
              if (vertices[l].adj[m].v->adj[n].v->load > vertices[l].adj[m].v->adj[n].v->SoC){
                vertices[l].adj[m].v->adj[n].v->needPower = true;
                powerNeeded = true;
              }
            }
          }
        }

        if (powerAvailable && powerNeeded) { //Does not try to balance unless enough power is available and power is needed
          maxChargeVertex = findMaxExtraChargeVertex(findMaxExtraChargeVertex(&vertices[l], true), false);
          maxNeedVertex = findMaxChargeNeedVertex(findMaxChargeNeedVertex(&vertices[l], true), false);

          if (maxChargeVertex->SoC != maxChargeVertex->load) { //Makes sure not to take charge if max extra charge vertex is at its limit
            maxNeedVertex->SoC += 1;
            maxChargeVertex->SoC -= 1;
            isBalanced = false;
          }

          recalculateUpperLevelParameters();
        }
      }
    }

    createOutput();

    refreshNcurses();

    #ifdef STEPBYSTEP
    printw("\n\nPress any key to step forward");
    getch(); //Wait for user input
    #endif

    #ifndef STEPBYSTEP
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    #endif
  }
  #endif

  createOutput();

  closeNcurses();

}

template <class T>
void Graph <T>::recalculateUpperLevelParameters() { //Recalculates parameters for MGCCs and SGCC

  float averageSoCSum;
  float averageLoadSum;

  for (int i = 0; i < vertices.size(); i++) {
    if (vertices[i].level == "MGCC") {
      vertices[i].SoC = 0; //Changes SoC from not set (-1) to 0
      averageLoadSum = 0;
      averageSoCSum = 0;

      for (int j = 0; j < vertices[i].adj.size(); j++) { //Loops through all adjacent vertices and identifies SoC and load
        if (vertices[i].adj[j].v->level != "SGCC") {
          averageSoCSum += vertices[i].adj[j].v->SoC;
          averageLoadSum += vertices[i].adj[j].v->load;
        }
      }
      vertices[i].SoC = averageSoCSum / (vertices[i].adj.size() - 1); //Subtract 1 to account for edge to SGCC
      vertices[i].load = averageLoadSum / (vertices[i].adj.size() - 1);
    }
  }

  for (int i = 0; i < vertices.size(); i++) {
    if (vertices[i].level == "SGCC") {
      vertices[i].SoC = 0; //Changes SoC from not set (-1) to 0
      averageLoadSum = 0;
      averageSoCSum = 0;

      for (int j = 0; j < vertices[i].adj.size(); j++) { //Loops through all adjacent vertices and identifies SoC and load
        averageSoCSum += vertices[i].adj[j].v->SoC;
        averageLoadSum += vertices[i].adj[j].v->load;
      }
      vertices[i].SoC = averageSoCSum / vertices[i].adj.size();
      vertices[i].load = averageLoadSum / vertices[i].adj.size();
    }
  }

}

template <class T>
vertex<T> *Graph <T>::findMaxExtraChargeVertex(vertex<T> *v, bool balancingMGCCS) { //Finds adjacent vertex with most extra charge

  int maxCharge;
  vertex<T> *maxChargeVertex;

  for (int i = 0; i < v->adj.size(); i++) { //Sets initial SoC to compare to but makes sure this isn't from an SGCC or MGCC
    if (v->adj[i].v->level != "SGCC" && (v->adj[i].v->level != "MGCC" || balancingMGCCS)) {
      maxCharge = v->adj[i].v->SoC - v->adj[i].v->load;
      maxChargeVertex = v->adj[i].v;
      break;
    }
  }

  for (int j = 0; j < v->adj.size(); j++) {
    if ((v->adj[j].v->SoC - v->adj[j].v->load) > maxCharge && v->adj[j].v->level != "SGCC" && (v->adj[j].v->level != "MGCC" || balancingMGCCS)) {
      maxCharge = v->adj[j].v->SoC - v->adj[j].v->load;
      maxChargeVertex = v->adj[j].v;
    }
  }

  return maxChargeVertex;

}

template <class T>
vertex<T> *Graph <T>::findMaxChargeNeedVertex(vertex<T> *v, bool balancingMGCCS) { //Finds adjacent vertex with most charge need

  int maxNeed;
  vertex<T> *maxNeedVertex;

  for (int i = 0; i < v->adj.size(); i++) { //Sets initial SoC to compare to but makes sure this isn't from an SGCC or MGCC
    if (v->adj[i].v->level != "SGCC" && (v->adj[i].v->level != "MGCC" || balancingMGCCS)) {
      maxNeed = v->adj[i].v->SoC - v->adj[i].v->load;
      maxNeedVertex = v->adj[i].v;
      break;
    }
  }

  for (int j = 0; j < v->adj.size(); j++) {
    if ((v->adj[j].v->SoC - v->adj[j].v->load) < maxNeed && v->adj[j].v->level != "SGCC" && (v->adj[j].v->level != "MGCC" || balancingMGCCS)) {
      maxNeed = v->adj[j].v->SoC - v->adj[j].v->load;
      maxNeedVertex = v->adj[j].v;
    }
  }

  return maxNeedVertex;

}

template <class T>
adjVertex<T> Graph <T>::findClosestVertex(vertex<T> *v) { //Finds closest vertex to inputed vertex

  float minDistance = v->adj[0].distance;
  adjVertex<T> closest = v->adj[0];

  for (int i = 0; i < v->adj.size(); i++) { //Loops through all adjacent vertices and finds closest one
    if (v->adj[i].distance < minDistance){
      minDistance = v->adj[i].distance;
      closest = v->adj[i];
    }

  }

  return closest; //Returns adjVertex struct so that both distance and all other vertex properties are available

}

template <class T>
void Graph <T>::printClosestVertex(T name) { //Prints name and distance of closest vertex

  vertex<T> *v = findVertex(name);

  if (v == NULL) {
    cout << "Station not found" << endl;
    return;
  }

  adjVertex<T> closest = findClosestVertex(v);

  cout << "The closest station is " << closest.v->name << " at a distance of " << closest.distance << " km" << endl;

}

template <class T>
vertex<T> *Graph <T>::findVertex(T name) { //Finds vertex based on name

  for(int i = 0; i < vertices.size(); i++) {
    if(vertices[i].name == name) return &vertices[i];
  }

  return NULL;

}

template <class T>
string Graph <T>::progressBar(int SoC, int load) { //Creates string that represents a progress bar relating to input value

  if (SoC <= load) {
    return "[" + string(round(SoC / 2.0), '|') + string(round(load / 2.0) - round(SoC / 2.0), '*') + string(50 - round(load / 2.0), ' ') + "]";
  }

  else {
    return "[" + string(round(load / 2.0), '*') + string(round(SoC / 2.0) - round(load / 2.0), '|') + string(50 - round(SoC / 2.0), ' ') + "]";
  }

}

template <class T>
void Graph <T>::createOutput() { //Creates string for output

  *output = "|: SoC\t*: Load\n\n";

  for (int i = 0; i < vertices.size(); i++) {
    if (vertices[i].level == "SGCC") { //Print SGCC at the top
      *output += vertices[i].name + " - SoC: " + to_string(vertices[i].SoC) + "%%\tLoad: " +
                to_string(vertices[i].load) + "  \n" + progressBar(vertices[i].SoC, vertices[i].load)
                + string(2, '\n');

      for (int j = 0; j < vertices[i].adj.size(); j++) {
        if (vertices[i].adj[j].v->level == "MGCC") { //Print MGCCs next
          *output += "\n\n\t" + vertices[i].adj[j].v->name + " - SoC: " +
                    to_string(vertices[i].adj[j].v->SoC) + "%%\tLoad: " +
                    to_string(vertices[i].adj[j].v->load) + "  \n\t" +
                    progressBar(vertices[i].adj[j].v->SoC, vertices[i].adj[j].v->load)
                    + string(2, '\n');

          for (int k = 0; k < vertices[i].adj[j].v->adj.size(); k++) { //Prints locals last
            if (vertices[i].adj[j].v->adj[k].v->level == "Local") {
              *output += "\t\t" + vertices[i].adj[j].v->adj[k].v->name + " - SoC: " +
                        to_string(vertices[i].adj[j].v->adj[k].v->SoC) + "%%\tLoad: " +
                        to_string(vertices[i].adj[j].v->adj[k].v->load) + "  \n\t\t" +
                        progressBar(vertices[i].adj[j].v->adj[k].v->SoC, vertices[i].adj[j].v->adj[k].v->load)
                        + string(2, '\n');
            }
          }
        }
      }
    }
  }

}

template <class T>
void Graph <T>::setUpNcurses() {

  initscr(); //Sets up ncurses
  noecho();
  curs_set(false);
  clear();

}

template <class T>
void Graph <T>::refreshNcurses() {

  clear();
  printw(output->c_str());
  refresh();

}

template <class T>
void Graph <T>::closeNcurses() {

  clear();
  printw(output->c_str()); //Prints final concatenated string
  printw("\n\nPress any key to continue");
  getch(); //Wait for user input
  endwin();

}

template <class T>
void Graph <T>::identifyLevels() {

  bool isSGCC;
  bool isLocal;

  for (int i = 0; i < vertices.size(); i++) { //Loops through vertices
    isSGCC = true;
    isLocal = true;


    if (vertices[i].adj.size() > LOCAL_STATION_MAX_NUMBER_CONNECTIONS) { //Checks if vertex has more than min number of adjacent vertices for local vertex
      isLocal = false;
    }

    for (int j = 0; j < vertices[i].adj.size(); j++) { //Loops through all adjacent vertices and identifies level
      if (vertices[i].adj[j].distance < MGCC_TO_SGCC_MIN_DISTANCE) { //Checks if all adjacent vertices are farther than MGCC to SGCC min distance
        isSGCC = false;
      }
    }

    if (isSGCC && !isLocal) vertices[i].level = "SGCC"; //Assigns level to current vertex
    else if (!isSGCC && isLocal) vertices[i].level = "Local";
    else if (!isSGCC && !isLocal) vertices[i].level = "MGCC";
    else vertices[i].level = "Unknown";
  }

}
