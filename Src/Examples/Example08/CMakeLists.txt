add_executable(Example08 main.cpp)
target_link_libraries(Example08 PRIVATE $<IF:$<BOOL:${BUILD_BASICSYNTH_SHARED}>,basicsynth,basicsynth-static>)
