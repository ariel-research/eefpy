from .eef import *
import cppyy
import logging

logging.basicConfig(format='%(name)s: %(message)s')

logger = logging.getLogger('eefpy')
logger.setLevel(logging.DEBUG)

def solve(num_agents, num_types,agent_utils,items, alpha=None, iteralphalock=0,divisibles=True, 
           analyse=None, debug=False, efficiency:EfficiencyNotion=None, envy:EnvyNotion=None,
           incomplete=False,iter_alpha_lock=False, mintc_crude=False, mintc_eq0=False,
           MintcMode:MintcMode=None, obj:Objective=None,):
    
    kwargs = locals().copy() # arguments as dict
    I = EEF()
    args = Arguments()
    get_input_args(kwargs,args)

    read_instance(I, num_agents, num_types, agent_utils, items, args)

    I.has_negatives = False
    for row in agent_utils:
        for col in row:
            if col<0:
                I.has_negatives = True
                break
    if I.has_negatives:
        logger.info("#     Warning: Utility matrix has negative utilities!")
        logger.info("#              The notions of EFX, EF1 and relative envy may not function as desired")
    
    if not args.analyze:
        allocation = I.solve(args.eef_cfg)
        if args.eef_cfg.trash_agent: 
            I.n -= 1
        I.print_allocation(allocation)
        py_allocation = [list(row) for row in allocation]

        return py_allocation
        
        
    else:
        pass # READ FILE 


def read_instance(I:EEF, num_agents, num_types,agent_utils, items, args: Arguments):

    I.n = num_agents
    I.m = num_types

    if I.n == 0 or I.m == 0:
        return

    I.u.resize(I.n)
    for i in range(I.n):
        print(agent_utils[i])
        I.u[i]=agent_utils[i]
        if len(I.u[i]) != I.m:
            logger.error(f"Insufficient number of utilities provided for agent {i + 1}")
            exit(1)

    I.mu = items
    if len(I.mu) != I.m:
        logger.error("Insufficient number of item multiplicities provided")
        exit(1)

    if args.eef_cfg.trash_agent:
        logger.info("# Adding trash agent")
        I.n += 1
        I.u.resize(I.n)
        I.u[I.n - 1].resize(I.m)
        for j in range(I.m):
            max_util = max([I.u[i][j] for i in range(I.n)])
            I.u[I.n - 1][j] = -max_util


    #divisible = []
    if args.divisibles:
        for j in range(I.m):
            #divisible[j] = int(input())

            if args.divisibles[j] == 1:
                I.mu[j] *= DIV_MULT
            else:
                for i in range(I.n):
                    I.u[i][j] *= DIV_MULT


def get_input_args(kwargs: dict ,args: Arguments):
    if kwargs['envy']:
        args.eef_cfg.envy = kwargs['envy'].value
    if kwargs['alpha']:
        args.eef_cfg.alpha = kwargs['alpha'].value
    if kwargs['iter_alpha_lock']:
        args.eef_cfg.iterated_alpha_locking = True
    if kwargs['obj']:
        args.eef_cfg.objectives.push_back(kwargs['obj'].value)
    if kwargs['efficiency']:
        args.eef_cfg.efficiency = kwargs['efficiency'].value
        if kwargs['efficiency'] == EfficiencyNotion.DYNAMIC_MINTC_CRUDE_EQ0:
            args.eef_cfg.efficiency = EfficiencyNotion.DYNAMIC_MINTC
            args.eef_cfg.mintc.eq0 = True
            args.eef_cfg.mintc.crude = True
    if kwargs['mintc_crude']:
        args.eef_cfg.mintc.crude = True
    if kwargs['mintc_eq0']:
        args.eef_cfg.mintc.eq0  = True
    if kwargs['MintcMode']:
        args.eef_cfg.mintc.mode = kwargs['MintcMode'].value
    if kwargs['incomplete']:
        args.eef_cfg.trash_agent = True
    if kwargs['analyse']:
        args.analyze = kwargs['analyse'].value
    if kwargs['debug']:
        args.eef_cfg.debug = True