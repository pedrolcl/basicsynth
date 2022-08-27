cmake_minimum_required(VERSION 3.15)

project(basicsynth
    LANGUAGES CXX
    VERSION 1.5.1
)

include(GNUInstallDirs)

if (WIN32)
    set(_INIT_TINYXML OFF)
else()
    set(_INIT_TINYXML ON)
endif()

option(USE_LIBXML "Use libXml instead of TinyXML" OFF)
option(USE_TINYXML "Use TinyXML instead of libXML" ${_INIT_TINYXML})
option(USE_OSCILI "Use interpolated oscillators" ON)
option(BUILD_BASICSYNTH_SHARED "Build a shared library" ON)
option(BUILD_BASICSYNTH_STATIC "Build a static library" ON)
option(BUILD_EXAMPLES "Build example programs" ON)

if (CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND PROJECT_COMMON_FLAGS "-DGCC")
endif()

if (UNIX)
    list(APPEND PROJECT_COMMON_FLAGS "-DUNIX")
    if (NOT APPLE)
        find_library(MATH_LIBRARY m)
        #message(STATUS "MATH lib: ${MATH_LIBRARY}")
    endif()
    if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
        find_package(ALSA REQUIRED)
        find_package(Threads REQUIRED)
    endif()
endif()

if (USE_LIBXML)
    find_package(LibXml2 REQUIRED)
    list(APPEND PROJECT_COMMON_FLAGS "-DUSE_LIBXML")
elseif (USE_TINYXML)
    list(APPEND PROJECT_COMMON_FLAGS "-DUSE_TINYXML")
elseif (WIN32)
    list(APPEND PROJECT_COMMON_FLAGS "-DUSE_MSXML")
endif()    

include(TestBigEndian)
test_big_endian(TARGET_SYSTEM_IS_BIG_ENDIAN)
if (TARGET_SYSTEM_IS_BIG_ENDIAN)
    list(APPEND PROJECT_COMMON_FLAGS "-DBS_BIG_ENDIAN=1;-DSYNTH_BIG_ENDIAN=1")
endif()

if(USE_OSCILI)
    list(APPEND PROJECT_COMMON_FLAGS "-DUSE_OSCILI")
endif()

add_subdirectory(Src)

if(IS_ABSOLUTE ${CMAKE_INSTALL_LIBDIR})
    set(basicsynth_libdir "${CMAKE_INSTALL_LIBDIR}")
else()
    set(basicsynth_libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
endif()
if(IS_ABSOLUTE ${CMAKE_INSTALL_INCLUDEDIR})
    set(basicsynth_includedir "${CMAKE_INSTALL_INCLUDEDIR}")
else()
    set(basicsynth_includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
endif()

if (BUILD_BASICSYNTH_STATIC)
    configure_file(basicsynth-static.pc.in ${PROJECT_BINARY_DIR}/basicsynth-static.pc @ONLY)
    install( FILES ${CMAKE_CURRENT_BINARY_DIR}/basicsynth-static.pc 
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig )
endif()
if (BUILD_BASICSYNTH_SHARED)
    configure_file(basicsynth.pc.in ${PROJECT_BINARY_DIR}/basicsynth.pc @ONLY)
    install( FILES ${CMAKE_CURRENT_BINARY_DIR}/basicsynth.pc 
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig )
endif()

include( CMakePackageConfigHelpers )

write_basic_package_version_file(
    ${PROJECT_NAME}-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${PROJECT_NAME}-config.cmake.in
    ${PROJECT_NAME}-config.cmake
    INSTALL_DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

install( FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
