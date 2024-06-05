#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <chrono>
#include <limits>

using namespace std;
using namespace std::chrono;

// Função para encontrar a posição de menor custo para inserir um nó
pair<int, int> find_best_insertion(const vector<int>& route, int new_node, const vector<vector<int>>& adj_matrix) {
    int min_cost_increase = numeric_limits<int>::max();
    int best_position = -1;

    for (size_t i = 0; i < route.size() - 1; ++i) {
        int from = route[i];
        int to = route[i + 1];

        // Verifica se há caminho válido
        if (adj_matrix[from][new_node] != -1 && adj_matrix[new_node][to] != -1) {
            int cost_increase = adj_matrix[from][new_node] + adj_matrix[new_node][to] - adj_matrix[from][to];

            if (cost_increase < min_cost_increase) {
                min_cost_increase = cost_increase;
                best_position = i + 1;
            }
        }
    }

    return {best_position, min_cost_increase};
}

// Heurística de Inserção Mais Próxima
vector<int> nearest_insertion(const vector<vector<int>>& adj_matrix, int vehicle_capacity, const unordered_map<int, int>& demands, int num_stops) {
    int num_nodes = adj_matrix.size();
    vector<int> route = {0, 0};  // Inicia com o depósito
    vector<bool> visited(num_nodes, false);
    visited[0] = true;
    int current_capacity = 0;
    int current_stops = 0;

    while (true) {
        int best_node = -1;
        int best_position = -1;
        int min_cost_increase = numeric_limits<int>::max();

        for (int i = 1; i < num_nodes; ++i) {
            if (!visited[i]) {
                auto [position, cost_increase] = find_best_insertion(route, i, adj_matrix);
                if (position != -1 && cost_increase < min_cost_increase) {
                    best_node = i;
                    best_position = position;
                    min_cost_increase = cost_increase;
                }
            }
        }

        if (best_node == -1) break; // Todos os nós foram visitados

        route.insert(route.begin() + best_position, best_node);
        visited[best_node] = true;
        current_capacity += demands.at(best_node);
        current_stops++;

        // Verifica se a capacidade do veículo ou o número de paradas é excedido
        if (current_capacity > vehicle_capacity || current_stops >= num_stops) {
            route.insert(route.begin() + best_position + 1, 0); // Retorna ao depósito
            current_capacity = 0; // Reinicia a capacidade
            current_stops = 0; // Reinicia o número de paradas
        }
    }

    // Garante que todos os nós sejam visitados
    for (int i = 1; i < num_nodes; ++i) {
        if (!visited[i]) {
            bool inserted = false;
            for (size_t j = 0; j < route.size() - 1; ++j) {
                int from = route[j];
                int to = route[j + 1];
                if (adj_matrix[from][i] != -1 && adj_matrix[i][to] != -1) {
                    route.insert(route.begin() + j + 1, i);
                    visited[i] = true;
                    current_capacity += demands.at(i);
                    current_stops++;
                    if (current_capacity > vehicle_capacity || current_stops >= num_stops) {
                        route.insert(route.begin() + j + 2, 0); // Retorna ao depósito
                        current_capacity = 0;
                        current_stops = 0;
                    }
                    inserted = true;
                    break;
                }
            }
            if (!inserted) {
                for (size_t j = 0; j < route.size() - 1; ++j) {
                    int from = route[j];
                    if (adj_matrix[from][i] != -1) {
                        route.insert(route.begin() + j + 1, i);
                        visited[i] = true;
                        route.insert(route.begin() + j + 2, 0); // Retorna ao depósito
                        break;
                    }
                }
            }
        }
    }

    // Remover retornos ao depósito redundantes (caminhos 0 0)
    vector<int> clean_route;
    clean_route.push_back(0);
    for (size_t i = 1; i < route.size(); ++i) {
        if (route[i] != 0 || (route[i] == 0 && route[i - 1] != 0)) {
            clean_route.push_back(route[i]);
        }
    }

    // Verificação final para garantir que todos os nós foram visitados
    for (int i = 1; i < num_nodes; ++i) {
        if (!visited[i]) {
            cerr << "Erro: Não foi possível inserir o nó " << i << " na rota." << endl;
        }
    }

    return clean_route;
}

// Função para calcular o custo total de uma rota
int calculate_route_cost(const vector<int>& route, const vector<vector<int>>& adj_matrix) {
    int cost = 0;
    for (size_t i = 0; i < route.size() - 1; ++i) {
        int from = route[i];
        int to = route[i + 1];
        if (adj_matrix[from][to] == -1) {
            cerr << "Erro: Caminho inválido de " << from << " para " << to << endl;
            return numeric_limits<int>::max(); // Rota inválida
        }
        cost += adj_matrix[from][to];
    }
    return cost;
}

int main() {
    //cout << "Iniciando leitura do arquivo..." << endl;
    auto start = high_resolution_clock::now();

    ifstream infile("grafo.txt");
    if (!infile) {
        cerr << "Não foi possível abrir o arquivo grafo.txt" << endl;
        return 1;
    }

    int num_nodes;
    infile >> num_nodes;

    // Mapa para armazenar demandas
    unordered_map<int, int> demands;

    // Lê as demandas
    //cout << "Lendo demandas..." << endl;
    for (int i = 1; i < num_nodes; ++i) {
        int node, demand;
        infile >> node >> demand;
        demands[node] = demand;
    }

    //cout << "Demandas lidas." << endl;

    // Inicializa a matriz de adjacência
    vector<vector<int>> adj_matrix(num_nodes, vector<int>(num_nodes, -1));

    // Lê as arestas
    //cout << "Lendo arestas..." << endl;
    int num_edges;
    infile >> num_edges;

    for (int i = 0; i < num_edges; ++i) {
        int from, to, weight;
        infile >> from >> to >> weight;
        adj_matrix[from][to] = weight;
    }

    infile.close();
    //cout << "Arestas lidas." << endl;

    // Capacidade do veículo e número máximo de paradas (exemplo)1
    int vehicle_capacity = 15;
    int num_stops = 5;

    // Encontra a rota usando a heurística de inserção mais próxima
    vector<int> min_cost_route = nearest_insertion(adj_matrix, vehicle_capacity, demands, num_stops);

    // Calcula o custo da rota
    int min_cost = calculate_route_cost(min_cost_route, adj_matrix);

    // Imprime a rota de menor custo e seu custo
    cout << "Custo da rota: " << min_cost << endl;
    cout << "Rota aproximada de menor custo: ";
    for (int node : min_cost_route) {
        cout << node << " ";
    }
    cout << endl;

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    cout << "Tempo total de execução: " << duration << " ms." << endl;

    return 0;
}
