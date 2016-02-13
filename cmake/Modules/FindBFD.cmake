# - Find BFD library
# Find the native BFD includes and library
# This module defines
#  BFD_INCLUDE_DIRS, where to find bfd.h, Set when BFD is found.
#  BFD_LIBRARIES, libraries to link against to use BFD.
#  BFD_ROOT_DIR, The base directory to search for BFD.
#                This can also be an environment variable.
#  BFD_FOUND, If false, do not try to use PugiXML.
#
# also defined, but not for general use are
#  BFD_LIBRARY, where to find the PugiXML library.

#=============================================================================
# Copyright 2016 Blender Foundation.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

# If BFD_ROOT_DIR was defined in the environment, use it.
IF(NOT BFD_ROOT_DIR AND NOT $ENV{BFD_ROOT_DIR} STREQUAL "")
  SET(BFD_ROOT_DIR $ENV{BFD_ROOT_DIR})
ENDIF()

SET(_bfd_SEARCH_DIRS
  ${BFD_ROOT_DIR}
  /usr/local
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt/lib/libbfd
)

FIND_PATH(BFD_INCLUDE_DIR
  NAMES
    bfd.h
  HINTS
    ${_bfd_SEARCH_DIRS}
  PATH_SUFFIXES
    include
)

FIND_LIBRARY(BFD_LIBRARY
  NAMES
    bfd
  HINTS
    ${_bfd_SEARCH_DIRS}
  PATH_SUFFIXES
    lib64 lib
  )

# handle the QUIETLY and REQUIRED arguments and set BFD_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BFD DEFAULT_MSG
    BFD_LIBRARY BFD_INCLUDE_DIR)

IF(BFD_FOUND)
  SET(BFD_LIBRARIES ${BFD_LIBRARY})
  SET(BFD_INCLUDE_DIRS ${BFD_INCLUDE_DIR})
ENDIF()

MARK_AS_ADVANCED(
  BFD_INCLUDE_DIR
  BFD_LIBRARY
)
