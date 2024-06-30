from eefpy import solve, Objective, EnvyNotion
import logging

if __name__ == "__main__":
    logger = logging.getLogger('eefpy')
    logger.addHandler(logging.StreamHandler())

    print("*****Example from paper*****")
    num_agents = 3 
    num_types= 5
    agent_valuations = [[3,2,1,-1,3],[1,2,4,3,2],[1,3,1,3,-1]]
    item_capacities = [4,10,4,2,1]
    print(solve(num_agents=num_agents,num_types=num_types,agent_utils=agent_valuations,items=item_capacities, envy=EnvyNotion.NONE, obj=Objective.NONE))

    print("\n*****Example 2*****")
    item_capacities = [111,442,317,294,312]
    print(solve(num_agents=num_agents,num_types=num_types,agent_utils=agent_valuations,items=item_capacities, envy=EnvyNotion.EF, obj=Objective.NONE))

    print("\n*****Example 3*****")
    num_agents = 4
    num_types= 6
    agent_valuations = [[100,60,60,60,70,60],[60,100,60,60,70,60],[60,60,100,60,60,70],[60,60,60,100,60,70]]
    item_capacities = [2,2,2,2,2,2]
    print(solve(num_agents=num_agents,num_types=num_types,agent_utils=agent_valuations,items=item_capacities, envy=EnvyNotion.EF, obj=Objective.NONE))
