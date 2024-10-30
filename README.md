# Querying-Cohesive-Subgraphs-in-Temporal-Graphs

## Compilation Instructions

Clone the repository and compile the project with `g++`:

```sh
g++ -std=c++20 -O3 src/main.cpp -o main
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

For example:
```sh
# test cc with 10,000 queries and query interval sizes of 30% and 40% t_max:
./main cc data/contact.data -n 10000 -r 0.3 0.4

# test core with 5,000 queries, query interval size of 10%, 20% and 30% of t_max and k values of 30% and 80% of k_max:
./main core data/contact.data -n 5000 -r 0.1 0.2 0.3 -k 0.3 0.8
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
