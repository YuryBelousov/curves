# Brute-Force Search for Simple Arcs in Plane Curves and Knot Diagrams

## Overview
This repository serves as the computational foundation for the article ["Simple Arcs in Plane Curves and Knot Diagrams"](http://journal.imm.uran.ru/2017-v.23-4-pp.63-76) (in Russian). It is used to prove the following theorems:

#### **Theorem 1.1** 
> In each minimal diagram of an arbitrary knot $ K $, there exists a simple arc passing through $ \min\{\text{cr}(K),6\} $ crossings, where $ \text{cr}(K) $ is the crossing number.
#### **Theorem 1.2.** 
> For any knot $ K $ except for the four simple knots $8_{16}$, $8_{18}$, $9_{40}$ and $10_{120}$ in the notation of the Rolfsen table, there exists a minimal diagram with a simple arc passing through $ \min\{\text{cr}(K),8\} $ crossings.

## Prerequisites
The toolkit requires a **C++20-compatible compiler**. No external dependencies are needed.

## Structure and Execution
The main execution logic is contained in **`Source.cpp`**.  
- **`Prediagram.h`** defines the `pre_diagram` class, which is central to the algorithm.  
- **`Domain.h`** provides technical classes used within `pre_diagram`.  

After execution, the program generates **`exceptional_curves.txt`**, containing Gauss codes of all exceptional curves.

## To run the program
```sh
g++ -std=c++20 *.h *.cpp -o brute_force
```

## Math details of the algorithm 
By a *curve* we mean a plane curve of general position. A *pre-diagram* $D$ is an open curve with a finite number of marked points that can be extended to a closed curve $X$ without marked points, such that all new double points of $X$ are marked points of $D$. The program successively goes through all possible pre-diagrams and discards those for which all of the possible extensions satisfies the condition of Theorem 1.1 or 1.2 (here, [flypes](https://en.wikipedia.org/wiki/Flype) are used). It turns out that the set of such pre-diagrams is finite, and in the case of the condition of Theorem 1.1 it is empty, and in case of Theorem 1.2, all of them can be extended to a curve of one of the 4 cases presented in the Theorem.

## Mathematical Details of the Algorithm
By a  *curve* we mean a plane curve in general position. A *pre-diagram* $D$ is an open curve with a finite number of marked points that can be extended to a closed curve $X$ without marked points, such that all new double points of $X$ originate from the marked points of $D$.

The program systematically iterates through all possible pre-diagrams, discarding those for which all possible extensions satisfy the conditions of **Theorem 1.1** or **Theorem 1.2** (in the case of **Theorem 1.2** [flypes](https://en.wikipedia.org/wiki/Flype) are used). It turns out that the set of such pre-diagrams is finite:
- For **Theorem 1.1**: The set of exceptional cases is empty.
- For **Theorem 1.2**: Every exceptional pre-diagram can be extended into a diagram belonging to one of four specific cases listed in the theorem.

For more details, see source files.