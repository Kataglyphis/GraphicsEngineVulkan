function(enable_sanitizers project_name)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" OFF)

        if(ENABLE_COVERAGE)
            target_compile_options(${project_name} INTERFACE --coverage -g)
            target_link_libraries(${project_name} INTERFACE --coverage)
        endif()

        option(ENABLE_SANITIZER_ADDRESS "Enable address sanitizer for gcc/clang" OFF)
        set(SANITIZERS "")
        if(ENABLE_SANITIZER_ADDRESS)
            list(APPEND SANITIZERS "address")
        endif()

        option(ENABLE_SANITIZER_UNDEFINED_BEHAVIOUR "Enable undefined behaviour sanitizer for gcc/clang" OFF)
        if(ENABLE_SANITIZER_UNDEFINED_BEHAVIOUR)
            list(APPEND SANITIZERS "undefined")
        endif()

        option(ENABLE_SANITIZER_THREAD "Enable thread sanitizer for gcc/clang" OFF)
        if(ENABLE_SANITIZER_THREAD)
            list(APPEND SANITIZERS "thread")
        endif()

        list(JOIN SANITIZERS "," LIST_OF_SANITIZERS)
    endif()

    if(LIST_OF_SANITIZERS)
        if(NOT "${LIST_OF_SANITIZERS}" STREQUAL "")
            target_compile_options(${project_name} INTERFACE -fsanitize=${LIST_OF_SANITIZERS})
            target_link_libraries(${project_name} INTERFACE -fsanitize=${LIST_OF_SANITIZERS})
        endif()
    endif()

endfunction()