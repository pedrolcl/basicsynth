add_executable(Example03 main.cpp)
target_link_libraries(Example03 PRIVATE $<IF:$<BOOL:${BUILD_BASICSYNTH_SHARED}>,basicsynth,basicsynth-static>)
