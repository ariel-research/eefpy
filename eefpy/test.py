num_agents = 3 
num_types= 5

agents = [[3,2,1,-1,3],[1,2,4,3,2],[1,3,1,3,-1]]

items = [4,10,4,2,1]

from eefpy import solve, Objective, EnvyNotion
print(solve(num_agents=num_agents,num_types=num_types,agent_utils=agents,items=items, envy=EnvyNotion.NONE, obj=Objective.NONE))


