if (HAVE_UNFOLD_AUTO_UPDATE)

  if (PLATFORM_OS_WINDOWS)
    FetchContent_Declare_Logged(
      unfold
      "Auto-update framework"
      "MIT"
      GIT_REPOSITORY https://github.com/rcaelers/unfold.git
      GIT_TAG 14abf214d640cf8e45c385ba6ce5b4f9866993d9
    )
    if(NOT unfold_POPULATED)
      FetchContent_MakeAvailable(unfold)
      install(TARGETS unfold RUNTIME DESTINATION bin)
    endif()

    # Prefer system/MSYS2 cmark (CMake package); fallback to FetchContent.
    set(WORKRAVE_CMARK_TARGET "")
    find_package(cmark CONFIG QUIET GLOBAL)
    if (TARGET cmark::cmark)
      set(WORKRAVE_CMARK_TARGET cmark::cmark)
    elseif (TARGET cmark)
      set(WORKRAVE_CMARK_TARGET cmark)
    endif()

    if (WORKRAVE_CMARK_TARGET)
      message(STATUS "Using system cmark")
      install(IMPORTED_RUNTIME_ARTIFACTS ${WORKRAVE_CMARK_TARGET} RUNTIME DESTINATION bin)
    else()
      message(STATUS "System cmark not found; using FetchContent")
      FetchContent_Declare_Logged(
        cmark
        "CommonMark parsing and rendering library"
        "BSD-2-Clause/MIT"
        GIT_REPOSITORY https://github.com/commonmark/cmark.git
        GIT_TAG        0.31.2
        GIT_SHALLOW    TRUE
        CMAKE_ARGS "BUILD_TESTING=OFF BUILD_STATIC_LIBS=OFF BUILD_SHARED_LIBS=ON"
      )
      set(BUILD_TESTING OFF CACHE INTERNAL "Disable tests")
      if(NOT cmark_POPULATED)
        FetchContent_MakeAvailable(cmark)
        install(TARGETS cmark RUNTIME DESTINATION bin)
      endif()
      set(WORKRAVE_CMARK_TARGET cmark)
    endif()
    set(WORKRAVE_CMARK_TARGET "${WORKRAVE_CMARK_TARGET}" CACHE INTERNAL "cmark library target")

    # Prefer system/MSYS2 WebView2 (headers + loader lib); fallback to NuGet package.
    find_path(WORKRAVE_WEBVIEW2_INCLUDE_DIR NAMES WebView2.h)
    find_library(WORKRAVE_WEBVIEW2_LOADER_LIBRARY NAMES WebView2Loader)
    if(NOT WORKRAVE_WEBVIEW2_LOADER_LIBRARY)
      # MSYS2 ships only the DLL, no import lib; MinGW ld/lld can link against the DLL directly.
      find_file(WORKRAVE_WEBVIEW2_LOADER_LIBRARY NAMES WebView2Loader.dll PATH_SUFFIXES bin)
    endif()

    add_library(webview2 INTERFACE)
    if (NOT MSVC)
      target_compile_options(webview2 INTERFACE -Wno-unknown-pragmas)
    endif()

    if (WORKRAVE_WEBVIEW2_INCLUDE_DIR AND WORKRAVE_WEBVIEW2_LOADER_LIBRARY)
      message(STATUS "Using system WebView2")
      target_include_directories(webview2 INTERFACE "${WORKRAVE_WEBVIEW2_INCLUDE_DIR}")
      target_link_libraries(webview2 INTERFACE "${WORKRAVE_WEBVIEW2_LOADER_LIBRARY}")
      if(WORKRAVE_WEBVIEW2_LOADER_LIBRARY MATCHES "\\.dll$")
        install(FILES "${WORKRAVE_WEBVIEW2_LOADER_LIBRARY}" DESTINATION bin)
      endif()
    else()
      message(STATUS "System WebView2 not found; using NuGet WebView2 package")
      FetchContent_Declare_Logged(
        webview2
        "Microsoft Edge WebView2 embedded web browser control"
        "Microsoft-Web-WebView2"
        URL https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/1.0.3856.49
        URL_HASH SHA1=688937fd04b7d1a1d69fcbd914063895fa26d38c
      )
      FetchContent_MakeAvailable(webview2)

      target_include_directories(webview2 INTERFACE "${webview2_SOURCE_DIR}/build/native/include/")

      if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64|AMD64")
        target_link_directories(webview2 INTERFACE ${webview2_SOURCE_DIR}/build/native/x64)
        install(FILES ${webview2_SOURCE_DIR}/runtimes/win-x64/native/WebView2Loader.dll DESTINATION bin)
      elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86|X86")
        target_link_directories(webview2 INTERFACE ${webview2_SOURCE_DIR}/build/native/x86)
        install(FILES ${webview2_SOURCE_DIR}/runtimes/win-x86/native/WebView2Loader.dll DESTINATION bin)
      elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|ARM64")
        target_link_directories(webview2 INTERFACE ${webview2_SOURCE_DIR}/build/native/arm64)
        install(FILES ${webview2_SOURCE_DIR}/runtimes/win-arm64/native/WebView2Loader.dll DESTINATION bin)
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

  add_subdirectory(src)
endif()
