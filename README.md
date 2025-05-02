# Simple Scales
By: Simon Libricky - [simonlibricky.com](www.simonlibricky.com)

As part of Charles University C++ Course

# TL;DR

This is a simple command line tool for practising recognising western musical scales based on the notes they contain. Accompanying it is a (very) simple C++ library for symbolic music.

# Installation

I have included a CMakeLists.txt file so you can proceed using CMake.

There is also a Makefile I personally used to compile in my environment (VSCode, g++ on Mac OS).

C++20 is required.

Launch the compiled output as normal with command line arguments.

# Usage

The script can be called with command line arguments to specify how you want the script to behave.

```-n {int}``` - sets how many questions you want to be asked in the session
```-i {path.csv}``` - sets the path to the .csv file where the scales are stored
```-o {path.csv}``` - sets the path to the .csv file where the session results are stored
```-d {Easy|Medium|Hard}``` - sets the difficulty of the questions you will be asked

Then, you will be asked to **NAME THAT SCALE!** You can do this by typing 1 through 4 (corresponding to the presented choices) and pressing Enter.

That's it; it's not all that difficult to use.

# Developer documentation

Doxygen documentation is available in ```/docs/html```.

A short document explaining the programming choices is also present at ```/docs/prog_doc.md```

# Attribution

I use [this argparse library](https://github.com/morrisfranken/argparse) by [morrisfranken](https://github.com/morrisfranken) to handle the command line arguments.