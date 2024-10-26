function(enable_sanitizers project_name)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT MSVC)
        
        option(ENABLE_SANITIZER_ADDRESS "Enable address sanitizer for gcc/clang" OFF)#
        set(SANITIZERS "")
        if(ENABLE_SANITIZER_ADDRESS)
            list(APPEND SANITIZERS "address")
        endif()

        option(ENABLE_SANITIZER_UNDEFINED_BEHAVIOUR "Enable undefined behaviour sanitizer for gcc/clang" OFF)#OFF
        if(ENABLE_SANITIZER_UNDEFINED_BEHAVIOUR)
            list(APPEND SANITIZERS "undefined")
        endif()

        option(ENABLE_SANITIZER_THREAD "Enable thread sanitizer for gcc/clang" OFF)#OFF
        if(ENABLE_SANITIZER_THREAD)
            list(APPEND SANITIZERS "thread")
        endif()

        option(ENABLE_SANITIZER_MEMORY "Enable memory sanitizer for gcc/clang" OFF)#OFF
        if(ENABLE_SANITIZER_MEMORY)
            list(APPEND SANITIZERS "memory")
        endif()

        list(JOIN SANITIZERS "," LIST_OF_SANITIZERS)
        
    elseif(MSVC)

        option(ENABLE_SANITIZER_ADDRESS_MSVC "Enable address sanitizer for msvc" OFF)#
        if(ENABLE_SANITIZER_ADDRESS_MSVC)
            target_compile_options(${project_name} INTERFACE /fsanitize=address)
        endif()   
    endif()

    if(LIST_OF_SANITIZERS)
        if(NOT "${LIST_OF_SANITIZERS}" STREQUAL "")
            target_compile_options(${project_name} INTERFACE -fsanitize=${LIST_OF_SANITIZERS})
            target_link_libraries(${project_name} INTERFACE -fsanitize=${LIST_OF_SANITIZERS})
        endif()
    endif()

endfunction()