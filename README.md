# eefpy -- efficient envy-free practical solver in Python

This is a Python envelope for the C++ library [eef-practical-solver-code](https://git.tu-berlin.de/akt-public/eef-practical-solver-code), by Andrzej Kaczmarczyk.

The solver, given a collection of indivisible resources, agents and the agents' valuations of resources, finds an allocation meeting some configurable fairness and efficiency properties, or declares that such an allocation meeting the desired desiderata does not exist.

For a description of the supported fairness and efficiency concepts, techniques used by this solver, and the results otained see the paper: "High-Multiplicity Fair Allocation Made More Practical", referred at the 20th International Conference on Autonomous Agents and Multiagent Systems.

## Prerequisites
CPLEX solver by IBM:
1. For free academic license follow [this guide](https://community.ibm.com/community/user/ai-datascience/blogs/xavier-nodet1/2020/07/09/cplex-free-for-students).
2. Follow the [installation guide](https://www.ibm.com/docs/en/icos/20.1.0?topic=2010-installing-cplex-optimization-studio).
3. Edit the [Makefile](/solver/Makefile) with the relevant path and version in `CPLEX_DIR`.

## Requirements
1. C++ GNU
2. python & pip
3. venv

## Installation 
1. Clone the repository:
    ```
    git clone https://github.com/ariel-research/eefpy
    ```
2. Create a virtual environment:
    ```
    python -m venv venv
    ```
3. Activate the venv:
    ```
    source venv/bin/activate
    ```
4. Install requirements:
    ```
    pip install -r eefpy/requirements.txt
    ```
5. Build and install eefpy from source
    ```
    pip install -e .
    ```
    
## Usage examples

see [eefpy/examples/lib_examples.py](eefpy/examples/lib_examples.py).

