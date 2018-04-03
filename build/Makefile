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

ROOT = $(realpath .)

include Config.mk

all:
	@echo Done

showver:
	@echo "$(GEAR_VERSION_FULL) ($(GEAR_VERSION_CODE))"

incver:
	@let "nb=$(BUILD_NUMBER) + 1"; echo $$nb > BuildNumber; git commit -m "Updating BuildNumber to $$nb" -- BuildNumber; git push origin HEAD;

sync:
	@git pull --ff-only

clean:
	@true

reset:
	@git reset --hard

tagver:
	@echo "tag: $(GEAR_VERSION_FULL)"
	@git tag -m "Tagging $(GEAR_VERSION_FULL)" $(GEAR_VERSION_FULL) && git push --tags

deploy:
	@echo -n ""

dailybuild: incver
	@$(MAKE) -f BuildAll.mk

exportvars:
	@echo "GEAR_VERSION_MAJOR=$(GEAR_VERSION_MAJOR)"
	@echo "GEAR_VERSION_MINOR=$(GEAR_VERSION_MINOR)"
	@echo "GEAR_VERSION_BUILD=$(GEAR_VERSION_BUILD)"
	@echo "GEAR_VERSION_CODE=$(GEAR_VERSION_CODE)"
	@echo "GEAR_VERSION=$(GEAR_VERSION)"
	@echo "GEAR_VERSION_FULL=$(GEAR_VERSION_FULL)"
	@echo "DEPLOYDIR=$(DEPLOYDIR)"

createbranch:
	@if [[ "$(branch)" == "" ]]; then echo "Usage: make createbranch branch={branch}"; exit 1; fi
	git push origin master:refs/heads/$(branch)
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
	@ctags -R

fetch:
	git fetch
