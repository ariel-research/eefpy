This is the code accompanying the paper High-Multiplicity Fair
Allocation Made More Practical by Robert [Bredereck](https://robert.bredereck.info/), Aleksander Figiel (the main code contributor), [Andrzej Kaczmarczyk](https://www.user.tu-berlin.de/droores/), [Dušan Knop](https://fit.cvut.cz/cs/fakulta/lide/5176-rndr-dusan-knop-ph-d/publikace) and [Rolf Niedermeier](https://www.akt.tu-berlin.de/index.php?id=110570), which was [accepted for publication](https://aamas2021.soton.ac.uk/programme/accepted-papers/) at [the 20th
International Conference on Autonomous Agents and Multiagent Systems](https://aamas2021.soton.ac.uk/).

The code is protected by GNU GPL v3.0.

# Short Introduction

The solver, given a collection of indivisible resources, agents and the agents' valuations of resources, finds an allocation meeting couple of configurable fairness and efficiency properties or declares that such an allocation meeting the desired desiderata does not exist.

For a description of the supported fairness and efficiency concepts, techniques used by this solver, and the results otained see the paper High-Multiplicity Fair Allocation Made More Practical referred at the 20th International Conference on Autonomous Agents and Multiagent Systems.

*From now on, the readme assumes knowledge of the paper; in partiucular the nomenclature used therein.*


# Repository Organization

- `solver` --- solver in C++ to solve the problem
- `COPYING` --- the GNU General Public License v3.0


# Solver

## Prerequisites
CPLEX solver by IBM

## Installation
The solver can be compiled simply by runing `make` in directory `solver`. As an effect, the solver executable called `main` should appear in the directory.

## Usage

The solver reads in the input data (a single instance of the problem) from the standard input unless the user specifies otherwise (through a parameter).

The solver accepts the following arguments whose slightly more detailed description is following the list (the arguments can also be printed passing -? as an argument to `main`, however it would list some of the deprecated arguements as well).
```
      --alpha=VALUE          Specify alpha value for EF-alpha
      --analyse[=FILE]       Read allocation from file or stdin if no file is given
      --debug                Enable debug output
      --efficiency=NOTION, --eff=NOTION
                             Specify efficiency notion to use (pareto, mintc, none)
      --envy=NOTION, --env=NOTION
                             Specify envyness notion to use (ef, efx, ef1, ef-alpha)
      --incomplete           Enable incomplete allocations through the introduction of a trash agent
      --iter-alpha-lock      Minimize the alpha in EF-alpha, find bottleneck agent and lock their alpha
      --mintc_crude          Use a crude, but faster version of MINTCs
      --mintc_mode=MODE      Specify in what way to use MINTCs
      --objective=NOTION, --obj=NOTION
                             Specify objective (none, max_swf, min_swf, min_max_abs_envy)
```

We provide an extended description of several parameters. We start with the most important ones, those which describe what allocations should be sought:
- `--envy=NOTION` --- Specify which fairness concept a sought allocation should meet: none, envy-freeness (ef), envy-freeness up to one good (ef1), envy-freeness up to any good (efx), or alpha-factor envy freenees (ef-alpha).
- `--alpha=VALUE` --- Specify the maximum possible alpha value for EF-alpha.
- `--efficiency=NOTION` --- Specify which fairness criteria a sought allocation should meet: none, Pareto-efficiency (pareto), or efficiency based on the trading cycles (mintc) (see parameter `--mintc_mode`).
- `--objective=NOTION` --- Specify which additional objective should be applied: none, maximizing social welfare (max_swf), minimizing social welfare (min_swf), minimizing maximimal absolute envy (min_max_abs_envy). The additional objective is optimized **only** within the space of the allocations meeting the specified efficiency and envy criteria!

Parameters affecting considering usage of minimum trading cycles (MINTC):
- `--mintc_crude` --- The solver adds trading cycles **that shrink the search space** basing **only** on the resources that are taken away in each cycle (it is enough since the fact that a cycle makes one agent ``having'' a negative number of resources is enough).
- `--mintc_mode=MODE` --- Specify how to exactly optimize using minimal trading cycle when using `mintc` efficiency value: minimizing the number of minimal trading cycle (value: `const`), minimizing the number of univolved agents (value: `num_agents`), or maximizing the number of involved agents (value: `num_positive_agents`).

Additional parameters affecting the outcoming allocations:
- `--incomplete` --- This switch turns on the trash agent which is **not considered** in envy and fairness concepts. Effectively, it means that an outcome allocation can be incomplete (not all resources allocated to the age2nts).
- `--iter-alpha-lock` --- This switch only works when the envy concept chosen is EF-alpha. If this argument is given, then whenever the solver finds an agent whose envy-alpha-coeffiecient cannot be improved, the solver fixes this agent's coefficient and tries to minimize the alpha-coefficients of the remaining agents. Note, that in case of two agents whose coefficients cannot be descreased, the coefficient to fix is chosen arbitrarily (which might lead to suboptimal choice of the fixed coefficient).
