add_executable(Example07 main.cpp)
target_link_libraries(Example07 PRIVATE $<IF:$<BOOL:${BUILD_BASICSYNTH_SHARED}>,basicsynth,basicsynth-static>)
