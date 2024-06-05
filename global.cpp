#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream> // Include the <fstream> header file

struct Edge {
    int source;
    int destination;
    int weight;
};

struct Node {
    int id;
    int demand;
};

std::map<int, int> demandMap;
std::vector<Edge> edges;

void readGraphFromFile(const std::string& filePath) {
    

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cout << "Failed to open file: " << filePath << std::endl;
        return;
    }

    int numNodes;
    file >> numNodes;

    for (int i = 0; i < numNodes; i++) {
        int nodeId, demand;
        file >> nodeId >> demand;
        demandMap[nodeId] = demand;
    }

    int numEdges;
    file >> numEdges;

    for (int i = 0; i < numEdges; i++) {
        int source, destination, weight;
        file >> source >> destination >> weight;
        edges.push_back({source, destination, weight});
    }

    file.close();
}

int main() {
    std::string filePath = "grafo.txt";
    readGraphFromFile(filePath);

    // Construct the graph
    std::map<int, std::vector<std::pair<int, int>>> graph;
    for (const auto& edge : edges) {
        graph[edge.source].push_back({edge.destination, edge.weight});
    }

    // Set node demands
    std::map<int, int> nodeDemands;
    for (const auto& demand : demandMap) {
        nodeDemands[demand.first] = demand.second;
    }

    // Solve the VRP
    int loadCapacity = 15;  // Can be modified
    int maxStops = 5;       // Can be modified

    // TODO: Implement the global search algorithm to solve the VRP with demand constraints
    

    return 0;
}