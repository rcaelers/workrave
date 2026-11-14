add_library(workrave-libs-ui-prefwidgets-qtquick STATIC)

set_target_properties(workrave-libs-ui-prefwidgets-qtquick PROPERTIES AUTOMOC ON AUTORCC ON)

target_sources(workrave-libs-ui-prefwidgets-qtquick PRIVATE
  src/Builder.cc
  src/Models.cc
  src/Formatting.cc
  include/ui/prefwidgets/qtquick/Models.hh
  prefwidgets_quick.qrc)

target_include_directories(workrave-libs-ui-prefwidgets-qtquick PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(workrave-libs-ui-prefwidgets-qtquick PUBLIC
  workrave-libs-ui-prefwidgets
  Qt${QT_VERSION_MAJOR}::Quick
  Qt${QT_VERSION_MAJOR}::QuickControls2)

if (WITH_TESTS)
  add_executable(prefwidgets-qtquick-test tests/BuilderTest.cc)
  target_link_libraries(prefwidgets-qtquick-test PRIVATE workrave-libs-ui-prefwidgets-qtquick)
  add_test(NAME prefwidgets-qtquick COMMAND prefwidgets-qtquick-test)
  set_tests_properties(prefwidgets-qtquick PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")
endif()
