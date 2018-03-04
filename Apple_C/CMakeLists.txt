#
# Copyright (c) 2017-2018 Volker Wiegand <volker@railduino.de>
#
# This file is part of Zeroconf-Lookup.
# Project home: https://www.railduino.de/zeroconf-lookup
# Source code:  https://github.com/railduino/zeroconf-lookup.git
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
#
# A personal note:
#   Most valuable resources learning CMake (this is my first CMake project):
#   - http://rachid.koucha.free.fr/tech_corner/cmake_manual.html
#   - http://cicero.xyz/v2/remark/github/coderefinery/lessons/master/cmake-kung-fu.mkd/
#   - https://samthursfield.wordpress.com/2015/11/21/cmake-dependencies-between-targets-and-files-and-custom-commands/
#   - https://stackoverflow.com/questions/6003374/what-is-cmake-equivalent-of-configure-prefix-dir-make-all-install
#   - https://mirkokiefer.com/cmake-by-example-f95eb47d45b1
#

###########################################################
#
# Part 1: The project
#
###########################################################

cmake_minimum_required (VERSION 3.10)
project (zeroconf_lookup C)

set (VERSION_MAJOR 2)
set (VERSION_MINOR 4)
set (VERSION_PATCH 1)
set (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

set (GOOGLE_TAG  gikfkgfjepbdpiljbieedpkcjikapbbg)
set (MOZILLA_TAG zeroconf_lookup@railduino.com)


###########################################################
#
# Part 2: Compile C source
#
###########################################################

add_definitions (-W -Wall -Wextra -Wshadow -Wstrict-prototypes -Wpointer-arith -Wcast-qual -Winline -Werror)

file (GLOB SOURCES "src/*.c")
include_directories (${CMAKE_BINARY_DIR})

add_executable (zeroconf_lookup ${SOURCES})


###########################################################
#
# Part 3: Find OS dependent libraies and link
#
###########################################################

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Hint: DNSSD is actually mDNSResponder
find_package (DNSSD REQUIRED)
if (DNSSD_FOUND)
  include_directories (${DNSSD_INCLUDE_DIRS})
  target_link_libraries (zeroconf_lookup ${DNSSD_LIBRARIES})
endif (DNSSD_FOUND)

configure_file (config.h.in config.h)

install (TARGETS zeroconf_lookup RUNTIME DESTINATION bin)

