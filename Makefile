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

ROOT := $(realpath .)
include $(ROOT)/Config.mk

DIRS = \
	src

.PHONY: all clean sync tagver deploy createbranch install checkout pull fetch commit push ctags incver $(DIRS)

all: $(DIRS)

clean: 
	-@rm -rf out/

reset:
	@git reset --hard

$(DIRS):
	@$(MAKE) -C $@ $(MAKECMDGOALS)

sync:
	@git pull --ff-only

tagver:
	@echo "tag: $(GEAR_VERSION_FULL)"
	@git tag -m "Tagging $(GEAR_VERSION_FULL)" $(GEAR_VERSION_FULL) && git push --tags

deploy:
	@echo "deploy: $(DEPLOYDIR)/external.$(GEAR_VERSION_PLATFORM).$(BUILDARCH).tgz"
	@mkdir -p $(DEPLOYDIR)
	@tar -czf $(DEPLOYDIR)/external.$(GEAR_VERSION_PLATFORM).$(BUILDARCH).tgz --exclude "*/obj/*" out

createbranch:
	@if [[ "$(branch)" == "" ]]; then echo "Usage: make createbranch branch={branch}"; exit 1; fi
	git push origin origin:refs/heads/$(branch)
	git fetch origin
	git checkout --track -b $(branch) origin/$(branch)
	git pull

checkout:
	@if [[ "$(branch)" == "" ]]; then echo "Usage: make checkout branch={branch}"; exit 1; fi
	git checkout $(branch)
	git pull

pull:
	git pull

commit:
	@if [[ "$(message)" == "" ]]; then echo -e "Usage: make commit message={message} [ args=... ]\n\nExample: make commit message=\"Fixing bug 1234\" args=-a"; exit 1; fi
	git commit $(args) -m "$(message)"

push:
	git push origin HEAD

ctags:
	@pushd $(ROOT)/src > /dev/null; ctags --tag-relative=yes -R -f $(ROOT)/tags;

fetch:
	git fetch

incver:
	@pushd build; make incver; popd
