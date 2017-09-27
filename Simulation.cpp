#include <iostream>
#include <fstream>
#include <sstream>
#include <ncurses.h>
#include "Graph.h"

#define DEBUG
// #define NEARESTNEIGHBOR

using namespace std;

Graph<string> g;

int debugMenu(char *parametersFilename) { //Displays menu with options and calls functions according to user input

  cout << "\n========Debug Menu========\n1. Print vertices\n2. Display Parameters\n3. Find Closest "
          << "Station\n4. Set Parameters Manually\n5. Equalize Charge\n6. Quit" << endl;

  int choice;
  cin >> choice;

  switch (choice) {

    case 1: {
      g.displayEdges();

      return 0;
    }

    case 2: {
      g.displayParameters();

      return 0;
    }

    case 3: {
      string startVertex;

      cout << "Enter station to search from: ";
      cin.ignore();
      getline(cin, startVertex);

      g.printClosestVertex(startVertex);

      return 0;
    }

    case 4: {
      cin.ignore();
      g.userInputParameters();

      return 0;
    }

    case 5: {
      #ifdef NEARESTNEIGHBOR
      g.equalizeChargeNearestNeighbor();
      #endif

      #ifndef NEARESTNEIGHBOR
      g.equalizeChargeLoad();
      #endif

      return 0;
    }

    case 6: {
      return 1;
    }

    default: return 0;

  }

}

void menu() { //Equalizes charge

  string temp;

  cout << "Press enter to equalize charge." << endl;

  getline(cin, temp);

  #ifdef NEARESTNEIGHBOR
  g.equalizeChargeNearestNeighbor();
  #endif

  #ifndef NEARESTNEIGHBOR
  g.equalizeChargeLoad();
  #endif

}

void buildGraph(char *filename) { //Builds graph from adjacency matrix

  ifstream inFile; //open file
	inFile.open(filename);

  vector<string> cities; //Creates vector to store city names
  string word;
  string station;
  string fromStation;
  string toStation;
  string strDistance;
  string tmp;
  float distance;

  int index=0;

  while(getline(inFile, tmp, '\n')) { //Reads one line at a time until end of file
    stringstream ssLine(tmp);
    getline(ssLine, word, ','); //Separates line into words

    if (word == "stations") { //Checks if this line is the first line
      while (getline(ssLine, station, ',')) { //Goes through all of words in line
        cities.push_back(station); //Adds station name to vector containing all city names
        g.addVertex(station); //Adds station to graph as a vertex
      }
      continue; //Goes to next iteration of while loop
    }

    else { //Line must not be the first line
      fromStation = word;
      index = 0;

      while (getline(ssLine, strDistance, ',')) { //Goes through all of words in line

        distance = stof(strDistance); //Converts distance string to float

        if(distance != -1 && distance !=0) { //Checks if distance is 0 (same station) or -1 (no path)
          toStation = cities[index]; //Finds station it is connected to
          g.addEdge(fromStation, toStation, distance); //Adds edge
        }

        index++;
      }
    }
  }

}

int main(int argc, char *argv[]) { //Program takes filename of adjacency matrix as command line argument

  char *filename=argv[1];
  char *parametersFilename=argv[2];

  buildGraph(filename); //Pass filename to build graph function
  g.identifyLevels(); //Automatically assigns levels to stations
  g.setParameters(parametersFilename); //Sets parameters for stations from parameters file

  #ifdef DEBUG
  while (debugMenu(parametersFilename)!=1) {} //Runs debug menu function
  #endif

  #ifndef DEBUG
  menu(); //Runs menu function
  #endif

  return 0;
}
