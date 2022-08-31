add_executable(Example02b main.cpp)
target_link_libraries(Example02b PRIVATE $<IF:$<BOOL:${BUILD_BASICSYNTH_SHARED}>,basicsynth,basicsynth-static>)
