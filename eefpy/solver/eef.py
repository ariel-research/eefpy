from cppyy.gbl import EEF,EEF_Config
DIV_MULT = 100
PRINT_PRECISION = 2
from enum import Enum

class Objective(Enum):
    NONE = 0
    MAX_SWF = 1
    MIN_SWF = 2
    MIN_MAX_ABS_ENVY = 3
    MIN_MAX_ABS_ENVY_OLD = 4
    MIN_EMPTY_AGENTS = 5
    MIN_TRASHED_ITEMS = 6
    MIN_TRASHED_UTIL = 7
    MIN_DOMTC_WEIGHT = 8
    MIN_MAX_ALPHA = 9
    MIN_MAX_ALPHA_QP = 10

class EnvyNotion(Enum):
    NONE = 0
    EF = 1
    EF1 = 2
    EFX = 3
    EF_ALPHA = 4
    EF_ALPHA_QP = 5

class EfficiencyNotion(Enum):
    NONE = 0
    PARETO = 1
    DYNAMIC_MINTC_CRUDE_EQ0 = 2
    DYNAMIC_MINTC = 3
    STATIC_MINTC = 4

class MintcMode(Enum):
    CONST = 0
    NUM_AGENTS = 1
    NUM_POSITIVE_AGENTS = 2
    MIN_SET_OF_AGENTS = 3
    SUM_ENTRIES = 4

class Arguments:
    def __init__(self, divisibles=False, analyze=False, alloc_file=""):
        self.eef_cfg = EEF_Config()
        self.divisibles = divisibles
        self.analyze = analyze
        self.alloc_file = alloc_file
