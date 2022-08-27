add_subdirectory(Common)
add_subdirectory(Instruments)

if (BUILD_BASICSYNTH_SHARED)
    add_library(basicsynth SHARED)
    target_link_libraries(basicsynth PRIVATE common instruments)
    target_compile_definitions(basicsynth PUBLIC ${PROJECT_COMMON_FLAGS})
    set_target_properties(basicsynth PROPERTIES 
        PUBLIC_HEADER "${HEADERS}"
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR})
    target_include_directories(basicsynth PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Include;${CMAKE_CURRENT_SOURCE_DIR}/Instruments>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/basicsynth>")
    add_library( basicsynth::basicsynth ALIAS basicsynth)
    list( APPEND BASICSYNTH_TARGETS basicsynth )
endif()

if (BUILD_BASICSYNTH_STATIC)
    add_library(basicsynth-static STATIC $<TARGET_OBJECTS:common> $<TARGET_OBJECTS:instruments>)
    target_compile_definitions(basicsynth-static PUBLIC ${PROJECT_COMMON_FLAGS})
    target_include_directories(basicsynth-static PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Include;${CMAKE_CURRENT_SOURCE_DIR}/Instruments>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/basicsynth>")
    set_target_properties( basicsynth-static PROPERTIES PUBLIC_HEADER "${HEADERS}")
    add_library( basicsynth::basicsynth-static ALIAS basicsynth-static)
    list( APPEND BASICSYNTH_TARGETS basicsynth-static )
endif()

if (BUILD_EXAMPLES)
    add_subdirectory(Examples)
endif()

install( TARGETS ${BASICSYNTH_TARGETS}
    EXPORT basicsynth-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} 
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/basicsynth
)

install( EXPORT basicsynth-targets
    FILE ${PROJECT_NAME}-targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

export( EXPORT basicsynth-targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-targets.cmake"
    NAMESPACE ${PROJECT_NAME}::
)

