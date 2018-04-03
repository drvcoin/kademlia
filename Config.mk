# 
# MIT License
# 
# Copyright (c) 2018 drvcoin
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# 
# =============================================================================
# 

-include $(ROOT)/Local.mk

DEVROOT    ?= $(realpath $(ROOT))
BUILD_ROOT ?= $(DEVROOT)/build

include $(BUILD_ROOT)/Config.mk

SRC_ROOT = $(ROOT)/src
SRC_EXTERNAL = $(SRC_ROOT)/external

OUT_ROOT = $(ROOT)/out
OUT_INCLUDE = $(OUT_ROOT)/include
OUT_BLD = $(OUT_ROOT)/$(BLD)
OUT_BIN = $(OUT_BLD)/bin
OUT_LIB = $(OUT_BLD)/lib

OUT_CFG = $(OUT_ROOT)/cfg
OUT_SQL = $(OUT_ROOT)/sql
