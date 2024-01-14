sources := main.cpp config.cpp eef.cpp trading_cycle.cpp stats.cpp ilp_model/model.cpp ilp_model/primary.cpp ilp_model/efficiency/efficiency.cpp ilp_model/efficiency/pareto.cpp ilp_model/efficiency/mintc.cpp ilp_model/efficiency/none.cpp
headers := config.h eef.h trading_cycle.h ilp_model/model.h ilp_model/primary.h ilp_model/efficiency/efficiency.h ilp_model/efficiency/pareto.h ilp_model/efficiency/mintc.h ilp_model/efficiency/none.h

CFLAGS := -std=c++11 -O2
#CFLAGS := -std=c++11 -g -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers
#CFLAGS := -std=c++11 -g -Og -Wall -Wextra -pg -no-pie

CPLEX_DIR := /opt/ibm/ILOG/CPLEX_Studio2211
CPLEX_FLAGS := -isystem$(CPLEX_DIR)/cplex/include \
               -isystem$(CPLEX_DIR)/concert/include \
               -Xlinker -L$(CPLEX_DIR)/cplex/lib/x86-64_linux/static_pic \
               -Xlinker -L$(CPLEX_DIR)/concert/lib/x86-64_linux/static_pic \
	       -DIL_STD

eef.so: $(sources) $(headers) Makefile
	g++ $(CFLAGS) $(CPLEX_FLAGS) -shared -fPIC -o $@ $(sources) -lilocplex -lconcert -lcplex -lm -pthread -ldl

main: $(sources) $(headers) Makefile
	g++ $(CFLAGS) $(CPLEX_FLAGS) -o $@ $(sources) -lilocplex -lconcert -lcplex -lm -pthread -ldl
