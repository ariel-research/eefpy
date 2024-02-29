import cppyy
#cppyy.include('eefpy/redirect.h')
cppyy.include('../../solver/eef.h')
cppyy.include('../../solver/config.h')
cppyy.load_library('eef.so')


from .solver import solve
from .solver import Objective, EnvyNotion, EfficiencyNotion, MintcMode
