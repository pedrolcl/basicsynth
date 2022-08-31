add_executable(Example02 main.cpp)
target_link_libraries(Example02 PRIVATE $<IF:$<BOOL:${BUILD_BASICSYNTH_SHARED}>,basicsynth,basicsynth-static>)
