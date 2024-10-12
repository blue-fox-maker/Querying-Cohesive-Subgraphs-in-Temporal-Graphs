# Querying-Cohesive-Subgraphs-in-Temporal-Graphs

## Compilation Instructions

Clone the repository and compile the project with `g++`:

```sh
g++ -std=c++20 -O2 src/main.cpp -o src/main
```

## Run the Program

You can run the program using the following command:

```sh
./src/main --help
```

This will display the help menu:

```txt
Usage: main [-h]

Pisitional arguments:
  file          the dataset file

Optional arguments:
  -n            the number of queries to test
```

## Dataset

Datasets should be placed in the `data/` directory with `.data` extension.
The dataset file should have the following structure:

1. The first line contains three integers:

```txt
num_time num_vert num_edge
```
2. Each subsequent line represents a temporal edge in the format:
```txt
u v t
```

The index of vertex and timestamp should start from zero.
