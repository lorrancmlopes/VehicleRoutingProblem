#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <chrono>
#include <limits>
#include <omp.h>

using namespace std;
using namespace std::chrono;

// Função para gerar permutações
void generate_permutations(vector<int>& nodes, vector<vector<int>>& permutations) {
    sort(nodes.begin(), nodes.end()); // Ordena os nós para garantir todas as permutações
    do {
        // Adiciona 0 no início e no fim de cada permutação
        vector<int> perm = {0};
        perm.insert(perm.end(), nodes.begin(), nodes.end());
        perm.push_back(0);
        permutations.push_back(perm);
    } while (next_permutation(nodes.begin(), nodes.end()));
}

// Função para ajustar permutações com base nas restrições
void adjust_permutations(vector<vector<int>>& permutations, const vector<vector<int>>& adj_matrix, const unordered_map<int, int>& demands, int vehicle_capacity, int num_stops) {
    #pragma omp parallel for
    for (size_t perm_index = 0; perm_index < permutations.size(); ++perm_index) {
        auto& perm = permutations[perm_index];
        int current_capacity = 0;
        int stops = 0;

        for (auto it = perm.begin() + 1; it != perm.end() - 1; ++it) {
            int from = *(it - 1);
            int to = *it;

            // Verifica se não há caminho entre dois nós consecutivos
            if (adj_matrix[from][to] == -1) {
                it = perm.insert(it, 0);
                current_capacity = 0; // Reinicia a capacidade após retorno ao depósito
                stops = 0; // Reinicia o contador de paradas
                continue;
            }

            // Verifica se a capacidade do veículo é excedida
            if (demands.find(to) != demands.end()) {
                current_capacity += demands.at(to);
                stops++; // Incrementa o contador de paradas
            }

            if (current_capacity > vehicle_capacity || stops > num_stops) {
                if (*(it - 1) != 0) { // Evita inserções repetidas de retornos ao depósito
                    it = perm.insert(it, 0);
                    current_capacity = demands.at(to); // Reinicia a capacidade com a demanda do nó atual
                    stops = 1; // Reinicia o contador de paradas com a parada atual
                }
            }
        }
    }
}

// Função para encontrar a rota de menor custo
vector<int> find_min_cost_route(const vector<vector<int>>& permutations, const vector<vector<int>>& adj_matrix) {
    vector<int> min_cost_route;
    int min_cost = numeric_limits<int>::max();

    #pragma omp parallel
    {
        vector<int> local_min_cost_route;
        int local_min_cost = numeric_limits<int>::max();

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
            if (valid_route && cost < local_min_cost) {
                local_min_cost = cost;
                local_min_cost_route = perm;
            }
        }

        #pragma omp critical
        {
            if (local_min_cost < min_cost) {
                min_cost = local_min_cost;
                min_cost_route = local_min_cost_route;
            }
        }
    }

    cout << "Menor custo encontrado: " << min_cost << endl;
    return min_cost_route;
}

int main() {
    auto start = high_resolution_clock::now();

    ifstream infile("grafo.txt");
    if (!infile) {
        cerr << "Não foi possível abrir o arquivo grafo.txt" << endl;
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

    vector<vector<int>> permutations;
    generate_permutations(nodes, permutations);

    int vehicle_capacity = 15;
    int num_stops = 5;

    adjust_permutations(permutations, adj_matrix, demands, vehicle_capacity, num_stops);

    vector<int> min_cost_route = find_min_cost_route(permutations, adj_matrix);

    cout << "Rota de menor custo: ";
    for (int node : min_cost_route) {
        cout << node << " ";
    }
    cout << endl;

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    cout << "Tempo total de execução: " << duration << " ms." << endl;

    return 0;
}
