add_executable(Example10 main.cpp)
target_link_libraries(Example10 PRIVATE $<IF:$<BOOL:${BUILD_BASICSYNTH_SHARED}>,basicsynth,basicsynth-static>)
