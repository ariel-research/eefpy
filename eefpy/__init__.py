import os
import cppyy
dir = os.path.dirname(os.path.abspath(__file__))
cppyy.include(dir+'/solver_cpp/eef.h')
cppyy.include(dir+'/solver_cpp/config.h')
cppyy.load_library(dir+'/solver_cpp/eef.so')

__version__ = "0.1.5"

from .solver import solve
from .solver import Objective, EnvyNotion, EfficiencyNotion, MintcMode
