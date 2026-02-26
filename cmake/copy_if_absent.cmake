if(NOT EXISTS "${DST}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${SRC}" "${DST}")
endif()
