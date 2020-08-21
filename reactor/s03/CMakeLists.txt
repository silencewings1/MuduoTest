
file(GLOB SRC "*.cpp")
file(GLOB IgnoreFiles "test[0-9]*.cpp")
list(REMOVE_ITEM SRC ${IgnoreFiles})
aux_source_directory(../../thread/ THREAD_SRC)
aux_source_directory(../../datatime/ DATATIME_SRC)

add_executable(test5 test5.cpp ${SRC} ${THREAD_SRC} ${DATATIME_SRC})
add_executable(test6 test6.cpp ${SRC} ${THREAD_SRC} ${DATATIME_SRC})

find_package(Threads)
target_link_libraries(test5 ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(test6 ${CMAKE_THREAD_LIBS_INIT})
