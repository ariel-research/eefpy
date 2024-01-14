import cppyy
cppyy.include('/home/fd-algos/eef-practical-solver-code/solver/eef.h')        # bring in C++ definitions
cppyy.include('/home/fd-algos/eef-practical-solver-code/solver/config.h')        # bring in C++ definitions

cppyy.load_library('/home/fd-algos/eef-practical-solver-code/solver/eef.so')     # load linker symbols


num_agents = 3 
num_types= 5

agents = [[3,2,1,-1,3],[1,2,4,3,2],[1,3,1,3,-1]]

items = [4,10,4,2,1]

from solver.solver import create
create(num_agents=num_agents,num_types=num_types,agent_utils=agents,items=items)