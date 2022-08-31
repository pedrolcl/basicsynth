add_executable(Example09 main.cpp)
target_link_libraries(Example09 PRIVATE $<IF:$<BOOL:${BUILD_BASICSYNTH_SHARED}>,basicsynth,basicsynth-static>)
