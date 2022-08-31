add_executable(Example4a main.cpp)
target_link_libraries(Example4a PRIVATE $<IF:$<BOOL:${BUILD_BASICSYNTH_SHARED}>,basicsynth,basicsynth-static>)
