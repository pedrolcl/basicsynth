set ( COMMON_SOURCES
    DLSFile.cpp
    Global.cpp
    InstrManager.cpp
    MIDIControl.cpp
    MIDIInput.cpp
    MIDISequencer.cpp
    Player.cpp
    SequenceFile.cpp
    Sequencer.cpp
    SFFile.cpp
    SMFFile.cpp
    SoundBank.cpp
    SynthMutex.cpp
    SynthString.cpp
    SynthThread.cpp
    WaveFile.cpp
)

set( HEADERS
    ${PROJECT_SOURCE_DIR}/Include/AllPass.h
    ${PROJECT_SOURCE_DIR}/Include/BasicSynth.h
    ${PROJECT_SOURCE_DIR}/Include/BiQuad.h
    ${PROJECT_SOURCE_DIR}/Include/DelayLine.h
    ${PROJECT_SOURCE_DIR}/Include/DLSDefs.h
    ${PROJECT_SOURCE_DIR}/Include/DLSFile.h
    ${PROJECT_SOURCE_DIR}/Include/Docpages.h
    ${PROJECT_SOURCE_DIR}/Include/dsoundintf.h
    ${PROJECT_SOURCE_DIR}/Include/DynFilter.h
    ${PROJECT_SOURCE_DIR}/Include/EnvGen.h
    ${PROJECT_SOURCE_DIR}/Include/EnvGenSeg.h
    ${PROJECT_SOURCE_DIR}/Include/Filter.h
    ${PROJECT_SOURCE_DIR}/Include/Flanger.h
    ${PROJECT_SOURCE_DIR}/Include/GenNoise.h
    ${PROJECT_SOURCE_DIR}/Include/GenWave64.h
    ${PROJECT_SOURCE_DIR}/Include/GenWaveDSF.h
    ${PROJECT_SOURCE_DIR}/Include/GenWave.h
    ${PROJECT_SOURCE_DIR}/Include/GenWaveWT.h
    ${PROJECT_SOURCE_DIR}/Include/GenWaveX.h
    ${PROJECT_SOURCE_DIR}/Include/Instrument.h
    ${PROJECT_SOURCE_DIR}/Include/MIDIControl.h
    ${PROJECT_SOURCE_DIR}/Include/MIDIDefs.h
    ${PROJECT_SOURCE_DIR}/Include/MIDIInput.h
    ${PROJECT_SOURCE_DIR}/Include/MIDISequencer.h
    ${PROJECT_SOURCE_DIR}/Include/Mixer.h
    ${PROJECT_SOURCE_DIR}/Include/Player.h
    ${PROJECT_SOURCE_DIR}/Include/Reverb.h
    ${PROJECT_SOURCE_DIR}/Include/SeqEvent.h
    ${PROJECT_SOURCE_DIR}/Include/SequenceFile.h
    ${PROJECT_SOURCE_DIR}/Include/Sequencer.h
    ${PROJECT_SOURCE_DIR}/Include/SFDefs.h
    ${PROJECT_SOURCE_DIR}/Include/SFFile.h
    ${PROJECT_SOURCE_DIR}/Include/SFGen.h
    ${PROJECT_SOURCE_DIR}/Include/SMFFile.h
    ${PROJECT_SOURCE_DIR}/Include/SoundBank.h
    ${PROJECT_SOURCE_DIR}/Include/SynthDefs.h
    ${PROJECT_SOURCE_DIR}/Include/SynthFile.h
    ${PROJECT_SOURCE_DIR}/Include/SynthList.h
    ${PROJECT_SOURCE_DIR}/Include/SynthMutex.h
    ${PROJECT_SOURCE_DIR}/Include/SynthString.h
    ${PROJECT_SOURCE_DIR}/Include/SynthThread.h
    ${PROJECT_SOURCE_DIR}/Include/tinystr.h
    ${PROJECT_SOURCE_DIR}/Include/tinyxml.h
    ${PROJECT_SOURCE_DIR}/Include/WaveFile.h
    ${PROJECT_SOURCE_DIR}/Include/WaveOutALSA.h
    ${PROJECT_SOURCE_DIR}/Include/WaveOutDirect.h
    ${PROJECT_SOURCE_DIR}/Include/WaveTable.h
    ${PROJECT_SOURCE_DIR}/Include/XmlWrap.h
    PARENT_SCOPE
)

if (UNIX)
    list ( APPEND COMMON_SOURCES SynthFileU.cpp )
endif()

if (WIN32)
    list ( APPEND COMMON_SOURCES SynthFileW.cpp WaveOutDirect.cpp )
endif()

set ( TINYXML_SOURCES
    tinyxml/tinystr.cpp
    tinyxml/tinyxml.cpp
    tinyxml/tinyxmlerror.cpp
    tinyxml/tinyxmlparser.cpp
)

if (USE_LIBXML)
    list ( APPEND COMMON_SOURCES XmlWrapU.cpp )
elseif (USE_TINYXML)
    list ( APPEND COMMON_SOURCES ${TINYXML_SOURCES} XmlWrapT.cpp )
elseif (WIN32)
    list ( APPEND COMMON_SOURCES XmlWrapW.cpp )
else()
    list ( APPEND COMMON_SOURCES XmlWrapN.cpp )
endif()

add_library(common OBJECT ${COMMON_SOURCES})

set_target_properties(common PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_compile_options(common PUBLIC ${PROJECT_COMMON_FLAGS})
target_include_directories(common PUBLIC ${PROJECT_SOURCE_DIR}/Include )

if (USE_LIBXML)
    target_link_libraries(common PUBLIC LibXml2::LibXml2)
endif()    

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_link_libraries(common PUBLIC ALSA::ALSA Threads::Threads )
endif()

if (WIN32)
    target_link_libraries(common PUBLIC winmm)
endif()

if (UNIX AND NOT APPLE)
    target_link_libraries(common PUBLIC ${MATH_LIBRARY})
endif()
