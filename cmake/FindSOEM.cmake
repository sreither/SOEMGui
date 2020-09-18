# - Try to find soem
# Once done this will define
#
#  SOEM_FOUND - soem found
#  SOEM_INCLUDE_DIR - the soem include directory
#  SOEM_LIBRARIES - soem library
#

if(NOT "$ENV{SOEM_DIR}" EQUAL "")
    set(SOEM_DIR $ENV{SOEM_DIR} CACHE PATH "Path to SOEM" FORCE)
endif()

set(HEADER_SEARCH_PATHS
    ${SOEM_DIR}/include/
    ${SOEM_DIR}/include/soem/
    ${SOEM_DIR}/../soem/         #if SOEM_DIR points to build
    ${SOEM_DIR}/../osal/         #if SOEM_DIR points to build
    ${SOEM_DIR}/../oshw/         #if SOEM_DIR points to build
    ${SOEM_DIR}/../osal/linux/   #if SOEM_DIR points to build
    ${SOEM_DIR}/../oshw/linux/   #if SOEM_DIR points to build
    ENV CPATH
    /usr/include/
    /usr/include/soem/
    /usr/include/
    /usr/local/include/soem/
    /opt/local/include/soem/
)

#if soem is used from build, all headers are scattered around
#find all of them and add them to SOEM_INCLUDE_DIR
find_path(SOEM_INCLUDE_DIR_0 NAMES ethercatmain.h PATHS ${HEADER_SEARCH_PATHS} NO_DEFAULT_PATH)

find_path(SOEM_INCLUDE_DIR_1 NAMES osal.h PATHS ${HEADER_SEARCH_PATHS} NO_DEFAULT_PATH)
find_path(SOEM_INCLUDE_DIR_2 NAMES osal_defs.h PATHS ${HEADER_SEARCH_PATHS} NO_DEFAULT_PATH)

find_path(SOEM_INCLUDE_DIR_3 NAMES oshw.h PATHS ${HEADER_SEARCH_PATHS} NO_DEFAULT_PATH)
find_path(SOEM_INCLUDE_DIR_4 NAMES nicdrv.h PATHS ${HEADER_SEARCH_PATHS} NO_DEFAULT_PATH)

if(SOEM_INCLUDE_DIR_0 AND SOEM_INCLUDE_DIR_1 AND SOEM_INCLUDE_DIR_2 AND SOEM_INCLUDE_DIR_3 AND SOEM_INCLUDE_DIR_4)
    set(SOEM_INCLUDE_DIR ${SOEM_INCLUDE_DIR_0} ${SOEM_INCLUDE_DIR_1} ${SOEM_INCLUDE_DIR_2} ${SOEM_INCLUDE_DIR_3} ${SOEM_INCLUDE_DIR_4})
endif()

FIND_LIBRARY(SOEM_LIBRARIES NAMES soem
  PATHS
  ${SOEM_DIR}                #if SOEM_DIR points to build
  ${SOEM_DIR}/lib
  ENV LD_LIBRARY_PATH
  ENV LIBRARY_PATH
  /usr/lib
  /usr/local/lib
  /opt/local/lib
  NO_DEFAULT_PATH
)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set OODL_YOUBOT_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(SOEM  DEFAULT_MSG
                                  SOEM_LIBRARIES SOEM_INCLUDE_DIR)


# show the SOEM_INCLUDE_DIR and SOEM_LIBRARY_DIR variables only in the advanced view
MARK_AS_ADVANCED(SOEM_INCLUDE_DIR SOEM_LIBRARIES)
