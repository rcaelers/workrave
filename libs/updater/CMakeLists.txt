if (HAVE_UNFOLD_AUTO_UPDATE)

  if (PLATFORM_OS_WINDOWS)
    FetchContent_Declare(
        unfold
        GIT_REPOSITORY https://github.com/rcaelers/unfold.git
        GIT_TAG c709f2d08e384b4506b67acd3391e157640b3dee
      )
    if(NOT unfold_POPULATED)
      FetchContent_Populate(unfold)
      add_subdirectory(${unfold_SOURCE_DIR} ${unfold_BINARY_DIR} EXCLUDE_FROM_ALL)
      install(TARGETS unfold RUNTIME DESTINATION bin)
    endif()

    FetchContent_Declare(
      cmark
      GIT_REPOSITORY https://github.com/commonmark/cmark.git
      GIT_TAG        0.30.2
      GIT_SHALLOW    TRUE
      CMAKE_ARGS CMARK_TESTS=OFF
      )
    set(CMARK_TESTS OFF CACHE INTERNAL "Disable tests")
    if(NOT cmark_POPULATED)
      FetchContent_Populate(cmark)
      add_subdirectory(${cmark_SOURCE_DIR} ${cmark_BINARY_DIR} EXCLUDE_FROM_ALL)
      install(TARGETS cmark RUNTIME DESTINATION bin)
    endif()

    FetchContent_Declare(
      webview2
      URL https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/1.0.1418.22
      URL_HASH SHA1=08bf310d4bc9f5ab5a495c47501aa286720c3e36
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
