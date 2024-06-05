import os
import re
from networkx import DiGraph, set_node_attributes
from vrpy import VehicleRoutingProblem

# Função para ler o arquivo do grafo
def ler_arquivo_grafo(caminho_arquivo):
    with open(caminho_arquivo, 'r') as arquivo:
        N = int(arquivo.readline().strip()) - 1
        demandas = {}
        for _ in range(N):
            linha = arquivo.readline().strip().split()
            id_no, demanda = int(linha[0]), int(linha[1])
            demandas[id_no] = demanda

        K = int(arquivo.readline().strip())
        arestas = []
        for _ in range(K):
            linha = arquivo.readline().strip().split()
            origem, destino, peso = int(linha[0]), int(linha[1]), int(linha[2])
            arestas.append((origem, destino, peso))

    return demandas, arestas

# Função para extrair o custo e a rota dos arquivos de saída
def extract_cost_and_route(filename):
    with open(filename, 'r') as file:
        content = file.read()
    cost_match = re.search(r'(Menor custo encontrado|Custo da rota): (\d+)', content)
    route_match = re.search(r'Rota (de menor custo|aproximada de menor custo): ([\d\s]+)', content)
    if cost_match and route_match:
        cost = int(cost_match.group(2))
        route = list(map(int, route_match.group(2).strip().split()))
        return cost, route
    return None, None

# Função para validar a rota e o custo usando o vrpy
def validate_route_and_cost(demandas, arestas, route, vehicle_capacity, num_stops):
    G = DiGraph()
    for inicio, fim, custo in arestas:
        if inicio == 0: inicio = "Source"
        if fim == 0: fim = "Sink"
        G.add_edge(inicio, fim, cost=custo)

    set_node_attributes(G, values=demandas, name="demand")

    prob = VehicleRoutingProblem(G, load_capacity=vehicle_capacity)
    prob.num_stops = num_stops
    prob.solve()

    best_routes = prob.best_routes
    best_value = prob.best_value

    return best_routes, best_value

# Caminho para o diretório base
base_dir = '/home/lorran/Documentos/Projeto (1)'

# Configurações de capacidade do veículo e número máximo de paradas
vehicle_capacity = 15
num_stops = 5

# Arquivo de saída
output_file = 'validation_results.txt'

# Abre o arquivo de saída para escrita
with open(output_file, 'w') as f:
    # Itera sobre cada subpasta no diretório base
    for folder in sorted(os.listdir(base_dir)):
        folder_path = os.path.join(base_dir, folder)
        if os.path.isdir(folder_path):
            grafo_path = os.path.join(folder_path, 'grafo.txt')
            if os.path.exists(grafo_path):
                f.write(f"Processing folder: {folder}\n")
                # Ler demandas e arestas do grafo
                demandas, arestas = ler_arquivo_grafo(grafo_path)

                out_files = sorted([f for f in os.listdir(folder_path) if f.endswith('.out')])
                for out_file in out_files:
                    out_file_path = os.path.join(folder_path, out_file)
                    cost, route = extract_cost_and_route(out_file_path)
                    if cost is not None and route is not None:
                        f.write(f"File: {out_file}\n")
                        f.write(f"Extracted cost: {cost}\n")
                        f.write(f"Extracted route: {route}\n")
                        best_routes, best_value = validate_route_and_cost(demandas, arestas, route, vehicle_capacity, num_stops)
                        f.write(f"Best routes: {best_routes}\n")
                        f.write(f"Best value: {best_value}\n")
                        f.write(f"Valid route cost: {best_value}\n")
                        f.write(f"Match found: {cost == best_value}\n")
                        f.write("-----\n")