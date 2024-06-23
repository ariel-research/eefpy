# eefpy -- efficient envy-free practical solver in Python

This is a Python envelope for the C++ library [eef-practical-solver-code](https://git.tu-berlin.de/akt-public/eef-practical-solver-code), by Andrzej Kaczmarczyk.
The C++ code accompanies the paper "High-Multiplicity Fair Allocation Made More Practical", by Robert Bredereck, Aleksander Figiel (the main code contributor), Andrzej Kaczmarczyk, Dušan Knop and Rolf Niedermeier, which was presented at AAMAS 2021. The code is protected by GNU GPL v3.0.

The solver, given a collection of indivisible resources, agents and the agents' valuations of resources, finds an allocation meeting some configurable fairness and efficiency properties, or declares that such an allocation meeting the desired desiderata does not exist.

For a description of the supported fairness and efficiency concepts, techniques used by this solver, and the results obtained, see the above-mentioned paper.

## Prerequisites
CPLEX solver by IBM:
1. For free academic license, follow [this guide](https://community.ibm.com/community/user/ai-datascience/blogs/xavier-nodet1/2020/07/09/cplex-free-for-students): create an IBM id with your university-based email, login, and then click on "Data Science" at the left menu bar.
2. Follow the [installation guide](https://www.ibm.com/docs/en/icos/20.1.0?topic=2010-installing-cplex-optimization-studio) -- install on Linux.


## Requirements
1. C++ GNU
2. python & pip, virtualenv

## Installation (pip)
1. Run:
    ```
    pip install eefpy
    ```
2. Navigate to the `solver` folder:
    ```
    cd solver
    ```
3. Edit the file [/solver/Makefile](/solver/Makefile): update the `CPLEX_DIR` variable to match the folder in which you installed CPLEX.
4.  Save the Makefile and run:
    ```
    make eef
    ```

## Installation from source
1. Clone the repository:
    ```
    git clone https://github.com/ariel-research/eefpy
    cd eefpy
    ```
2. Create a Python virtual environment and activate it:
    ```
    virtualenv venv
    source venv/bin/activate
    ```
4. Install requirements:
    ```
    pip install -r requirements.txt
    ```
5. Build and install eefpy from source
    ```
    pip install -e .
    ```
6. Edit the file [/solver/Makefile](/solver/Makefile): update the `CPLEX_DIR` variable to match the folder in which you installed CPLEX.
7.  Save the Makefile and run:
    ```
    make eef
    ```
    
## Usage examples

Run the file [examples/lib_examples.py](examples/lib_examples.py).

## Verification

To verify the results, make the solver in the [solver/](solver/) folder, then run:

    solver/main

Copy and paste the text from the files [examples/paper_example](examples/paper_example) and  [examples/example2](examples/example2).
