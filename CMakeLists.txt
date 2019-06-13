cmake_minimum_required(VERSION 3.8.2)
set (CMAKE_CXX_STANDARD 11)

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Turn on automatic invocation of the MOC, UIC & RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# There may be a way to tell up front if Qt5 is going to be found, but I haven't found
# a foolproof way to do it yet, so settle for the default error message for now.
#if(NOT CMAKE_PREFIX_PATH AND NOT Qt5Widgets_DIR)
#    message(WARNING "CMAKE_PREFIX_PATH is not defined, so find_package may not work. Set the CMAKE_PREFIX_PATH "
#            "environment variable to the install prefix of Qt 5, either on the command line as "
#            "-DCMAKE_PREFIX_PATH=\"path/to/Qt5/lib/cmake\" or with set(CMAKE_PREFIX_PATH path/to/Qt5/lib/cmake)")
#endif(NOT CMAKE_PREFIX_PATH AND NOT Qt5Widgets_DIR)

# Add a compiler flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Make this a GUI application on Windows
if(WIN32)
  set(CMAKE_WIN32_EXECUTABLE ON)
endif()

# Find the QtWidgets library
execute_process(COMMAND brew --prefix qt5 OUTPUT_VARIABLE Qt5_DIR)
string(STRIP ${Qt5_DIR} Qt5_DIR)
set (Qt5_DIR "${Qt5_DIR}/lib/cmake/Qt5")
message(${Qt5_DIR})
find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(UISRCs
  mainwindow.cpp mainwindow.ui 
)

set(OtherSrcs
  RoverCommunicator.cpp WindowWorker.cpp
)

add_subdirectory(Serial)

# Tell CMake to create the helloworld executable
add_executable(Houston main.cpp ${UISRCs} ${OtherSrcs})

add_dependencies(Houston Serial)
include_directories(${CMAKE_SOURCE_DIR}/Serial)
target_link_libraries(Houston Serial)

# Add the Qt5 Widgets for linking
target_link_libraries(Houston Qt5::Widgets)
