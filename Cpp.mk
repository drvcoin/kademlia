#
# Copyright (c) 2018 Drive Foundation
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

_OBJS0 = ${SRCS:.cpp=.o}
_OBJS1 = ${_OBJS0:.c=.o}
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS1))

_DEPS0 = ${SRCS:.cpp=.dep}
_DEPS1 = ${_DEPS0:.c=.dep}
DEPS = $(patsubst %,$(OBJDIR)/%,$(_DEPS1))

HDRS = $(patsubst %,$(INCDIR)/%,$(HEADERS))

OUTLIB = $(patsubst %,$(BINDIR)/%.a,$(TARGETLIB))
OUTSO  = $(patsubst %,$(BINDIR)/%.so,$(TARGETSO))
OUTEXE = $(patsubst %,$(BINDIR)/%,$(TARGETEXE))

CCFLAGS += -g -Wall

ifeq ($(BUILDTYPE),debug)
CCFLAGS += -DDEBUG
endif

define cc-command
@echo "compile: $<"
@$(CC) $(CCFLAGS) -Wp,-MD,${patsubst %.o,%.dep,$@} -Wp,-MT,$@ -Wp,-MP -o $@ -c $<
endef

.PHONY: all clean

all: $(OUTSO) $(OUTLIB) $(OUTEXE) $(HDRS)

$(OUTEXE): $(OBJS) $(LIBS)
	@echo "link: $@"
	@mkdir -p $$(dirname $@)
	@$(CXX) ${CCFLAGS} ${LDFLAGS} -o $@ $^ $(SYSLIBS)

$(OUTLIB): $(OBJS)
	@echo "link: $@"
	@mkdir -p $$(dirname $@)
	@rm -f $@
	@ar -cvq $@ $^

$(OUTSO): $(OBJS) $(LIBS)
	@echo "link: $@"
	@mkdir -p $$(dirname $@)
	@$(CXX) -shared -fvisibility=default ${CCFLAGS} ${LDFLAGS} -o $@ $^ $(SYSLIBS)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $$(dirname $@)
	$(cc-command)

$(OBJDIR)/%.o: %.c
	@mkdir -p $$(dirname $@)
	$(cc-command)

$(INCDIR)/%.h: %.h
	@mkdir -p $$(dirname $@)
	@cp -rf $< $@

clean:
	-@rm -rf $(OBJDIR)
	-@rm -rf $(OUTLIB)
	-@rm -rf $(OUTSO)
	-@rm -rf $(OUTEXE)
