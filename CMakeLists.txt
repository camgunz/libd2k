PROJECT("libd2k" C)

CMAKE_MINIMUM_REQUIRED(VERSION 2.9.0 FATAL_ERROR)

INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckSymbolExists)
INCLUDE(TestBigEndian)

CMAKE_POLICY(SET CMP0042 NEW)

IF(${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
  MESSAGE(FATAL_ERROR "Cannot build source in-tree.")
ENDIF()

IF((NOT DEFINED CMAKE_BUILD_TYPE) OR (CMAKE_BUILD_TYPE STREQUAL ""))
  SET(CMAKE_BUILD_TYPE Release)
  SET(DEBUGGING FALSE)
ENDIF()

IF(CMAKE_BUILD_TYPE STREQUAL "Release")
  SET(DEBUGGING FALSE)
ELSEIF(CMAKE_BUILD_TYPE STREQUAL "Debug")
  SET(DEBUGGING TRUE)
ELSE()
  MESSAGE(FATAL_ERROR "Unknown build type ${CMAKE_BUILD_TYPE}")
ENDIF()

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

SET(VERSION "0")
SET(PACKAGE "LibD2K")
SET(PACKAGE_NAME "LibD2K")
SET(PACKAGE_VERSION "${VERSION}")
SET(PACKAGE_STRING "${PACKAGE_NAME} v${PACKAGE_VERSION}")
SET(PACKAGE_TARNAME "libd2k")
SET(PACKAGE_URL "http://github.com/camgunz/libd2k")

SET(CPACK_GENERATOR "DEB;RPM;STGZ;ZIP")
SET(CPACK_PACKAGE_VENDOR "Team TotalTrash")
SET(CPACK_PACKAGE_CONTACT "Team TotalTrash <charles.gunyon@gmail.com>")
SET(CPACK_PACKAGE_VERSION "${VERSION}")
SET(CPACK_PACKAGE_NAME "${PACKAGE_TARNAME}")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "A Doom Library for the 20th Century"
)
SET(CPACK_DEBIAN_PACKAGE_SECTION "devel")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.4)")

IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "i686")
  SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
  SET(CPACK_RPM_PACKAGE_ARCHITECTURE "i686" )
  SET(BUILD_ARCH "${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
ELSEIF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
  SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
  SET(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64" )
  SET(BUILD_ARCH "${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
ELSEIF(WIN32)
  SET(BUILD_ARCH "windows-x86")
  IF("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "AMD64")
    SET(BUILD_ARCH "windows-$ENV{PROCESSOR_ARCHITECTURE}")
  ENDIF()
ELSE()
  SET(BUILD_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
ENDIF()

SET(CPACK_PACKAGE_FILE_NAME
  "${CPACK_PACKAGE_NAME}-v${CPACK_PACKAGE_VERSION}_${BUILD_ARCH}"
)
SET(CPACK_DEBIAN_PACKAGE_NAME "${CPACK_PACKAGE_FILE_NAME}")
SET(CPACK_RPM_PACKAGE_NAME "${CPACK_PACKAGE_FILE_NAME}")

IF(CMAKE_C_COMPILER_ID STREQUAL "Clang")
  SET(CLANG TRUE)
ELSE()
  SET(CLANG FALSE)
ENDIF()

IF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  SET(GCC TRUE)
ELSE()
  SET(GCC FALSE)
ENDIF()

IF(CLANG OR GCC)
  SET(GCC_OR_CLANG TRUE)
ELSE()
  SET(GCC_OR_CLANG FALSE)
ENDIF()

CHECK_SYMBOL_EXISTS(_stricmp string.h HAVE_STRING__STRICMP)
CHECK_SYMBOL_EXISTS(_stricmp strings.h HAVE_STRINGS__STRICMP)
CHECK_SYMBOL_EXISTS(stricmp string.h HAVE_STRING_STRICMP)
CHECK_SYMBOL_EXISTS(stricmp strings.h HAVE_STRINGS_STRICMP)
CHECK_SYMBOL_EXISTS(strcasecmp string.h HAVE_STRING_STRCASECMP)
CHECK_SYMBOL_EXISTS(strcasecmp strings.h HAVE_STRINGS_STRCASECMP)

IF(HAVE_STRING_STRICMP OR HAVE_STRINGS_STRICMP)
  SET(stricmp stricmp)
ELSEIF(HAVE_STRING__STRICMP OR HAVE_STRINGS__STRICMP)
  SET(stricmp _stricmp)
ELSEIF(HAVE_STRING_STRCASECMP OR HAVE_STRINGS_STRCASECMP)
  SET(stricmp strcasecmp)
ELSE()
  MESSAGE(FATAL_ERROR "Neither _stricmp, stricmp, or strcasecmp found")
ENDIF()

CHECK_SYMBOL_EXISTS(_strnicmp string.h HAVE_STRING__STRNICMP)
CHECK_SYMBOL_EXISTS(_strnicmp strings.h HAVE_STRINGS__STRNICMP)
CHECK_SYMBOL_EXISTS(strnicmp string.h HAVE_STRING_STRNICMP)
CHECK_SYMBOL_EXISTS(strnicmp strings.h HAVE_STRINGS_STRNICMP)
CHECK_SYMBOL_EXISTS(strncasecmp string.h HAVE_STRING_STRNCASECMP)
CHECK_SYMBOL_EXISTS(strncasecmp strings.h HAVE_STRINGS_STRNCASECMP)

IF(HAVE_STRING_STRNICMP OR HAVE_STRINGS_STRNICMP)
  SET(strnicmp strnicmp)
ELSEIF(HAVE_STRING__STRNICMP OR HAVE_STRINGS__STRNICMP)
  SET(strnicmp _strnicmp)
ELSEIF(HAVE_STRING_STRCASECMP OR HAVE_STRINGS_STRCASECMP)
  SET(strnicmp strncasecmp)
ELSE()
  MESSAGE(FATAL_ERROR "Neither _strnicmp, strnicmp, or strncasecmp found")
ENDIF()

FUNCTION(ADD_CFLAG flag flag_supported)
  CHECK_C_COMPILER_FLAG(${flag} ${flag_supported})
  IF(${flag_supported})
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" PARENT_SCOPE)
  ENDIF()
ENDFUNCTION(ADD_CFLAG flag flag_supported)

CHECK_C_COMPILER_FLAG(-std=c99 C99_FLAG_SUPPORTED)
IF(${C99_FLAG_SUPPORTED})
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
ENDIF()

ADD_CFLAG(-march=native ARCH_NATIVE_FLAG_SUPPORTED)
ADD_CFLAG(-Wextra WARN_EXTRA_FLAG_SUPPORTED)
ADD_CFLAG(-Wall WARN_ALL_FLAG_SUPPORTED)

IF(DEBUGGING)
  IF(GCC)
    ADD_CFLAG(-fstack-check STACK_CHECK_FLAG_SUPPORTED)
    ADD_CFLAG(-pedantic-errors PEDANTIC_FLAG_SUPPORTED)
  ELSEIF(CLANG)
    ADD_CFLAG(-Werror WARNINGS_ARE_ERRORS_FLAG_SUPPORTED)
    ADD_CFLAG(-fsanitize=address ASAN_FLAG_SUPPORTED)
    # ADD_CFLAG(-fsanitize=address,thread,undefined
    #   ADDRESS_THREAD_UB_SANITIZATION_SUPPORTED)
  ENDIF()
  ADD_CFLAG(-fstack-protector-all STACK_PROTECTOR_ALL_FLAG_SUPPORTED)
  ADD_CFLAG(-ftrapv TRAPV_FLAG_SUPPORTED)
  ADD_CFLAG(-fbounds-checking BOUNDS_CHECKING_FLAG_SUPPORTED)
  ADD_CFLAG(-O0 O0_FLAG_SUPPORTED)
ELSE()
  ADD_CFLAG(-O3 O3_FLAG_SUPPORTED)
ENDIF()

IF(DEBUGGING)
  IF(CLANG)
    ADD_CFLAG(--coverage COVERAGE_SUPPORTED)
  ELSEIF(GCC)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
  ENDIF()
ENDIF()

ADD_DEFINITIONS(-DHAVE_CONFIG_H)
CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

FIND_PACKAGE(CBase REQUIRED)
INCLUDE_DIRECTORIES(${CBASE_INCLUDE_DIR})

FIND_PACKAGE(UTF8Proc REQUIRED)
INCLUDE_DIRECTORIES(${UTF8PROC_INCLUDE_DIR})

FIND_PACKAGE(MPDecimal REQUIRED)
INCLUDE_DIRECTORIES(${MPDECIMAL_INCLUDE_DIR})

FIND_PACKAGE(Iconv REQUIRED)
INCLUDE_DIRECTORIES(${ICONV_INCLUDE_DIR})

FIND_PACKAGE(Cmocka REQUIRED)
INCLUDE_DIRECTORIES(${CMOCKA_INCLUDE_DIR})

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

IF(GCC_OR_CLANG)
  ADD_DEFINITIONS(-D_GNU_SOURCE)
ENDIF()

OPTION(SPECIFY_ENDIANNESS
  "Use the value of 'FORCE_BIGENDIAN' instead of testing; for cross-compiling"
  OFF)
OPTION(FORCE_BIGENDIAN "Force bigendian mode" OFF)

IF(SPECIFY_ENDIANNESS)
  SET(BIGENDIAN ${FORCE_BIGENDIAN})
ELSE()
  TEST_BIG_ENDIAN(BIGENDIAN)
ENDIF()

IF(${BIGENDIAN})
  ADD_DEFINITIONS(-DWORDS_BIGENDIAN)
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)

SET(LIBD2K_SOURCE_FILES
  ${CMAKE_SOURCE_DIR}/src/angle.c
  ${CMAKE_SOURCE_DIR}/src/map.c
  ${CMAKE_SOURCE_DIR}/src/map_blockmap.c
  ${CMAKE_SOURCE_DIR}/src/map_linedefs.c
  ${CMAKE_SOURCE_DIR}/src/map_loader.c
  ${CMAKE_SOURCE_DIR}/src/map_nodes.c
  ${CMAKE_SOURCE_DIR}/src/map_sectors.c
  ${CMAKE_SOURCE_DIR}/src/map_segs.c
  ${CMAKE_SOURCE_DIR}/src/map_sidedefs.c
  ${CMAKE_SOURCE_DIR}/src/map_subsectors.c
  ${CMAKE_SOURCE_DIR}/src/map_vertexes.c
  ${CMAKE_SOURCE_DIR}/src/wad.c
)

SET(INCLUDE_FILES
  ${CMAKE_SOURCE_DIR}/src/d2k.h
  ${CMAKE_SOURCE_DIR}/src/d2k/internal.h
  ${CMAKE_SOURCE_DIR}/src/d2k/alloc.h
  ${CMAKE_SOURCE_DIR}/src/d2k/angle.h
  ${CMAKE_SOURCE_DIR}/src/d2k/fixed_math.h
  ${CMAKE_SOURCE_DIR}/src/d2k/fixed_vertex.h
  ${CMAKE_SOURCE_DIR}/src/d2k/map.h
  ${CMAKE_SOURCE_DIR}/src/d2k/map_blockmap.h
  ${CMAKE_SOURCE_DIR}/src/d2k/map_linedefs.h
  ${CMAKE_SOURCE_DIR}/src/d2k/map_loader.h
  ${CMAKE_SOURCE_DIR}/src/d2k/map_nodes.h
  ${CMAKE_SOURCE_DIR}/src/d2k/map_object.h
  ${CMAKE_SOURCE_DIR}/src/d2k/map_object_info.h
  ${CMAKE_SOURCE_DIR}/src/d2k/map_object_type.h
  ${CMAKE_SOURCE_DIR}/src/d2k/map_problem.h
  ${CMAKE_SOURCE_DIR}/src/d2k/map_sectors.h
  ${CMAKE_SOURCE_DIR}/src/d2k/map_segs.h
  ${CMAKE_SOURCE_DIR}/src/d2k/map_sidedefs.h
  ${CMAKE_SOURCE_DIR}/src/d2k/map_subsectors.h
  ${CMAKE_SOURCE_DIR}/src/d2k/map_vertexes.h
  ${CMAKE_SOURCE_DIR}/src/d2k/sound_origin.h
  ${CMAKE_SOURCE_DIR}/src/d2k/sprite.h
  ${CMAKE_SOURCE_DIR}/src/d2k/thinker.h
  ${CMAKE_SOURCE_DIR}/src/d2k/wad.h
)

SET(LIBD2K_LIBRARIES
  ${UTF8PROC_LIBRARIES}
  ${MPDECIMAL_LIBRARIES}
  ${ICONV_LIBRARIES}
  ${CBASE_LIBRARIES}
  m
)

SET(D2KTEST_LIBRARIES ${LIBD2K_LIBRARIES}
  ${CMOCKA_LIBRARIES}
)

ADD_LIBRARY(d2kstaticlib STATIC ${LIBD2K_SOURCE_FILES})
TARGET_LINK_LIBRARIES(d2kstaticlib ${LIBD2K_LIBRARIES})
SET_TARGET_PROPERTIES(d2kstaticlib PROPERTIES OUTPUT_NAME d2k)

ADD_LIBRARY(d2ksharedlib SHARED ${LIBD2K_SOURCE_FILES})
TARGET_LINK_LIBRARIES(d2ksharedlib ${LIBD2K_LIBRARIES})
SET_TARGET_PROPERTIES(d2ksharedlib PROPERTIES OUTPUT_NAME d2k)

ADD_EXECUTABLE(d2k_test ${LIBD2K_SOURCE_FILES}
  ${CMAKE_SOURCE_DIR}/test/main.c
  ${CMAKE_SOURCE_DIR}/test/basic.c
  ${CMAKE_SOURCE_DIR}/test/blockmap.c
  ${CMAKE_SOURCE_DIR}/test/map.c
  ${CMAKE_SOURCE_DIR}/test/wad.c
)
TARGET_LINK_LIBRARIES(d2k_test ${D2KTEST_LIBRARIES})

IF((NOT DEFINED BIN_DIR) OR (BIN_DIR STREQUAL ""))
  SET(BIN_DIR "${PREFIX}/bin")
ENDIF()

IF((NOT DEFINED LIB_DIR) OR (LIB_DIR STREQUAL ""))
  SET(LIB_DIR "${PREFIX}/lib")
ENDIF()

IF((NOT DEFINED INCLUDE_DIR) OR (INCLUDE_DIR STREQUAL ""))
  SET(INCLUDE_DIR "${PREFIX}/include")
ENDIF()

IF((NOT DEFINED SHARE_DIR) OR (SHARE_DIR STREQUAL ""))
  SET(SHARE_DIR "${PREFIX}/share")
ENDIF()

INSTALL(
  TARGETS d2kstaticlib d2ksharedlib
  RUNTIME DESTINATION ${BIN_DIR}
  LIBRARY DESTINATION ${LIB_DIR}
  ARCHIVE DESTINATION ${LIB_DIR}
)

INSTALL(
  FILES ${INCLUDE_FILES}
  DESTINATION ${INCLUDE_DIR}/d2k
)

INSTALL(
  FILES ${CMAKE_SOURCE_DIR}/src/d2k.h
  DESTINATION ${INCLUDE_DIR}
)

# vi: ai:ts=2:sw=2:et
