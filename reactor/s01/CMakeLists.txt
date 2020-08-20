
aux_source_directory(./ SRC)
aux_source_directory(../../thread/ THREAD_SRC)

add_executable(test3 ${SRC} ${THREAD_SRC})

find_package(Threads)
target_link_libraries(test3 ${CMAKE_THREAD_LIBS_INIT})