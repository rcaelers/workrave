if (HAVE_UNFOLD_AUTO_UPDATE)

  if (PLATFORM_OS_WINDOWS)
    FetchContent_Declare_Logged(
      unfold
      "Auto-update framework"
      "MIT"
      GIT_REPOSITORY https://github.com/rcaelers/unfold.git
      GIT_TAG 6afccbf9cb3bdf0366353781890a0f12e161cb8c
    )
    if(NOT unfold_POPULATED)
      FetchContent_MakeAvailable(unfold)
      install(TARGETS unfold RUNTIME DESTINATION bin)
    endif()

    if(HAVE_SBOM)
      set(_deps_dir "${FETCHCONTENT_BASE_DIR}")
      sbom_add_fetched_package(semver "Semantic Versioning for modern C++" "MIT"
        "${_deps_dir}/semver-src" "https://github.com/Neargye/semver")
      sbom_add_fetched_package(sigstore-verifier-cpp "Sigstore signature verification library" "Apache-2.0"
        "${_deps_dir}/sigstore-src" "https://github.com/rcaelers/sigstore-verifier-cpp")
      sbom_add_fetched_package(sigstore_protos "Sigstore protobuf specifications" "Apache-2.0"
        "${_deps_dir}/sigstore_protos-src" "https://github.com/sigstore/protobuf-specs.git")
      sbom_add_fetched_package(googleapis_protos "Google API protocol buffer definitions" "Apache-2.0"
        "${_deps_dir}/googleapis_protos-src" "https://github.com/googleapis/googleapis.git")
    endif()

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

    FetchContent_Declare_Logged(
      webview2
      "Microsoft Edge WebView2 embedded web browser control"
      "Microsoft-Web-WebView2"
      URL https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/1.0.3856.49
      URL_HASH SHA1=688937fd04b7d1a1d69fcbd914063895fa26d38c
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

  add_subdirectory(src)
endif()
