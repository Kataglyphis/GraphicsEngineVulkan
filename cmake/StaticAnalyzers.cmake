option(ENABLE_CPPCHECK "Enable cppcheck" ON)
if(ENABLE_CPPCHECK)
        find_program(CPPCHECK cppcheck)
        if(CPPCHECK)
        set     (CMAKE_CXX_CPPCHECK     ${CPPCHECK}  
                                        --enable=all
                )                                   
        endif(CPPCHECK)
endif()

option(ENABLE_CLANGTIDY "Enable clangtidy" ON) #OFF
if(ENABLE_CLANGTIDY)
        find_program(CLANGTIDY clang-tidy)
        if(CLANGTIDY)
            set(CMAKE_CXX_CLANG_TIDY    ${CLANGTIDY} 
                                        --config=""
                                        --config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy                                      
                )
        endif(CLANGTIDY)
endif()