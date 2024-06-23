import cppyy
cppyy.include('solver/eef.h')
cppyy.include('solver/config.h')
cppyy.load_library('eefpy/eef.so')

__version__ = "0.1.2"

from .solver import solve
from .solver import Objective, EnvyNotion, EfficiencyNotion, MintcMode
