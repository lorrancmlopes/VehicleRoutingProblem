# Vehicle Routing Problem (VRP) - SuperComp Project

### The complete PDF is here: [Projeto - SuperComp.pdf](./Projeto%20-%20SuperComp.pdf)

## Project Overview

This project focuses on solving the Vehicle Routing Problem (VRP) using various computational methods, including exhaustive search, heuristic approaches, and parallel computing with OpenMP and MPI. The goal is to implement, validate, and compare these methods to find the most efficient solution for VRP.

## Table of Contents
1. [Implementations](#implementations)
   - Exhaustive Search (1_brute_force.cpp)
   - Heuristic (2_heuristica.cpp)
   - OpenMP Implementation (3_openMP.cpp)
   - MPI + OpenMP Implementation (4_mpi.cpp)
2. [Validation of Implementations](#validation)
3. [Evaluation of Results](#evaluation)

## Implementations

### Exhaustive Search (1_brute_force.cpp)
This implementation explores all possible routes to find the one with the lowest cost, considering vehicle capacity and the maximum number of stops.

**Steps:**
1. **Reading the Graph:**
   - Reads a graph from `grafo.txt` containing nodes, demands, and edge weights.
2. **Generating Permutations:**
   - Generates all possible permutations of nodes, ensuring each starts and ends at the depot.
3. **Adjusting Permutations:**
   - Adjusts permutations to meet capacity and stop constraints.
4. **Finding Minimum Cost Route:**
   - Calculates the cost of each adjusted permutation to find the one with the lowest cost.

### Heuristic (2_heuristica.cpp)
Uses the Nearest Insertion heuristic to construct a route iteratively, inserting the nearest unvisited node in the most cost-effective position.

**Steps:**
1. **Reading the Graph:**
   - Reads graph details from `grafo.txt`.
2. **Finding Best Insertion Position:**
   - Identifies the best position to insert a new node to minimize route cost.
3. **Nearest Insertion Heuristic:**
   - Constructs the route iteratively by inserting the nearest unvisited node.
4. **Calculating Route Cost:**
   - Computes the total cost of the constructed route.

### OpenMP Implementation (3_openMP.cpp)
Parallelizes parts of the exhaustive search using OpenMP to reduce execution time.

**Key Differences:**
1. **Parallelizing Permutation Adjustment:**
   - Uses `#pragma omp parallel for` to adjust permutations concurrently.
2. **Parallelizing Minimum Cost Search:**
   - Utilizes OpenMP directives to parallelize the search for the minimum cost route, ensuring safe updates with `#pragma omp critical`.

### MPI + OpenMP Implementation (4_mpi.cpp)
Combines MPI for inter-process communication and OpenMP for intra-process parallelism to distribute the workload across multiple machines and cores.

**Key Differences:**
1. **Introducing MPI:**
   - Initializes MPI environment and distributes the workload.
2. **Dividing Work Among MPI Processes:**
   - Distributes permutations among MPI processes for parallel adjustment and cost calculation.
3. **Collecting Results:**
   - Aggregates results from all processes to determine the global minimum cost route.

## Validation of Implementations

Validation is conducted using the `vrpy` library to ensure the correctness of the implemented algorithms.

**Steps:**
1. **Reading Graphs:**
   - Reads demands and edges from `grafo.txt` for different node sets.
2. **Extracting Costs and Routes:**
   - Extracts computed costs and routes from output files using regex.
3. **Configuration and Resolution with vrpy:**
   - Configures and solves the VRP using `vrpy` to compare with implementation results.

**Results:**
- All algorithms, except the heuristic in seven cases, produced costs matching the optimal values from `vrpy`.
- The heuristic provided near-optimal solutions, demonstrating its efficiency.

## Evaluation of Results

Analysis is performed using `analysis.py`, comparing execution times for different approaches with varying graph sizes.

**Observations:**
1. **Global Search:**
   - Exponential growth in execution time with the number of nodes, confirming high computational cost.
2. **Heuristic (Nearest Insertion):**
   - Significantly better performance than global search, though not always the cheapest route.
3. **OpenMP:**
   - Improved execution time over global search but not as significant as the heuristic.
4. **MPI + OpenMP:**
   - No significant improvement over OpenMP alone, suggesting communication overhead.

**Conclusion:**
- Heuristic approaches are crucial for efficiency in combinatorial optimization problems like VRP.
- Parallel approaches (OpenMP and MPI) offer additional benefits but should be combined with heuristics for best results.

---

For detailed implementation, validation, and evaluation processes, refer to the respective source code files and the validation report (`validation_results.txt`). The project directories (`NNnos`) contain data for different node sets used in testing.
