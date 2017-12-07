# Conways Game of Life

This is a C++ implementation of [Conway's Game of life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life).

The algorithm is using ideas from Michael Abrash's Graphics Programming Black Book, [Chapter 17](http://www.jagregory.com/abrash-black-book/#chapter-17-the-game-of-life), of using neighbour counts. It seems that this method is several times faster than the naive way of implementing it. (The naive way of implementing it is using a bool for each cell, where `true` is used for alive cells and `false` is used for dead cells.)

I implemented this algorithm as part of my master studies (1st semester course in efficient programming) of Game Engineering at the University of Applied Sciences [FH Technikum Wien](https://www.technikum-wien.at/) (Austria).

## License

GNU General Public License 3.
See [LICENSE.md](LICENSE.md) for details.

## Usage

This is a command line tool (running on Windows and macOS.)

### Command line parameters:

| Parameter               | Description                      |
| ----------------------- | -------------------------------- |
| `--load FILENAME`       | file to load (see below for file format) |
| `--generations N`       | specify number of generations to calculate |
| `--save FILENAME`       | specify where the result should be saved to |
| `--verify FILENAME`     | for testing: after we finished calculating the generations, check that the resulting board is the same as the verification board given here. The result is printed to stdout. |
| `--mode seq/omp/ocl`    | Mode of operation. `seq` = "sequential", single-threaded operation. `omp` = `openmp` = Use OpenMP for parallelization. `ocl` = `opencl` = Use OpenCL. The Windows version currently supports `seq` and `omp`. The macOS version currently only supports `seq` and `ocl`. (Default: `seq`) |
| `--threads T`           | Specify number of threads to use. Only valid for OpenMP mode. Defaults to 4. |
| `--device cpu/gpu`      | Use CPU or GPU for OpenCL. This option is only valid in `ocl` mode. (Default: Use GPU but fallback to CPU.) |
| `--verbose`             | Enable verbose mode              |
| `--measure`             | Print time measurements at the end. Three times are printed: (1) the time it took to load the input file, (2) the time it took to calculate the generations, (3) the time it took to save the output file or to do the verification. |

### Example usage:

```
./GameOfLife --load GameOfLife_Testdata/step1000_in_250generations/random10000_in.gol --verify GameOfLife_Testdata/step1000_out_250generations/random10000_out.gol --verbose --measure --generations 250 --mode seq
```

```
./GameOfLife --load GameOfLife_Testdata/step1000_in_250generations/random10000_in.gol --save result.gol --verbose --measure --generations 250 --mode seq
```

### File format

```
width,height
....x
....x
```
where `x` denotes alive cells and `.` denotes dead cells.

Example:

```
6,4
......
..xx..
..xx..
......
```

### Compiling

You need either Xcode or Visual Studio.

It has been tested with Xcode 9 and with Visual Studio 2017

## Some example test data

A 1000*1000 game of life board:
[random1000_in.gol](http://www.michaelpeternell.at/2017/GameOfLife/random1000_in.gol)<br>
After 250 generations, this becomes: [random1000_out.gol](http://www.michaelpeternell.at/2017/GameOfLife/random1000_out.gol)

A 1000*10000 game of life board:
[random10000_in.gol](http://www.michaelpeternell.at/2017/GameOfLife/random10000_in.gol)<br>
After 250 generations, this becomes: [random10000_out.gol](http://www.michaelpeternell.at/2017/GameOfLife/random10000_out.gol)

Or you can create your own random test data with the included perl script `gen-random-board.pl`.
