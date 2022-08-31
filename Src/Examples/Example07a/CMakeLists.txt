add_executable(Example07a main.cpp)
target_link_libraries(Example07a PRIVATE $<IF:$<BOOL:${BUILD_BASICSYNTH_SHARED}>,basicsynth,basicsynth-static>)
