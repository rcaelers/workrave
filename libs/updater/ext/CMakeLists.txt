if (APPLE)

add_custom_target(Sparkle ALL 
                  xcodebuild -project ${CMAKE_SOURCE_DIR}/libs/updater/ext/sparkle/Sparkle.xcodeproj
                             SYMROOT=${CMAKE_CURRENT_BINARY_DIR}/sparkle build
                             INSTALL_PATH=${CMAKE_CURRENT_BINARY_DIR}/sparkle/Release
                   COMMENT building Sparkle)

install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/sparkle/Release/Sparkle.framework" DESTINATION "Workrave.app/Contents/Frameworks")

endif()

