add_executable(Example9a main.cpp MIDINames.cpp)
target_link_libraries(Example9a PRIVATE $<IF:$<BOOL:${BUILD_BASICSYNTH_SHARED}>,basicsynth,basicsynth-static>)
