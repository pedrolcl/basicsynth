add_executable(Example01 main.cpp)
target_link_libraries(Example01 PRIVATE $<IF:$<BOOL:${BUILD_BASICSYNTH_SHARED}>,basicsynth,basicsynth-static>)
