if (HAVE_UNFOLD_AUTO_UPDATE)

  if (PLATFORM_OS_WINDOWS)
    FetchContent_Declare(
        unfold
        GIT_REPOSITORY https://github.com/rcaelers/unfold.git
        GIT_TAG 92047aeaaa73c8ee470828a08a2c71b62e43eb4a
      )
    FetchContent_MakeAvailable(unfold)

    FetchContent_Declare(
      cmark
      GIT_REPOSITORY https://github.com/commonmark/cmark.git
      GIT_TAG        0.30.2
      GIT_SHALLOW    TRUE
      CMAKE_ARGS CMARK_TESTS=OFF
      )
    set(CMARK_TESTS OFF CACHE INTERNAL "Disable tests")
    FetchContent_MakeAvailable(cmark)

    FetchContent_Declare(
      webview2
      URL  https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/1.0.1072.54
      URL_HASH SHA1=aa8ae9db5015a9184011bb195efc5c8caa58a86b
    )
    FetchContent_MakeAvailable(webview2)

    add_library(webview2 INTERFACE)
    target_include_directories(webview2 INTERFACE "${webview2_SOURCE_DIR}/build/native/include/")
    if (NOT MSVC)
      target_compile_options(webview2 INTERFACE -Wno-unknown-pragmas)
    endif()

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64|AMD64")
      target_link_directories(webview2 INTERFACE ${webview2_SOURCE_DIR}/build/native/x64)
      install(FILES ${webview2_SOURCE_DIR}/runtimes/win-x64/native/WebView2Loader.dll DESTINATION bin)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86|X86")
      target_link_directories(webview2 INTERFACE ${webview2_SOURCE_DIR}/build/native/x86)
      install(FILES ${webview2_SOURCE_DIR}/runtimes/x86/native/WebView2Loader.dll DESTINATION bin)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|ARM64")
      target_link_directories(webview2 INTERFACE ${webview2_SOURCE_DIR}/build/native/arm64)
      install(FILES ${webview2_SOURCE_DIR}/runtimes/arm64/native/WebView2Loader.dll DESTINATION bin)
    else()
      message(FATAL_ERROR "Unsupported processor: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()
    if (MSVC)
      target_link_libraries(webview2 INTERFACE WebView2LoaderStatic)
    else()
      target_link_libraries(webview2 INTERFACE WebView2Loader)
    endif()

  endif()
endif()
