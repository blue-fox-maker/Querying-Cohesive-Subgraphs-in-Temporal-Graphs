# Querying-Cohesive-Subgraphs-in-Temporal-Graphs

## Compilation Instructions

Clone the repository and compile the project with `g++`:

```sh
g++ -std=c++20 -O2 src/main.cpp -o main
```

## Run the Program

Once compiled, you can run the program using the following command:

```sh
./main --help
```

This will display the help menu:

```txt
Usage: ./main [--help] [--version] [--num_query VAR] [--ratio VAR...] [--k_ratio VAR...] model file

Positional arguments:
  model            cc or core
  file             dataset path

Optional arguments:
  -h, --help       shows help message and exits
  -v, --version    prints version information and exits
  -n, --num_query  number of test queries [nargs=0..1] [default: 10000]
  -r, --ratio      ratio for query interval size [nargs: 1 or more]
  -k, --k_ratio    ratio for k (used in k-core) [nargs: 1 or more]
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
An example dataset file named `contact.data` is provided in the `data/` folder.
