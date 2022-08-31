add_executable(Example07c main.cpp)
target_link_libraries(Example07c PRIVATE $<IF:$<BOOL:${BUILD_BASICSYNTH_SHARED}>,basicsynth,basicsynth-static>)
