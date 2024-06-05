import os
import re
import pandas as pd
import matplotlib.pyplot as plt

def extract_execution_times(folder):
    times = {
        'nodes': [],
        'global_search': [],
        'heuristic': [],
        'openmp': [],
        'mpi_openmp': []
    }
    
    for subdir, dirs, files in os.walk(folder):
        files = sorted(files)  # Ensure files are sorted
        if len(files) == 5:  # Ensure the folder contains all necessary files
            print(f"Processing directory: {subdir}")
            
            # Extract number of nodes from the folder name
            match = re.search(r"(\d+)nos", subdir)
            if match:
                nodes = int(match.group(1))
                times['nodes'].append(nodes)
                print(f"Number of nodes: {nodes}")
            else:
                print(f"No match for nodes in directory name: {subdir}")
                continue
            
            # Check if the files are in the expected order
            for i, file in enumerate(files):
                if i == 0:
                    if not file.endswith(".txt"):
                        print(f"Expected .txt file but found: {file}")
                        break
                else:
                    if not file.endswith(".out"):
                        print(f"Expected .out file but found: {file}")
                        break
            
            # Extract times from the SLURM output files in the given order
            for i, file in enumerate(files[1:]):
                print(f"Processing file: {file}")
                if file.endswith(".out"):
                    with open(os.path.join(subdir, file)) as f:
                        content = f.read()
                        match = re.search(r"Tempo total de execução: (\d+) ms", content)
                        if match:
                            execution_time = int(match.group(1))
                            print(f"Found execution time: {execution_time} ms in file: {file}")
                            if i == 0:
                                times['global_search'].append(execution_time)
                            elif i == 1:
                                times['heuristic'].append(execution_time)
                            elif i == 2:
                                times['openmp'].append(execution_time)
                            elif i == 3:
                                times['mpi_openmp'].append(execution_time)
                        else:
                            print(f"No execution time found in file: {file}")
        else:
            print(f"Skipping directory: {subdir}, does not contain exactly 5 files")
    return times

def compile_data(times):
    print("Compiling data into DataFrame...")
    df = pd.DataFrame(times)
    df.sort_values(by='nodes', inplace=True)
    print("Data compiled successfully.")
    return df

def generate_graphs(df):
    print("Generating graphs...")
    plt.figure(figsize=(12, 8))

    plt.plot(df['nodes'], df['global_search'], marker='o', label='Global Search')
    plt.plot(df['nodes'], df['heuristic'], marker='o', label='Heuristic')
    plt.plot(df['nodes'], df['openmp'], marker='o', label='OpenMP')
    plt.plot(df['nodes'], df['mpi_openmp'], marker='o', label='MPI + OpenMP')

    plt.xlabel('Number of Nodes')
    plt.ylabel('Execution Time (ms)')
    plt.title('Execution Time vs. Number of Nodes')
    plt.legend()
    plt.grid(True)
    plt.savefig('execution_times_comparison.png')
    plt.show()
    print("Graphs generated and saved successfully.")


data_folder = '/home/lorran/Documentos/Projeto (1)'
execution_times = extract_execution_times(data_folder)
print("Execution times extracted:")
print(execution_times)
df = compile_data(execution_times)
print("DataFrame content:")
print(df)
generate_graphs(df)
