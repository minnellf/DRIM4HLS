CXX = g++

HOME := ${HOME}
SYSTEMC_HOME := ${HOME}/DRIM4HLS/systemc-2.3.4
PROJECT_DIR := ${PWD}/HLS/v1.0
AC_SIMUTILS_HOME := ${PWD}/ac_simutils
CONNECTIONS_HOME := ${PWD}/matchlib_connections
AC_TYPES_HOME := ${PWD}/ac_types
SELECTED_CORE := core

INCDIR ?=
# INCDIRS = -isystem $(SYSTEMC_HOME)/include
# INCDIRS += -isystem $(SYSTEMC_HOME)/src/
# INCDIRS += -isystem $(AC_SIMUTILS_HOME)/include/
# INCDIRS += -isystem $(CONNECTIONS_HOME)/include/
# INCDIRS += -isystem $(AC_TYPES_HOME)/include/
# INCDIRS += -isystem $(PROJECT_DIR)/src/
INCDIRS = -I$(SYSTEMC_HOME)/include
INCDIRS += -I$(SYSTEMC_HOME)/src/
INCDIRS += -I$(AC_SIMUTILS_HOME)/include/
INCDIRS += -I$(CONNECTIONS_HOME)/include/
INCDIRS += -I$(AC_TYPES_HOME)/include/
INCDIRS += -I$(PROJECT_DIR)/src/
INCDIRS += -I$(SELECTED_CORE)/src/

LIBDIR = -L. -L$(SYSTEMC_HOME)/lib-linux64 -Wl,-rpath=$(SYSTEMC_HOME)/lib-linux64

CFLAGS =   -Wall -Wno-unknown-pragmas -Wno-unused-variable -Wno-unused-label $(INCDIRS) $(LIBDIR)
USER_FLAGS = -DCONNECTIONS_ACCURATE_SIM -DSC_INCLUDE_DYNAMIC_PROCESSES -DCONNECTIONS_NAMING_ORIGINAL

# RAND_STALL
# 0 = Random stall of ports and channels disabled (default)
# 1 = Random stall of ports and channels enabled
#
# This feature aids in latency insensitive design verication.
# Note: Only valid if SIM_MODE = 1 (accurate) or 2 (fast)
ifeq ($(RAND_STALL),1)
	USER_FLAGS += -DCONN_RAND_STALL
endif

LIBS = -lsystemc


.PHONY: Build
Build: all

CFLAGS += -O0 -g -std=c++11 

all: sim_sc

build: sim_sc

run:
	./sim_sc

sim_sc: $(wildcard ${SELECTED_CORE}/src/*.cpp) $(wildcard ${SELECTED_CORE}/src/*.h)
	$(CXX) -o sim_sc $(SYSTEMC_HOME)/lib-linux64/libsystemc.a $(CFLAGS) $(USER_FLAGS) $(wildcard ${SELECTED_CORE}/src/*.cpp) $(LIBS)

clean:
	rm -f sim_sc

