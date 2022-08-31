add_executable(Example05 main.cpp)
target_link_libraries(Example05 PRIVATE $<IF:$<BOOL:${BUILD_BASICSYNTH_SHARED}>,basicsynth,basicsynth-static>)
