add_library(workrave-libs-ui-prefwidgets STATIC)

target_sources(workrave-libs-ui-prefwidgets PRIVATE
    Box.cc
    Choice.cc
    Entry.cc
    Frame.cc
    Time.cc
    Toggle.cc
    Value.cc
    Widget.cc
    Section.cc
    SizeGroup.cc
    )

target_link_libraries(workrave-libs-ui-prefwidgets
  PUBLIC
  workrave-libs-config
  workrave-libs-utils
  workrave-libs-commonui)

target_include_directories(
    workrave-libs-ui-prefwidgets
    INTERFACE
    ${CMAKE_SOURCE_DIR}/ui/libs/prefwidgets/include
    PRIVATE
    ${CMAKE_SOURCE_DIR}/ui/libs/prefwidgets/include/ui/prefwidgets
)
