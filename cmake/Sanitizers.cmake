function(metadrop_enable_sanitizers target)
    if(NOT METADROP_ENABLE_SANITIZERS OR MSVC)
        return()
    endif()

    target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
endfunction()
