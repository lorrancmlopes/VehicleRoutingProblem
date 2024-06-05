#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <chrono>
#include <limits>

using namespace std;
using namespace std::chrono;

// Função para gerar permutações
void generate_permutations(vector<int>& nodes, vector<vector<int>>& permutations) {
    //cout << "Gerando permutações..." << endl;

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
    //cout << "Ajustando permutações com base nas restrições..." << endl;

    for (size_t perm_index = 0; perm_index < permutations.size(); ++perm_index) {
        auto& perm = permutations[perm_index];
        int current_capacity = 0;
        int stops = 0;

        for (auto it = perm.begin() + 1; it != perm.end() - 1; ++it) {
            int from = *(it - 1);
            int to = *it;

            //cout << "Verificando permutação " << perm_index + 1 << " nó de " << from << " para " << to << endl;

            // Verifica se não há caminho entre dois nós consecutivos
            if (adj_matrix[from][to] == -1) {
                //cout << "Sem caminho entre " << from << " e " << to << ", inserindo retorno ao depósito" << endl;
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
                    //cout << "Capacidade ou número de paradas excedido após nó " << to << ", inserindo retorno ao depósito" << endl;
                    it = perm.insert(it, 0);
                    current_capacity = demands.at(to); // Reinicia a capacidade com a demanda do nó atual
                    stops = 1; // Reinicia o contador de paradas com a parada atual
                } else {
                    //cout << "Já retornado ao depósito anteriormente, continuando." << endl;
                }
            }
        }

        //cout << "Permutação ajustada: ";
        /*for (int node : perm) {
            cout << node << " ";
        }
        cout << endl;*/

    }

}

// Função para encontrar a rota de menor custo
vector<int> find_min_cost_route(const vector<vector<int>>& permutations, const vector<vector<int>>& adj_matrix) {
    //cout << "Analisando todas as rotas possíveis..." << endl;

    vector<int> min_cost_route;
    int min_cost = numeric_limits<int>::max();

    for (const auto& perm : permutations) {
        int cost = 0;
        bool valid_route = true;
        for (size_t i = 0; i < perm.size() - 1; ++i) {
            int from = perm[i];
            int to = perm[i + 1];
            if (adj_matrix[from][to] == -1) {
                valid_route = false;
                break;
            }
            cost += adj_matrix[from][to];
        }
        if (valid_route && cost < min_cost) {
            min_cost = cost;
            min_cost_route = perm;
        }
    }

    cout << "Menor custo encontrado: " << min_cost << endl;

    return min_cost_route;
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

    // Gera todas as permutações possíveis dos nós (excluindo o depósito)
    vector<int> nodes;
    for (int i = 1; i < num_nodes; ++i) {
        nodes.push_back(i);
    }

    vector<vector<int>> permutations;
    generate_permutations(nodes, permutations);

    // Capacidade do veículo e número máximo de paradas (exemplo)
    int vehicle_capacity = 15;
    int num_stops = 5;

    // Ajusta as permutações com base nas restrições
    adjust_permutations(permutations, adj_matrix, demands, vehicle_capacity, num_stops);

    // Encontra a rota de menor custo
    vector<int> min_cost_route = find_min_cost_route(permutations, adj_matrix);

    // Imprime a rota de menor custo
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