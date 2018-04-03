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

-include $(ROOT)/Config.mk

ifeq ($(LOCALE),)
$(error "Please define LOCALE")
endif

ifeq ($(BINDIR),)
BINDIR = bin
endif

MODIR = $(BINDIR)/$(LOCALE)

_MOS  = ${LOCSRCS:.po=.mo}
MOS   = $(patsubst %,$(MODIR)/%,$(_MOS))

LOCTARGETS = ${LOCSRCS:.po=}


.PHONY: all modir update clean

all: modir $(MOS)


modir:
	-@mkdir -p $(MODIR)


$(MODIR)/%.mo: %.po
	@echo "msgfmt: $@"
	@msgfmt -o $@ $<


update:
	@echo "Merge the current localization with the PO template."
	@for p in $(LOCTARGETS); do ( if [ -e $$p.po ]; then msgmerge -U $$p.po $(OBJDIR)/$$p.pot; else msginit -i $(OBJDIR)/$$p.pot -o $$p.po -l $(LOCALE).UTF-8 --no-translator; fi ); done


clean:
	-@for m in $(MOS); do ( rm -f $$m ); done

