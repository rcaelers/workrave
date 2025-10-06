# TODO: Harpoon does not work when build with Clang
#add_subdirectory(harpoon)
#add_subdirectory(harpoonHelper)

if (WIN32)
  find_program(MSBUILD_EXECUTABLE
      NAMES msbuild.exe
      PATHS
      "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin"
      "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Professional/MSBuild/Current/Bin"
      "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Enterprise/MSBuild/Current/Bin"
      "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin"
      "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/2022/Professional/MSBuild/Current/Bin"
      "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/2022/Enterprise/MSBuild/Current/Bin"
      NO_DEFAULT_PATH
  )

  if (NOT MSBUILD_EXECUTABLE)
      message(FATAL_ERROR "msbuild.exe not found. Please ensure Visual Studio 2022 is installed.")
  endif()

  set(HARPOON_SOLUTION_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  set(HARPOON_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/harpoon/_build")
  set(HARPOON_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/harpoon/_output")

  set(configurations Debug Release)
  set(platforms x86 x64 ARM64)

  # Determine Workrave architecture to pass to harpoon build
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64|AMD64")
    set(WORKRAVE_ARCH "X64")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|ARM64")
    set(WORKRAVE_ARCH "ARM64")
  else()
    message(FATAL_ERROR "Unsupported processor: ${CMAKE_SYSTEM_PROCESSOR}")
  endif()

  foreach(configuration IN LISTS configurations)
    foreach(platform IN LISTS platforms)
      set(proj_name "Harpoon_${configuration}_${platform}")

      if("${platform}" STREQUAL "x86")
        set(target_name "harpoon")
      else()
        set(target_name "harpoon64")
      endif()

      set(output_dir "${HARPOON_OUTPUT_DIR}/${configuration}/${platform}")
      set(build_dir "${HARPOON_BUILD_DIR}/${configuration}/${platform}")

      cmake_path(NATIVE_PATH output_dir output_dir_win)
      cmake_path(NATIVE_PATH build_dir build_dir_win)
      cmake_path(NATIVE_PATH HARPOON_SOLUTION_DIR HARPOON_SOLUTION_DIR_WIN)

      ExternalProject_Add(${proj_name}
        SOURCE_DIR        "${HARPOON_SOLUTION_DIR}"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND     "${MSBUILD_EXECUTABLE}" "${HARPOON_SOLUTION_DIR_WIN}\\harpoon.sln"
                          /p:Configuration=${configuration}
                          /p:Platform=${platform}
                          /p:OutDir=${output_dir_win}/
                          /p:DefineConstants=WORKRAVE_ARCH_${WORKRAVE_ARCH}
        INSTALL_COMMAND   ""
        BUILD_BYPRODUCTS  "${output_dir}/${target_name}.lib" "${output_dir}/${target_name}.dll"
      )
    endforeach()
  endforeach()

  if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64|AMD64")
    set(HARPOON_PLATFORM "x64")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|ARM64")
    set(HARPOON_PLATFORM "ARM64")
  else()
    message(FATAL_ERROR "Unsupported processor: ${CMAKE_SYSTEM_PROCESSOR}")
  endif()

  add_library(harpoon64 SHARED IMPORTED GLOBAL)
  set_target_properties(harpoon64 PROPERTIES
    IMPORTED_LOCATION "${HARPOON_OUTPUT_DIR}/Release/${HARPOON_PLATFORM}/harpoon64.dll"
    IMPORTED_IMPLIB "${HARPOON_OUTPUT_DIR}/Release/${HARPOON_PLATFORM}/harpoon64.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${HARPOON_SOLUTION_DIR}/harpoon/include;${HARPOON_SOLUTION_DIR}/harpoonHelper/include"
    PREFIX ""
  )

  if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64|AMD64")
    add_dependencies(harpoon64 Harpoon_Release_x64)
    install(FILES "${HARPOON_OUTPUT_DIR}/Release/x86/harpoonHelper.exe" DESTINATION ${BINDIR32} RENAME WorkraveHelper.exe)
    install(FILES "${HARPOON_OUTPUT_DIR}/Release/x86/harpoon.dll" DESTINATION ${BINDIR32})
    install(FILES "${HARPOON_OUTPUT_DIR}/Release/x64/harpoon64.dll" DESTINATION ${BINDIR})
    install(FILES "${HARPOON_OUTPUT_DIR}/Release/ARM64/harpoonHelper.exe" DESTINATION binarm RENAME WorkraveHelper.exe)
    install(FILES "${HARPOON_OUTPUT_DIR}/Release/ARM64/harpoon64.dll" DESTINATION binarm)

  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|ARM64")
    add_dependencies(harpoon64 Harpoon_Release_ARM64)
    # TODO: build harpoonHelper for x64 locking iso x86
    install(FILES "${HARPOON_OUTPUT_DIR}/Release/ARM64/harpoon64.dll" DESTINATION ${BINDIR})
    install(FILES "${HARPOON_OUTPUT_DIR}/Release/x64/harpoonHelper.exe" DESTINATION binx64 RENAME WorkraveHelper.exe)
    install(FILES "${HARPOON_OUTPUT_DIR}/Release/x64/harpoon64.dll" DESTINATION binx64)
  endif()

endif()
