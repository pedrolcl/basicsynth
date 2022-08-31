add_executable(Example06 main.cpp)
target_link_libraries(Example06 PRIVATE $<IF:$<BOOL:${BUILD_BASICSYNTH_SHARED}>,basicsynth,basicsynth-static>)
