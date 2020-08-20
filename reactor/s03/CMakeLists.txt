
aux_source_directory(./ SRC)
aux_source_directory(../../thread/ THREAD_SRC)
aux_source_directory(../../datatime/ DATATIME_SRC)

add_executable(test4 ${SRC} ${THREAD_SRC} ${DATATIME_SRC})

find_package(Threads)
target_link_libraries(test4 ${CMAKE_THREAD_LIBS_INIT})