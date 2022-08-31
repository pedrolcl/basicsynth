add_executable(Example9b main.cpp MIDINames.cpp)
target_link_libraries(Example9b PRIVATE $<IF:$<BOOL:${BUILD_BASICSYNTH_SHARED}>,basicsynth,basicsynth-static> ALSA::ALSA)
