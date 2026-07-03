# VFFS — Very Fucking File Snake 🐍💥

A hardcore, high-stakes CLI snake game written in C++ using `ncurses`. 
**Your files are the apples. ** If you win, they are saved. If you lose, your target directory gets completely formatted.

⚠️ **WARNING: RUNNING THIS GFUCKING SHITCAN CAUSE FUCKING TOTAL DATA LOSS WITHIN THE
FUCKING TARGET DIRECTORY. USE AT YOUR OWN FUCKING RISK.**

## Installation

### Dependencies
You need `ncurses` and a C++17 compliant compiler. On **Fedora** for example, install the development tools:

```bash
sudo dnf install ncurses-devel g++

g++ -std=c++17 main.cpp -o vffs -lncurses
chmod +x vffs
```

Usage
```bash

# Show help menu
./vffs -h

# Check version
./vffs -v

# Play in a specific sandbox directory
./vffs -d /path/to/target/dir
```