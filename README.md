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

```
Usage: main [-h]

Pisitional arguments:
  file          the dataset file

Optional arguments:
  -n            the number of queries to test
```

## Dataset

datasets are put in data/ folder. with .data

The first line should be 

```
num_time num_vert num_edge
```

follows by temporal edges

```
u1 v2 t1
u2 v2 t2
...
```
The index of verticle and timestamp should start with zero.
