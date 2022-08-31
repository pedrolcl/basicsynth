add_executable(Example02a main.cpp)
target_link_libraries(Example02a PRIVATE $<IF:$<BOOL:${BUILD_BASICSYNTH_SHARED}>,basicsynth,basicsynth-static>)
