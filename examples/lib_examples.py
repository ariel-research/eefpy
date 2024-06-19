from eefpy import solve, Objective, EnvyNotion
import logging

if __name__ == "__main__":
    logger = logging.getLogger('eefpy')
    logger.addHandler(logging.StreamHandler())
    num_agents = 3 
    num_types= 5

    print("*****paper example*****")
    agents = [[3,2,1,-1,3],[1,2,4,3,2],[1,3,1,3,-1]]
    items = [4,10,4,2,1]
    print(solve(num_agents=num_agents,num_types=num_types,agent_utils=agents,items=items, envy=EnvyNotion.NONE, obj=Objective.NONE))

    print("\n*****Example 2*****")
    items = [111,442,317,294,312]
    print(solve(num_agents=num_agents,num_types=num_types,agent_utils=agents,items=items, envy=EnvyNotion.NONE, obj=Objective.NONE))
