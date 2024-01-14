import cppyy
cppyy.include('../../solver/eef.h')
cppyy.include('../../solver/config.h')
cppyy.load_library('eefpy/eef.so')

from .solver import solve
from .solver import Objective, EnvyNotion, EfficiencyNotion, MintcMode
