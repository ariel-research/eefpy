import cppyy
from enum import Enum
DIV_MULT = 100
PRINT_PRECISION = 2
_cpp = cppyy.gbl

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

class EEFConfig:
    def __init__(self):
        self.envy = EnvyNotion.EF
        self.alpha = 0.0
        self.iterated_alpha_locking = False
        self.efficiency = EfficiencyNotion.PARETO
        self.objectives = []

        self.mintc = {
            'crude': False,
            'eq0': False,
            'mode': MintcMode.CONST
        }

        self.trash_agent = False
        self.tc_only_negatives = True
        self.debug = False

class EEF:
    def __init__(self):
        self.n = 0  # number of agents
        self.m = 0  # number of item groups
        self.mu = []  # item multiplicities
        self.u = []  # utilities matrix u[i][j] = utility of item j to agent i
        self.has_negatives = False

        # additional stuff for divisible items
        # WARNING: we modify u and mu to support divisible items
        self.enable_div = False
        self.divisible = []

        self.cached_tcs = []
    
    def cfg_py2cpp(self,cfg):
        # Convert Python EEF_Config to C++ EEF_Config
        print(cfg)
        cpp_cfg = _cpp.EEF_Config()

        cpp_cfg.envy = cfg.envy.value
        cpp_cfg.alpha = cfg.alpha
        cpp_cfg.iterated_alpha_locking = cfg.iterated_alpha_locking
        cpp_cfg.efficiency = cfg.efficiency.value
        cpp_cfg.objectives = [obj.value for obj in cfg.objectives]

        cpp_cfg.mintc.crude = cfg.mintc['crude']
        cpp_cfg.mintc.eq0 = cfg.mintc['eq0']
        cpp_cfg.mintc.mode = cfg.mintc['mode'].value

        cpp_cfg.trash_agent = cfg.trash_agent
        cpp_cfg.tc_only_negatives = cfg.tc_only_negatives
        cpp_cfg.debug = cfg.debug
        return cpp_cfg
    
    def eef_py2cpp(self):
        cpp_eef = _cpp.EEF()
        cpp_eef.n = self.n
        cpp_eef.m = self.m
        cpp_eef.mu = self.mu
        cpp_eef.u = self.u
        cpp_eef.has_negatives = self.has_negatives
        cpp_eef.enable_div = self.enable_div
        cpp_eef.divisible = self.divisible
        cpp_eef.cached_tcs = self.cached_tcs

        return cpp_eef

    def solve(self, cfg):
        cpp_cfg = self.cfg_py2cpp(cfg)
        cpp_eef = self.eef_py2cpp()
        # Call the C++ solve function
        cpp_solution = cpp_eef.solve(cpp_cfg)

        # Convert C++ solution to Python format
        py_solution = [list(row) for row in cpp_solution]
        return py_solution

    def find_alpha_range(self, locked):
       # Convert Python list to C++ vector<bool>
        cpp_locked = _cpp.std.vector[bool](locked)

        # Call the C++ find_alpha_range function
        cpp_alpha_range = _cpp.find_alpha_range(cpp_locked)

        # Convert C++ alpha range to Python list
        py_alpha_range = list(cpp_alpha_range)
        return py_alpha_range
    
    def minimal_trading_cycles(self, cfg):
        cpp_cfg = self.cfg_py2cpp(cfg)

        # Call the C++ minimal_trading_cycles function
        cpp_min_tcs = _cpp.minimal_trading_cycles(cpp_cfg)

        # Convert C++ minimal_trading_cycles to Python format
        py_min_tcs = [[list(row) for row in tc] for tc in cpp_min_tcs]
        return py_min_tcs

    def all_trading_cycles(self):
        # Call the C++ all_trading_cycles function
        cpp_all_tcs = _cpp.all_trading_cycles()

        # Convert C++ all_trading_cycles to Python format
        py_all_tcs = [[list(row) for row in tc] for tc in cpp_all_tcs]
        return py_all_tcs
    
    def print_allocation(self,allocation):
        n = self.n
        m = self.m
        u = self.u
        max_col_width = [0] * n

        for i in range(n):
            for j in range(n):
                util = sum(u[i][k] * allocation[j][k] for k in range(m))
                to_print = str(util)
                max_col_width[j] = max(max_col_width[j], len(to_print))
        print("# Allocation stats")
        print("#     Agents evaluation of allocation: ")
        for i in range(n):
            diff_str="#     "
            for j in range(n):
                util = sum(u[i][k] * allocation[j][k] for k in range(m))
                to_print = str(util)
                diff = max_col_width[j] - len(to_print)
                assert diff < 100

                diff_str+=" " * diff + to_print + " "
            print(diff_str)
