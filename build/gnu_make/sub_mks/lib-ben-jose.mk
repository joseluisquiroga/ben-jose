
TARGET := libben-jose.a

SRC_BASE_DIR := ../../../src

SOURCES := \
	$(SRC_BASE_DIR)/containers/bit_row.cpp \
	$(SRC_BASE_DIR)/sat_tools/brain/brain.cpp \
	$(SRC_BASE_DIR)/sat_tools/brain/sortor.cpp \
	$(SRC_BASE_DIR)/sat_tools/brain/reverse.cpp \
	$(SRC_BASE_DIR)/sat_tools/dimacs/dimacs.cpp \
	$(SRC_BASE_DIR)/sat_tools/shuffler/shuffler.cpp \
	$(SRC_BASE_DIR)/sat_tools/solver/support.cpp \
	$(SRC_BASE_DIR)/sat_tools/solver/central.cpp \
	$(SRC_BASE_DIR)/sat_tools/unsat_db/skeleton.cpp \
	$(SRC_BASE_DIR)/sat_tools/debug/test.cpp \
	$(SRC_BASE_DIR)/sat_tools/debug/dbg_ic.cpp \
	$(SRC_BASE_DIR)/sat_tools/debug/config.cpp \
	$(SRC_BASE_DIR)/utils/stack_trace.cpp \
	$(SRC_BASE_DIR)/utils/mem.cpp \
	$(SRC_BASE_DIR)/utils/tak_mak.cpp \
	$(SRC_BASE_DIR)/utils/sha2.c 


SRC_INCDIRS := \
	$(SRC_BASE_DIR)/sat_tools/brain \
	$(SRC_BASE_DIR)/sat_tools/debug \
	$(SRC_BASE_DIR)/sat_tools/dimacs \
	$(SRC_BASE_DIR)/sat_tools/solver \
	$(SRC_BASE_DIR)/sat_tools/unsat_db \
	$(SRC_BASE_DIR)/containers \
	$(SRC_BASE_DIR)/utils 

