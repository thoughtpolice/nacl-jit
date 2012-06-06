# Copyright (c) 2012 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#
# GNU Make based build file.  For details on GNU Make see:
# http://www.gnu.org/software/make/manual/make.html
#

#
# Project information
#
# These variables store project specific settings for the project name
# build flags, files to copy or install. In the examples it is typically
# only the list of sources and project name that will actually change and
# the rest of the makefile is boilerplate for defining build rules.
#
PROJECT:=jit
LDFLAGS:=-lppapi_cpp -lppapi -lnacl_dyncode
CXX_SOURCES:=module.cc
C_SOURCES:=support.c jit.c
DYNASM_SOURCES:=jit_x86.dasc jit_x86_64.dasc

#
# Get pepper directory for toolchain and includes.
#
# If NACL_SDK_ROOT is not set, then assume it can be found a two directories up,
# from the default example directory location.
#
THIS_MAKEFILE:=$(abspath $(lastword $(MAKEFILE_LIST)))
NACL_SDK_ROOT?=$(abspath $(dir $(THIS_MAKEFILE))../..)

# Project Build flags
WARNINGS:=-Wno-long-long -Wall -Wswitch-enum
CXXFLAGS:=-std=gnu++98 -Idynasm -pthread -std=gnu++98 $(WARNINGS)
CFLAGS:=-std=gnu99 -Idynasm -pthread $(WARNINGS)

#
# Compute tool paths
#
#
OSNAME:=$(shell python $(NACL_SDK_ROOT)/tools/getos.py)
TC_PATH:=$(abspath $(NACL_SDK_ROOT)/toolchain/$(OSNAME)_x86_newlib)
CXX:=$(TC_PATH)/bin/i686-nacl-g++
CC:=$(TC_PATH)/bin/i686-nacl-gcc

#
# Disable DOS PATH warning when using Cygwin based tools Windows
#
CYGWIN ?= nodosfilewarning
export CYGWIN


# Declare the ALL target first, to make the 'all' target the default build
DYNASM_OUT=$(patsubst %.dasc, %.h, $(DYNASM_SOURCES))
all: $(DYNASM_OUT) $(PROJECT)_x86_32.nexe $(PROJECT)_x86_64.nexe

# DynASM rules
$(DYNASM_OUT) : %.h : %.dasc $(THIS_MAKE)
	lua dynasm/dynasm.lua -c -M $< > $@

# Define 32 bit compile and link rules for main application
x86_32_OBJS_CPP:=$(patsubst %.cc,%_32.o,$(CXX_SOURCES))
x86_32_OBJS_C:=$(patsubst %.c,%_32.o,$(C_SOURCES))
$(x86_32_OBJS_CPP) : %_32.o : %.cc $(THIS_MAKE)
	$(CXX) -o $@ -c $< -m32 -DARCH_X86=1 -O0 -g $(CXXFLAGS)
$(x86_32_OBJS_C) : %_32.o : %.c $(THIS_MAKE)
	$(CC) -o $@ -c $< -m32 -DARCH_X86=1 -O0 -g $(CFLAGS)

$(PROJECT)_x86_32.nexe : $(x86_32_OBJS_CPP) $(x86_32_OBJS_C)
	$(CXX) -o $@ $^ -m32 -O0 -g $(CXXFLAGS) $(LDFLAGS)

# Define 64 bit compile and link rules for C++ sources
x86_64_OBJS_CPP:=$(patsubst %.cc,%_64.o,$(CXX_SOURCES))
x86_64_OBJS_C:=$(patsubst %.c,%_64.o,$(C_SOURCES))
$(x86_64_OBJS_CPP) : %_64.o : %.cc $(THIS_MAKE)
	$(CXX) -o $@ -c $< -m64 -DARCH_X86_64=1 -O0 -g $(CXXFLAGS)
$(x86_64_OBJS_C) : %_64.o : %.c $(THIS_MAKE)
	$(CC) -o $@ -c $< -m64 -DARCH_X86_64=1 -O0 -g $(CFLAGS)

$(PROJECT)_x86_64.nexe : $(x86_64_OBJS_CPP) $(x86_64_OBJS_C)
	$(CXX) -o $@ $^ -m64 -O0 -g $(CXXFLAGS) $(LDFLAGS)

# Define a phony rule so it always runs, to build nexe and start up server.
.PHONY: RUN 
RUN: all
	-python -m SimpleHTTPServer
clean:
	rm -f *.nexe *~ *.o jit_x86.h jit_x86_64.h \#*
