add_executable(Example9b main.cpp MIDINames.cpp)
target_link_libraries(Example9b PRIVATE basicsynth ALSA::ALSA)
