
file(GLOB SRC "*.cpp")
file(GLOB IgnoreFiles "test[0-9]*.cpp")
list(REMOVE_ITEM SRC ${IgnoreFiles})
aux_source_directory(../../thread/ THREAD_SRC)

add_executable(test1 test1.cpp ${SRC} ${THREAD_SRC})
# add_executable(test2 test2.cpp ${SRC} ${THREAD_SRC})

find_package(Threads)
target_link_libraries(test1 ${CMAKE_THREAD_LIBS_INIT})
# target_link_libraries(test2 ${CMAKE_THREAD_LIBS_INIT})