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

###############################################################################
# Prerequisites
###############################################################################

BUILD_ROOT := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include $(BUILD_ROOT)/Local.mk

ifeq ($(ROOT),)
$(error "Please define ROOT variable to point to the repository's root folder")
endif

###############################################################################
# Depot Roots
###############################################################################

DEVROOT         ?= $(realpath $(BUILD_ROOT)/..)

###############################################################################
# Versioning
###############################################################################

BUILD_NUMBER := $(shell cat $(BUILD_ROOT)/BuildNumber)

GEAR_VERSION_MAJOR        ?= $(shell cat $(BUILD_ROOT)/BuildMajorVersion)
GEAR_VERSION_MINOR        ?= $(shell cat $(BUILD_ROOT)/BuildMinorVersion)
GEAR_VERSION_BUILD        ?= $(BUILD_NUMBER)
ifeq ($(shell uname -r | grep -o el7), el7)
	GEAR_VERSION_PLATFORM     ?= el7
endif
ifeq ($(shell uname -r | grep -o fc14), fc14)
	GEAR_VERSION_PLATFORM     ?= fc14
endif
ifeq ($(shell uname -r | grep -o fc17), fc17)
	GEAR_VERSION_PLATFORM     ?= fc17
endif
ifeq ($(shell uname -r | grep -o fc21), fc21)
	GEAR_VERSION_PLATFORM     ?= fc21
endif
ifeq ($(shell uname -r | grep -o fc23), fc23)
	GEAR_VERSION_PLATFORM     ?= fc23
endif
GEAR_VERSION_MILESTONE    ?= $(shell cat $(BUILD_ROOT)/BuildMilestone)
GEAR_VERSION_BRANCH       ?= $(shell git rev-parse --abbrev-ref HEAD)
GEAR_VERSION_USER         ?= $(shell whoami)
GEAR_VERSION_CODE         ?= $(GEAR_VERSION_USER)_$(GEAR_VERSION_BRANCH)_$(GEAR_VERSION_MILESTONE)
GEAR_VERSION               = $(GEAR_VERSION_MAJOR).$(GEAR_VERSION_MINOR)
GEAR_VERSION_FULL          = $(GEAR_VERSION).$(GEAR_VERSION_BUILD).$(GEAR_VERSION_PLATFORM)

###############################################################################
# Product   
###############################################################################

PRODUCT_SKU								?= GEAR

ifneq (,$(filter $(PRODUCT_SKU),GEAR))
PRODUCT_NAME=GEAR
PRODUCT_DESC=GEAR Server
PRODUCT_EDITION=Standard
PACKAGE_NAME=gear
endif

ifneq (,$(filter $(PRODUCT_SKU),HARMONY-DC))
PRODUCT_NAME=Harmony
PRODUCT_DESC=Harmony Cloud Server
PRODUCT_EDITION=Ultimate
PACKAGE_NAME=harmony
endif

ifneq (,$(filter $(PRODUCT_SKU),ICONN-STD ICONN-PRO ICONN-ENT ICONN-OEM ICONN-PL))
PRODUCT_NAME?=iConn
PRODUCT_DESC?=iConn End User Computing Server
PACKAGE_NAME_BASE?=iconn
endif

ifneq (,$(filter $(PRODUCT_SKU),NEXUS-STD NEXUS-PRO NEXUS-ENT NEXUS-OEM NEXUS-PL))
PRODUCT_NAME?=Nexus
PRODUCT_DESC?=Nexus Infrastructure Server
PACKAGE_NAME_BASE?=nexus
endif

ifneq (,$(filter $(PRODUCT_SKU),ICONN-STD NEXUS-STD))
PRODUCT_EDITION=Standard
PACKAGE_NAME=$(PACKAGE_NAME_BASE)-std
endif

ifneq (,$(filter $(PRODUCT_SKU),ICONN-PRO NEXUS-PRO))
PRODUCT_EDITION=Professional
PACKAGE_NAME=$(PACKAGE_NAME_BASE)-pro
endif

ifneq (,$(filter $(PRODUCT_SKU),ICONN-ENT NEXUS-ENT))
PRODUCT_EDITION=Enterprise
PACKAGE_NAME=$(PACKAGE_NAME_BASE)-ent
endif

ifneq (,$(filter $(PRODUCT_SKU),HARMONY-DC))
PRODUCT_EDITION=Datacenter
PACKAGE_NAME_BASE=harmony
PACKAGE_NAME=$(PACKAGE_NAME_BASE)-dc
endif

ifneq (,$(filter $(PRODUCT_SKU),ICONN-OEM NEXUS-OEM))
PRODUCT_EDITION?=OEM
PACKAGE_NAME?=$(PACKAGE_NAME_BASE)-oem
endif

ifneq (,$(filter $(PRODUCT_SKU),ICONN-PL NEXUS-PL))
PRODUCT_EDITION?=PL
PACKAGE_NAME?=$(PACKAGE_NAME_BASE)-pl
endif

OEM_PRODUCT_NAME ?= $(PRODUCT_NAME)
OEM_PRODUCT_DESC ?= $(PRODUCT_DESC)

###############################################################################
# Platform & Build
###############################################################################

BUILDTYPE     ?= release
BUILDARCH     ?= $(shell uname -m)
BUILDHOST     ?= $(shell uname -m)
THISBUILDTYPE ?= $(BUILDTYPE)

ifeq ($(shell uname -r | cut -d '.' -f 5),fc14)
BUILD_PROFILE_SERVER ?= 1
BUILD_PROFILE_CLIENT ?= 1 
else
ifeq ($(shell uname -r | grep -o el7), el7)
BUILD_PROFILE_SERVER ?= 1
BUILD_PROFILE_CLIENT ?= 1
else
BUILD_PROFILE_SERVER ?= 0
BUILD_PROFILE_CLIENT ?= 1 
endif
endif

ifeq ($(OSTYPE),)
THISOSTYPE := $(shell uname)
ifeq ($(THISOSTYPE),Linux)
OSTYPE = linux
else
ifeq ($(THISOSTYPE),Darwin)
OSTYPE = darwin
endif
endif
endif

ifneq ($(findstring $(BUILDARCH),i386 i486 i586 i686 i786 i886 i986 x86 i86pc),)
VBOXBUILDARCH = x86
else
ifneq ($(findstring $(BUILDARCH),x86_64 amd64),)
VBOXBUILDARCH = amd64
else
ifeq ($(BUILDARCH), arm)
VBOXBUILDARCH = arm
else
$(error Unsupported build architecture: $(BUILDARCH))
endif
endif
endif

###############################################################################
# Software License
###############################################################################
PRODUCT_PUBLIC_KEY ?= $(BUILD_ROOT)/dev_public.pem
PRODUCT_PRIVATE_KEY ?= $(BUILD_ROOT)/dev_private.pem
PRODUCT_LMP_KEY ?= $(BUILD_ROOT)/dev_lmp.key

GEAR_LICENSE_SERVICE ?= https://license.filegear.com

###############################################################################
# Default Directories
###############################################################################

BLD    = $(OSTYPE).$(BUILDARCH)/$(BUILDTYPE)
THISBLD= $(OSTYPE).$(BUILDARCH)/$(THISBUILDTYPE)
OUTDIR = $(ROOT)/out
INCDIR = $(OUTDIR)/include
BLDDIR = $(OUTDIR)/$(THISBLD)
BINDIR = $(BLDDIR)/bin
OBJDIR = $(BLDDIR)/obj
LIBDIR = $(BLDDIR)/lib
SYMDIR = $(BLDDIR)/sym

DEPLOYROOT ?= $(HOME)/builds
DEPLOYDIR ?= $(DEPLOYROOT)/$(GEAR_VERSION_MAJOR)-$(GEAR_VERSION_MINOR)-$(GEAR_VERSION_BUILD).$(GEAR_VERSION_CODE)

VBOXOUT = $(ROOT_VBOX)/out
VBOXBLD = $(VBOXOUT)/$(OSTYPE).$(VBOXBUILDARCH)/$(BUILDTYPE)

ifeq ($(OSTYPE),darwin)
VBOXLIB = $(VBOXBLD)/dist/VirtualBox.app/Contents/MacOS
SHARELIBEXT=dylib
else
VBOXLIB = $(VBOXBLD)/bin
SHARELIBEXT=so
endif

VBoxRT_LIB = $(VBOXLIB)/VBoxRT.$(SHARELIBEXT)

