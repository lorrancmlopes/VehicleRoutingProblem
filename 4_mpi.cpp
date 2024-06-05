#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <chrono>
#include <limits>
#include <omp.h>
#include <mpi.h>

using namespace std;
using namespace std::chrono;

// Function to generate permutations
void generate_permutations(vector<int>& nodes, vector<vector<int>>& permutations) {
    sort(nodes.begin(), nodes.end()); // Sort nodes to ensure all permutations
    do {
        vector<int> perm = {0};
        perm.insert(perm.end(), nodes.begin(), nodes.end());
        perm.push_back(0);
        permutations.push_back(perm);
    } while (next_permutation(nodes.begin(), nodes.end()));
}

// Function to adjust permutations based on constraints
void adjust_permutations(vector<vector<int>>& permutations, const vector<vector<int>>& adj_matrix, const unordered_map<int, int>& demands, int vehicle_capacity, int num_stops) {
    #pragma omp parallel for
    for (size_t perm_index = 0; perm_index < permutations.size(); ++perm_index) {
        auto& perm = permutations[perm_index];
        int current_capacity = 0;
        int stops = 0;

        for (auto it = perm.begin() + 1; it != perm.end() - 1; ++it) {
            int from = *(it - 1);
            int to = *it;

            // Check if there is no path between consecutive nodes
            if (adj_matrix[from][to] == -1) {
                it = perm.insert(it, 0);
                current_capacity = 0; // Reset capacity after returning to depot
                stops = 0; // Reset stop counter
                continue;
            }

            // Check if vehicle capacity is exceeded
            if (demands.find(to) != demands.end()) {
                current_capacity += demands.at(to);
                stops++; // Increment stop counter
            }

            if (current_capacity > vehicle_capacity || stops > num_stops) {
                if (*(it - 1) != 0) { // Avoid repeated depot returns
                    it = perm.insert(it, 0);
                    current_capacity = demands.at(to); // Reset capacity with current node demand
                    stops = 1; // Reset stop counter with current stop
                }
            }
        }
    }
}

// Function to find the minimum cost route
vector<int> find_min_cost_route(const vector<vector<int>>& permutations, const vector<vector<int>>& adj_matrix, int& local_min_cost) {
    vector<int> local_min_cost_route;
    local_min_cost = numeric_limits<int>::max();

    #pragma omp parallel
    {
        vector<int> thread_min_cost_route;
        int thread_min_cost = numeric_limits<int>::max();

        #pragma omp for nowait
        for (size_t i = 0; i < permutations.size(); ++i) {
            const auto& perm = permutations[i];
            int cost = 0;
            bool valid_route = true;
            for (size_t j = 0; j < perm.size() - 1; ++j) {
                int from = perm[j];
                int to = perm[j + 1];
                if (adj_matrix[from][to] == -1) {
                    valid_route = false;
                    break;
                }
                cost += adj_matrix[from][to];
            }
            if (valid_route && cost < thread_min_cost) {
                thread_min_cost = cost;
                thread_min_cost_route = perm;
            }
        }

        #pragma omp critical
        {
            if (thread_min_cost < local_min_cost) {
                local_min_cost = thread_min_cost;
                local_min_cost_route = thread_min_cost_route;
            }
        }
    }

    return local_min_cost_route;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    auto start = high_resolution_clock::now();

    ifstream infile("grafo.txt");
    if (!infile) {
        cerr << "Cannot open grafo.txt" << endl;
        MPI_Finalize();
        return 1;
    }

    int num_nodes;
    infile >> num_nodes;

    unordered_map<int, int> demands;
    for (int i = 1; i < num_nodes; ++i) {
        int node, demand;
        infile >> node >> demand;
        demands[node] = demand;
    }

    vector<vector<int>> adj_matrix(num_nodes, vector<int>(num_nodes, -1));
    int num_edges;
    infile >> num_edges;
    for (int i = 0; i < num_edges; ++i) {
        int from, to, weight;
        infile >> from >> to >> weight;
        adj_matrix[from][to] = weight;
    }

    infile.close();

    vector<int> nodes;
    for (int i = 1; i < num_nodes; ++i) {
        nodes.push_back(i);
    }

    vector<vector<int>> all_permutations;

    // Generate permutations
    generate_permutations(nodes, all_permutations);

    // Split permutations among MPI processes
    int chunk_size = all_permutations.size() / world_size;
    int start_index = world_rank * chunk_size;
    int end_index = (world_rank == world_size - 1) ? all_permutations.size() : start_index + chunk_size;

    vector<vector<int>> local_permutations(all_permutations.begin() + start_index, all_permutations.begin() + end_index);

    int vehicle_capacity = 15;
    int num_stops = 5;

    // Adjust permutations
    adjust_permutations(local_permutations, adj_matrix, demands, vehicle_capacity, num_stops);

    // Find minimum cost route
    int local_min_cost;
    vector<int> local_min_cost_route = find_min_cost_route(local_permutations, adj_matrix, local_min_cost);

    // Gather all results
    int global_min_cost;
    MPI_Allreduce(&local_min_cost, &global_min_cost, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);

    // Determine which process found the global minimum
    int min_cost_rank = (local_min_cost == global_min_cost) ? world_rank : -1;
    int global_min_cost_rank;
    MPI_Allreduce(&min_cost_rank, &global_min_cost_rank, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

    // Communicate the size of the route before sending the actual route
    int local_min_cost_route_size = local_min_cost_route.size();
    MPI_Bcast(&local_min_cost_route_size, 1, MPI_INT, global_min_cost_rank, MPI_COMM_WORLD);

    // Resize global_min_cost_route to the correct size
    vector<int> global_min_cost_route(local_min_cost_route_size, 0);

    // Send the local minimum cost route from the process that found it
    if (world_rank == global_min_cost_rank) {
        MPI_Send(local_min_cost_route.data(), local_min_cost_route_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    // Root process receives the global minimum cost route
    if (world_rank == 0) {
        MPI_Recv(global_min_cost_route.data(), local_min_cost_route_size, MPI_INT, global_min_cost_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        cout << "Menor custo encontrado: " << global_min_cost << endl;
        cout << "Rota de menor custo: ";
        for (int node : global_min_cost_route) {
            cout << node << " ";
        }
        cout << endl;

        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start).count();
        cout << "Tempo total de execução: " << duration << " ms." << endl;
    }

    MPI_Finalize();
    return 0;
}
