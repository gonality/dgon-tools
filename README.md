# dgon-tools

This repository contains a collection of programs for studying divisorial gonality of finite graphs.
At the core lies a fast implementation of the (very slow) brute force algorithm for computing gonality, implemented in the file [`divisors.h`](divisors.h).

## Main programs

The suite consists of the following programs:

   * `find_gonality`: reads a number of graphs from the input, and computes the gonality of the k-regular subdivision of each of these graphs;
   * `subdivision_conjecture`: reads a number of graphs from the input, and tests the subdivision and Brill–Noether conjectures for these graphs;
   * `Brill_Noether_geng`: test the Brill–Noether conjecture for all simple, connected graphs on N vertices.
      This program is compiled and linked against the auxiliary program `geng` from the `gtools` suite packaged with [`nauty`](https://pallini.di.uniroma1.it) [MP20], which must be downloaded separately.

Note: although the tasks of these programs overlap, the more specific programs are (much) faster.
In particular, `subdivision_conjecture` with the `-f` flag set only searches for a positive rank divisor of degree dgon(G) - 1 on the k-subdivision of G, which is faster than computing the gonality of the subdivision.
Likewise, `Brill_Noether_geng` uses several heuristics to immediately discard graphs with too many edges (enough so that the Brill–Noether bound is trivially met) and graphs with a large enough independent set, making it several orders of magnitude faster than `find_gonality` and `subdivision_conjecture` for the task at hand.


## Command-line options
For an overview of the command line options of each of the programs, run
```
find_gonality -h
subdivision_conjecture -h
Brill_Noether_geng -h
```


## Input formats

The programs `find_gonality` and `subdivision_conjecture` can read two types of input: a graph6 file or a human-readible “plain” format.
The program `Brill_Noether_geng` does not take input.

### The graph6 input format
In the graph6 format, each line in the input should be a graph6-encoded graph, as documented in the user guide of [`nauty`](https://pallini.di.uniroma1.it) [MP20].
The optional header `>>graph6<<` at the beginning of the file is currently not supported and should be omitted.
Furthermore, note that the graph6 format does not support parallel edges.
The similar `sparse6` format does, but is currently not supported.
For parallel edges, the plain format must be used.

### The plain input format
The plain input format consists of any number of blocks of the following form:
   * One line indicating the name of the graph;
   * One line with two integers N and M, indicating the number of vertices and edges;
   * M lines containing two integers v<sub>i</sub> and w<sub>i</sub> (0 ≤ v<sub>i</sub>, w<sub>i</sub> < N; v<sub>i</sub> ≠ w<sub>i</sub>), indicating that there is an (undirected) edge between v<sub>i</sub> and w<sub>i</sub>.

Empty lines in the input will be ignored.

### Converting between the two input formats
Converting between the two input formats can be done using the auxiliary programs `convert_to_graph6` and `convert_from_graph6`.
Please note that the graph6 input format does not support parallel edges.
If some of the graphs in the plain input file `examples.in` contain parallel edges, it is only possible to save the k-subdivision of these graphs in graph6 format, like so:
```
convert_to_graph6 k < examples.in > examples-subdivided.g6
```
where `k` is an integer between 2 and the compile-time constant `MAX_PARTS_PER_EDGE` (default: 10).

## Troubleshooting

### Unable to compile
Please contact us and describe the problem in as much detail as possible.
We will try to look into it, but please bear in mind that we are mathematicians and not software engineers.
The code has been successfully compiled on modern versions of Debian and Ubuntu.

### The program crashes with a cryptic error message
The code will raise an assertion and crash if something unexpected happens.
For example, this may happen if the input cannot be parsed, if a graph is malformed or exceeds the maximum graph size (determined by the compile-time constant `MAX_N`), or if an intermediate result in the computation is not what we expect it to be.
This approach is very effective at eliminating bugs and unexpected behaviour, but the downside is that a lengthy computation may suddenly be aborted with only a cryptic message about what went wrong.

If you are unable to isolate the issue by looking at the code, please contact us for help.


## License and copyright

   Copyright 2021 Josse van Dobben de Bruyn, Harry Smit, and Marieke
   van der Wegen.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.


## References

  [MP20]: Brendan D. McKay and Adolfo Piperno, *nauty and Traces*, version 2.7r1, 2020. https://pallini.di.uniroma1.it.
