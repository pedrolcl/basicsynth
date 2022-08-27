set ( SOURCES
    AddSynth.cpp
    BuzzSynth.cpp
    Chuffer.cpp
    FMSynth.cpp
    GMPlayer.cpp
    Includes.cpp
    LFO.cpp
    LoadInstrLib.cpp
    MatrixSynth.cpp
    MixerControl.cpp
    ModSynth.cpp
    PitchBend.cpp
    SFPlayer.cpp
    SubSynth.cpp
    Tone.cpp
    WFSynth.cpp
)

list(APPEND HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/AddSynth.h
    ${CMAKE_CURRENT_SOURCE_DIR}/BuzzSynth.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Chuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/FMSynth.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GMPlayer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Includes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Instruments.h
    ${CMAKE_CURRENT_SOURCE_DIR}/LFO.h
    ${CMAKE_CURRENT_SOURCE_DIR}/LoadInstrLib.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MatrixSynth.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MixerControl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ModSynth.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ModSynthUG.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PitchBend.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SFPlayer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SubSynth.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Tone.h
    ${CMAKE_CURRENT_SOURCE_DIR}/UGCalc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/UGDelay.h
    ${CMAKE_CURRENT_SOURCE_DIR}/UGEnvGen.h
    ${CMAKE_CURRENT_SOURCE_DIR}/UGFilter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/UGOscil.h
    ${CMAKE_CURRENT_SOURCE_DIR}/WFSynth.h
)

set(HEADERS ${HEADERS} PARENT_SCOPE)

add_library(instruments OBJECT ${SOURCES})
target_compile_options(instruments PUBLIC ${PROJECT_COMMON_FLAGS})
target_include_directories(instruments PUBLIC ${PROJECT_SOURCE_DIR}/Include)
set_target_properties(instruments PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
