# Conways Game of Life

This is a C++ implementation of [Conway's Game of life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life).

The algorithm is using ideas from Michael Abrash's Graphics Programming Black Book, [Chapter 17](http://www.jagregory.com/abrash-black-book/#chapter-17-the-game-of-life), of using neighbour counts. It seems that this method is several times faster than the naive way of implementing it. (The naive way of implementing it is using a bool for each cell, where `true` is used for alive cells and `false` is used for dead cells.)

I implemented this algorithm as part of my master studies (1st semester course in efficient programming) of Game Engineering at the University of Applied Sciences [FH Technikum Wien](https://www.technikum-wien.at/) (Austria).

## License

GNU General Public License 3.
See [LICENSE.md](LICENSE.md) for details.

## Usage

This is a command line tool (currently only for Mac, but a Windows version will come soon.)

Example usage:

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

## Some example test data

A 1000*1000 game of life board:
[random1000_in.gol](http://www.michaelpeternell.at/2017/GameOfLife/random1000_in.gol)<br>
After 250 generations, this becomes: [random1000_out.gol](http://www.michaelpeternell.at/2017/GameOfLife/random1000_out.gol)
