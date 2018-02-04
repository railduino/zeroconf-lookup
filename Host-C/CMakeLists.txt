#
# Copyright (c) 2017-2018 Volker Wiegand <volker@railduino.de>
#
# This file is part of Zeroconf-Lookup.
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
#

###########################################################
#
# Part 1: The project
#
###########################################################

cmake_minimum_required (VERSION 3.0)
project (zeroconf_lookup C)

set (VERSION_MAJOR 2)
set (VERSION_MINOR 1)
set (VERSION_PATCH 0)
set (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})


###########################################################
#
# Part 2: Compile C source
#
###########################################################

if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
  add_definitions (-W -Wall -Wextra -Wshadow -Wstrict-prototypes -Wpointer-arith -Wcast-qual -Winline -Werror)
elseif (${CMAKE_C_COMPILER_ID} STREQUAL "MSVC")
  set (warnings "/W4 /WX /EHsc")
endif ()

include (CheckIncludeFile)
check_include_file ("winsock2.h"    HAVE_WINSOCK2_H)
check_include_file ("ws2tcpip.h"    HAVE_WS2TCPIP_H)
check_include_file ("io.h"          HAVE_IO_H)

check_include_file ("stdio.h"       HAVE_STDIO_H)
check_include_file ("stddef.h"      HAVE_STDDEF_H)
check_include_file ("stdlib.h"      HAVE_STDLIB_H)
check_include_file ("stdarg.h"      HAVE_STDARG_H)
check_include_file ("string.h"      HAVE_STRING_H)
check_include_file ("time.h"        HAVE_TIME_H)
check_include_file ("errno.h"       HAVE_ERRNO_H)
check_include_file ("stdint.h"      HAVE_STDINT_H)
check_include_file ("sys/types.h"   HAVE_SYS_TYPES_H)
check_include_file ("unistd.h"      HAVE_UNISTD_H)
check_include_file ("fcntl.h"       HAVE_FCNTL_H)
check_include_file ("sys/stat.h"    HAVE_SYS_STAT_H)
check_include_file ("getopt.h"      HAVE_GETOPT_H)
check_include_file ("syslog.h"      HAVE_SYSLOG_H)
check_include_file ("poll.h"        HAVE_POLL_H)
check_include_file ("sys/poll.h"    HAVE_SYS_POLL_H)

check_include_file ("netdb.h"       HAVE_NETDB_H)
check_include_file ("socket.h"      HAVE_SOCKET_H)
check_include_file ("sys/socket.h"  HAVE_SYS_SOCKET_H)
check_include_file ("netinet/in.h"  HAVE_NETINET_IN_H)
check_include_file ("arpa/inet.h"   HAVE_ARPA_INET_H)

check_include_file ("mach-o/dyld.h" HAVE_MACH_O_DYLD_H)

include (CheckSymbolExists)
check_symbol_exists (WSAStartup  "winsock2.h" HAVE_WSASTARTUP)
check_symbol_exists (getopt_long "getopt.h"   HAVE_GETOPT_LONG)
check_symbol_exists (_setmode    "io.h"       HAVE_SETMODE)
check_symbol_exists (strncpy_s   "string.h"   HAVE_STRNCPY_S)
check_symbol_exists (alarm       "signal.h;unistd.h" HAVE_ALARM)

file (GLOB SOURCES "src/*.c")
include_directories (${CMAKE_BINARY_DIR})

add_executable (zeroconf_lookup ${SOURCES})


###########################################################
#
# Part 3: Find OS dependent libraies and linking
#
###########################################################

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package (Avahi)
if (AVAHI_FOUND)
  include_directories (${AVAHI_INCLUDE_DIRS})
  target_link_libraries (zeroconf_lookup ${AVAHI_LIBRARIES})
endif (AVAHI_FOUND)

# Hint: DNSSD is actually mDNSResponder
find_package (DNSSD)
if (DNSSD_FOUND)
  include_directories (${DNSSD_INCLUDE_DIRS})
  target_link_libraries (zeroconf_lookup ${DNSSD_LIBRARIES})
endif (DNSSD_FOUND)

if (HAVE_NETINET_IN_H)
  if (HAVE_ARPA_INET_H)
    if (HAVE_NETDB_H)
      if (HAVE_POLL_FUNC)
        set (QUERY_FOUND TRUE)
      endif (HAVE_POLL_FUNC)
    endif (HAVE_NETDB_H)
  endif (HAVE_ARPA_INET_H)
endif (HAVE_NETINET_IN_H)

configure_file (config.h.in config.h)

install (TARGETS zeroconf_lookup RUNTIME DESTINATION bin)


###########################################################
#
# Part 4: Create OS specific packages
#
###########################################################

