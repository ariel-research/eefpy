from .eef import EEFConfig

class Arguments:
    def __init__(self, divisibles=False, analyze=False, alloc_file=""):
        self.eef_cfg = EEFConfig()
        self.divisibles = divisibles
        self.analyze = analyze
        self.alloc_file = alloc_file