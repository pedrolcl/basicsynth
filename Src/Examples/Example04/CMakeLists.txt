add_executable(Example04 main.cpp)
target_link_libraries(Example04 PRIVATE $<IF:$<BOOL:${BUILD_BASICSYNTH_SHARED}>,basicsynth,basicsynth-static>)
