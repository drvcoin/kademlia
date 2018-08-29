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

if (NOT DEFINED ROOT)
  message(FATAL_ERROR "Please define ROOT variable to point to the repository's root folder.")
endif (NOT DEFINED ROOT)

set(OUTDIR ${ROOT}/out)
set(BINDIR ${OUTDIR}/bin)
set(LIBDIR ${OUTDIR}/lib)
set(INCDIR ${OUTDIR}/inc)
set(CFGDIR ${OUTDIR}/cfg)
set(PKGDIR ${OUTDIR}/pkg)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINDIR})

# Parent module
if (NOT DEFINED ROOT_DRIVE)
  set(ROOT_DRIVE ${ROOT}/../.. CACHE PATH "Drive directory" FORCE)
endif (NOT DEFINED ROOT_DRIVE)

include(${ROOT}/Cpp.cmake)
