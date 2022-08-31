add_executable(Example07b main.cpp)
target_link_libraries(Example07b PRIVATE $<IF:$<BOOL:${BUILD_BASICSYNTH_SHARED}>,basicsynth,basicsynth-static>)
