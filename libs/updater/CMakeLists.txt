if (HAVE_UNFOLD_AUTO_UPDATE)

  if (PLATFORM_OS_WINDOWS)
    FetchContent_Declare_Logged(
      unfold
      "Auto-update framework"
      "MIT"
      GIT_REPOSITORY https://github.com/rcaelers/unfold.git
      GIT_TAG abc833f80b39e256c4a6ab88917ba0a223d0a1fd
    )
    if(NOT unfold_POPULATED)
      FetchContent_MakeAvailable(unfold)
      install(TARGETS unfold RUNTIME DESTINATION bin)
    endif()

    FetchContent_Declare_Logged(
      cmark
      "CommonMark parsing and rendering library"
      "BSD-2-Clause/MIT"
      GIT_REPOSITORY https://github.com/commonmark/cmark.git
      GIT_TAG        0.31.1
      GIT_SHALLOW    TRUE
      CMAKE_ARGS "BUILD_TESTING=OFF BUILD_STATIC_LIBS=OFF BUILD_SHARED_LIBS=ON"
    )
    set(BUILD_TESTING OFF CACHE INTERNAL "Disable tests")
    if(NOT cmark_POPULATED)
      FetchContent_MakeAvailable(cmark)
      install(TARGETS unfold RUNTIME DESTINATION bin)
    endif()

    FetchContent_Declare_Logged(
      webview2
      "Microsoft Edge WebView2 embedded web browser control"
      "Microsoft-Web-WebView2"
      URL https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/1.0.2651.64
      URL_HASH SHA1=e08093bd127686535cf3519011a584664e6d2b75
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

  add_subdirectory(src)
endif()
