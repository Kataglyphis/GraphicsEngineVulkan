function(enable_cppcheck project_name)
        option(ENABLE_CPPCHECK "Enable cppcheck" ON)#
        if(ENABLE_CPPCHECK)
                find_program(CPPCHECK cppcheck)
                if(CPPCHECK)
                        set(cppcheck
                        cppcheck
                        "--enable=warning"
                        "--inconclusive"
                        "--force" 
                        "--inline-suppr"
                        )
                        set_target_properties(${project_name} PROPERTIES CXX_CPPCHECK ${cppcheck})              
                # set(CMAKE_CXX_CPPCHECK ${CPPCHECK}  --suppress=missingInclude
                #                                         --enable=all
                # )
                
                endif(CPPCHECK) 
        endif()
endfunction()

function(enable_clangtidy project_name)
        option(ENABLE_CLANGTIDY "Enable clangtidy" ON)#
        if(ENABLE_CLANGTIDY)
                find_program(CLANGTIDY clang-tidy)
                if(CLANGTIDY)
                        set_target_properties(${project_name} PROPERTIES CXX_CLANG_TIDY clang-tidy)
                        #set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY})
                endif(CLANGTIDY)
        endif()
endfunction()