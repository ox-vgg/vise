##
## VGG Image Search Engine (VISE)
## vise
##
## Author: Abhishek Dutta <adutta _AT_ robots.ox.ac.uk>
## 12 Nov. 2019
##

cmake_minimum_required( VERSION 3.14 )

project(vise)

enable_testing()
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
SET(BUILD_SHARED_LIBS OFF)

#set( BUILD_SHARED_LIBS "ON" )
set( VISE_SOURCE_ROOT_DIR ${CMAKE_SOURCE_DIR}/../ )

set(CTEST_OUTPUT_ON_FAILURE 1)

## export version number in version file
set (VISE_NAME "VISE")
set (VISE_FULLNAME "VGG Image Search Engine")
set (VISE_VERSION_MAJOR "2")
set (VISE_VERSION_MINOR "0")
set (VISE_VERSION_PATCH "0-beta5")
configure_file (
  "${PROJECT_SOURCE_DIR}/vise/vise_version.h.in"
  "${PROJECT_SOURCE_DIR}/vise/vise_version.h"
  )

IF(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
ENDIF()

if (WIN32)
  message("************************************************************")
  message("************ Compiling for Windows Platform ****************")
  message("************************************************************")
  SET(WINDOWS_RESOURCE_FILENAME "${CMAKE_CURRENT_SOURCE_DIR}/res/win/vise.rc")

  SET (VLFEAT_LIB "C:\\Users\\tlm\\dep\\vise\\vlfeat-0.9.21\\bin\\win64\\vl.lib")
  SET (VLFEAT_INCLUDE_DIR "C:\\Users\\tlm\\dep\\vise\\vlfeat-0.9.21")
  SET (BOOST_ROOT "C:\\Users\\tlm\\dep\\vise\\boost_1_70_0")
  SET (BOOST_DIR "C:\\Users\\tlm\\dep\\vise\\boost_1_70_0")
  SET (Eigen3_DIR "C:\\Users\\tlm\\dep\\vise\\eigen-3.3.7\\cmake")
  SET (Protobuf_SRC_ROOT_FOLDER "C:\\Users\\tlm\\dep\\vise\\tmp_libsrc\\protobuf-2.6.1")
  SET (Protobuf_FIND_VERSION "2.6.1")
  SET (Protobuf_PROTOC_EXECUTABLE "C:\\Users\\tlm\\dep\\vise\\tmp_libsrc\\protobuf-2.6.1\\vsprojects\\x64\\Release\\protoc.exe")
  SET (Protobuf_LIBRARIES "C:\\Users\\tlm\\dep\\vise\\tmp_libsrc\\protobuf-2.6.1\\vsprojects\\x64\\Release\\libprotobuf.lib")
  SET (Protobuf_USE_STATIC_LIBS ON)
  SET (Protobuf_DEBUG OFF)

  #SET(CMAKE_CXX_FLAGS "/MT /O2 /Ob2 /DNDEBUG /EHsc")
  add_definitions("/MT /O2 /Ob2 /DNDEBUG /EHsc")
  SET (VISE_DEPS_BASEDIR "C:\\Users\\tlm\\dep\\vise\\dep_dll")
  STRING(REPLACE "\\" "\\\\" VISE_DEPS_BASEDIR_2SLASH ${VISE_DEPS_BASEDIR})
  STRING(REPLACE "\\" "\\\\" VISE_DEPS_BASEDIR_4SLASH ${VISE_DEPS_BASEDIR_2SLASH})

  SET (VISE_MAGICK_DEPS_BASEDIR "C:\\Users\\tlm\\dep\\vise\\dep_dll_imagemagick_from_traherne")
  STRING(REPLACE "\\" "\\\\" VISE_MAGICK_DEPS_BASEDIR_2SLASH ${VISE_MAGICK_DEPS_BASEDIR})
  STRING(REPLACE "\\" "\\\\" VISE_MAGICK_DEPS_BASEDIR_4SLASH ${VISE_MAGICK_DEPS_BASEDIR_2SLASH})

  SET (VISE_DEMO_PROJECTS_DIR "C:\\Users\\tlm\\dep\\vise\\demo_projects")
  STRING(REPLACE "\\" "\\\\" VISE_DEMO_PROJECTS_DIR_2SLASH ${VISE_DEMO_PROJECTS_DIR})
  STRING(REPLACE "\\" "\\\\" VISE_DEMO_PROJECTS_DIR_4SLASH ${VISE_DEMO_PROJECTS_DIR_2SLASH})

  SET (VISE_ASSET_DIR "C:\\Users\\tlm\\dep\\vise\\asset")
  STRING(REPLACE "\\" "\\\\" VISE_ASSET_DIR_2SLASH ${VISE_ASSET_DIR})
  STRING(REPLACE "\\" "\\\\" VISE_ASSET_DIR_4SLASH ${VISE_ASSET_DIR_2SLASH})
endif (WIN32)

if (APPLE)
  #set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  #set(CMAKE_SKIP_RPATH TRUE)
  set(CMAKE_MACOSX_RPATH TRUE)
endif(APPLE)

## Boost
set(Boost_DEBUG 0)
find_package(Boost 1.70.0 COMPONENTS filesystem system thread REQUIRED)
link_directories( ${Boost_LIBRARY_DIRS} )
include_directories( ${Boost_INCLUDE_DIRS} )

## Imagemagick
ADD_DEFINITIONS(-DMAGICKCORE_HDRI_ENABLE=0)
ADD_DEFINITIONS(-DMAGICKCORE_QUANTUM_DEPTH=8)
find_package( ImageMagick COMPONENTS Magick++ REQUIRED )
include_directories( ${ImageMagick_INCLUDE_DIRS} )

## OpenMP
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
include_directories( ${OpenMP_CXX_INCLUDE_DIRS} ) ## required by clang in MacOS 11

## Protocol Buffer
## required by "search_engine/relja_retrival"
include(FindProtobuf)
find_package(Protobuf 2.6.1 REQUIRED )
include_directories( ${PROTOBUF_INCLUDE_DIR} )

## Eigen
## requires EIGEN_DIR to be set to the location where Eigen3 is built using cmake
if (WIN32)
	find_package(Eigen3 CONFIG REQUIRED)
else (WIN32)
  find_package(Eigen3 3.3 REQUIRED NO_MODULE)
endif (WIN32)
include_directories( ${EIGEN3_INCLUDE_DIR} )

## vlfeat
## -DVLFEAT_LIB=/data/mybin/vlfeat/vlfeat-0.9.20/bin/glnxa64/libvl.so -DVLFEAT_INCLUDE_DIR=/data/mybin/vlfeat/vlfeat-0.9.20/vl
# @todo: remove dependency on absolute path
include_directories( ${VLFEAT_INCLUDE_DIR})

## sqlite
find_package(SQLite3)

message("Boost_LIBRARY_DIRS: " ${Boost_LIBRARY_DIRS})
message("Boost_INCLUDE_DIRS: " ${Boost_INCLUDE_DIRS})
message("ImageMagick_LIBRARIES: " ${ImageMagick_LIBRARIES})
message("ImageMagick_INCLUDE_DIRS: " ${ImageMagick_INCLUDE_DIRS})
message("PROTOBUF_INCLUDE_DIR: " ${PROTOBUF_INCLUDE_DIR} )
message("Protobuf_LIBRARIES: " ${Protobuf_LIBRARIES} )
message("EIGEN3_INCLUDE_DIR: " ${EIGEN3_INCLUDE_DIR} )
message("VLFEAT_LIB: " ${VLFEAT_LIB} )
message("VLFEAT_INCLUDE_DIR: " ${VLFEAT_INCLUDE_DIR} )
message("SQLite3_INCLUDE_DIRS: " ${SQLite3_INCLUDE_DIRS} )
message("SQLite3_LIBRARIES: " ${SQLite3_LIBRARIES} )
message("CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR} )
message("VISE_SOURCE_ROOT_DIR: " ${VISE_SOURCE_ROOT_DIR} )
message("OpenMP: " ${OpenMP_CXX_INCLUDE_DIRS} )

set(CMAKE_INCLUDE_CURRENT_DIR ON) # i.e. include_directories(.)
include_directories( . )

## include for relja_retrival
## @todo: remove all the include_directories() that are not required by
include_directories( search_engine )
include_directories( search_engine/relja_retrival )
include_directories( search_engine/relja_retrival/api )
include_directories( search_engine/relja_retrival/compression )
include_directories( search_engine/relja_retrival/external )
include_directories( search_engine/relja_retrival/external/KMCode_relja/exec/detect_points )
include_directories( search_engine/relja_retrival/external/KMCode_relja/exec/compute_descriptors )
include_directories( search_engine/relja_retrival/preprocessing )
include_directories( search_engine/relja_retrival/indexing )
include_directories( search_engine/relja_retrival/matching )
include_directories( search_engine/relja_retrival/matching/det_ransac )
include_directories( search_engine/relja_retrival/matching/registration )
include_directories( search_engine/relja_retrival/represent )
include_directories( search_engine/relja_retrival/retrieval )
include_directories( search_engine/relja_retrival/util )
include_directories( search_engine/relja_retrival/v2/dataset )
include_directories( search_engine/relja_retrival/v2/embedding )
include_directories( search_engine/relja_retrival/v2/evaluation )
include_directories( search_engine/relja_retrival/v2/indexing )
include_directories( search_engine/relja_retrival/v2/indexing/train )
include_directories( search_engine/relja_retrival/v2/retrieval )

add_subdirectory( "search_engine" )
add_subdirectory( "vise" )

##
## Packaging installer using cpack
##
## See
## - https://github.com/mgalloy/cpack-cheatsheets/blob/master/cpack-variables.rst\
if(WIN32 AND (CMAKE_GENERATOR_PLATFORM MATCHES "x64"))
  #INCLUDE(InstallRequiredSystemLibraries)
  SET(CPACK_GENERATOR "NSIS")
  SET(CPACK_PACKAGE_NAME "${VISE_NAME}")
  SET(CPACK_PACKAGE_VENDOR "Visual Geometry Group (VGG)")
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${VISE_FULLNAME} (${VISE_NAME}) is a standalone application for visual search of images.")
  SET(CPACK_PACKAGE_VERSION "${VISE_VERSION_MAJOR}.${VISE_VERSION_MINOR}.${VISE_VERSION_PATCH}")
  SET(CPACK_PACKAGE_VERSION_MAJOR ${VISE_VERSION_MAJOR})
  SET(CPACK_PACKAGE_VERSION_MINOR ${VISE_VERSION_MINOR})
  SET(CPACK_PACKAGE_VERSION_PATCH ${VISE_VERSION_PATCH})
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${VISE_NAME}")

  SET(CPACK_PACKAGE_EXECUTABLES "${VISE_NAME}" "${VISE_FULLNAME}")
  SET(CPACK_SYSTEM_NAME "Win64")

  SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}\\\\res\\\\win\\\\LICENSE_no_newline.txt")

  # NSIS specific settings
  SET(CPACK_NSIS_CONTACT "Abhishek Dutta <adutta@robots.ox.ac.uk>")
  #SET(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
  #SET(CPACK_NSIS_MUI_FINISHPAGE_RUN "$INSTDIR\\\\bin\\\\${VISE_NAME}.exe")

  SET(CPACK_NSIS_PACKAGE_NAME "${VISE_NAME}")
  SET(CPACK_NSIS_DISPLAY_NAME "${VISE_FULLNAME}")
  SET(CPACK_NSIS_MODIFY_PATH "OFF")
  SET(CPACK_NSIS_MANIFEST_DPI_AWARE "ON")

  SET(CPACK_NSIS_WELCOME_TITLE "Welcome to VGG Image Search Engine (VISE) Installer")
  SET(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\res\\\\win\\\\vise_icon.ico")
  SET(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\res\\\\win\\\\vise_icon.ico")
  SET(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\res\\\\win\\\\vise_uninstall_icon.ico")
  SET(CPACK_NSIS_MUI_HEADERIMAGE "${CMAKE_CURRENT_SOURCE_DIR}\\\\res\\\\win\\\\vise_logo_blur.bmp")
  SET(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${CMAKE_CURRENT_SOURCE_DIR}\\\\res\\\\win\\\\vise_logo_400x700.bmp")

  SET(CPACK_NSIS_HELP_LINK "https://www.robots.ox.ac.uk/~vgg/software/vise/")
  SET(CPACK_NSIS_URL_INFO_ABOUT "https://www.robots.ox.ac.uk/~vgg/software/vise/")

  #SET(CPACK_NSIS_MUI_FINISHPAGE_RUN "$INSTDIR\\\\bin\\\\${RELEASE_PACKAGE_NAME}.exe")

  # copy required resources
  INSTALL(DIRECTORY "${VISE_DEPS_BASEDIR_4SLASH}\\\\" DESTINATION "bin")
  INSTALL(DIRECTORY "${VISE_MAGICK_DEPS_BASEDIR_4SLASH}\\\\" DESTINATION "bin")

  INSTALL(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/www/" DESTINATION "www")
  #INSTALL(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/res/" DESTINATION "res")
  INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/win/vise_logo.bmp" DESTINATION "bin")
  INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/win/vise_icon.ico" DESTINATION "bin")

  INSTALL(DIRECTORY "${VISE_DEMO_PROJECTS_DIR_4SLASH}\\\\" DESTINATION "demo_projects")
  INSTALL(DIRECTORY "${VISE_ASSET_DIR_4SLASH}\\\\" DESTINATION "asset")
  INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt" DESTINATION ".")
  INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.txt" DESTINATION ".")
  INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.txt" DESTINATION ".")
  INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE_3rd_party.txt" DESTINATION ".")

  # Create Start menu shortcut
  set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\VISE.lnk' '$INSTDIR\\\\bin\\\\VISE.exe'")
  set(CPACK_NSIS_DELETE_ICONS_EXTRA "Delete '$SMPROGRAMS\\\\$START_MENU\\\\VISE.lnk'")

  # install dependencies (not required as we copy all the required DLLs)
  #list( APPEND  CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS "  ExecWait \\\"$INSTDIR\\\\vcredist_x64.exe /install /quiet /norestart\\\"")
  #list( APPEND  CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS "  ExecWait \\\"$INSTDIR\\\\VC_redist.x64.exe /install /quiet /norestart\\\"")
  #string (REPLACE ";" "\n" CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS "${CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS}")

  # move demo projects to VISE application data folder
  list( APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "  SetShellVarContext current")
  list( APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "  CreateDirectory \\\"$LOCALAPPDATA\\\\VISE\\\\store\\\"")
  list( APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "  CopyFiles \\\"$INSTDIR\\\\demo_projects\\\\*\\\" \\\"$LOCALAPPDATA\\\\${VISE_NAME}\\\\store\\\"")
  list( APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "  CopyFiles \\\"$INSTDIR\\\\asset\\\\*\\\" \\\"$LOCALAPPDATA\\\\${VISE_NAME}\\\\asset\\\"")
  list( APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "  CopyFiles \\\"$INSTDIR\\\\www\\\\*\\\" \\\"$LOCALAPPDATA\\\\${VISE_NAME}\\\\www\\\"")
  list( APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "  RMDir /r \\\"$INSTDIR\\\\demo_projects\\\"")
  list( APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "  RMDir /r \\\"$INSTDIR\\\\asset\\\"")
  list( APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "  RMDir /r \\\"$INSTDIR\\\\www\\\"")
  #list( APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "  DetailPrint $LOCALAPPDATA") # for debug
  #list( APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "  Sleep 50000") # for debug
  string (REPLACE ";" "\n" CPACK_NSIS_EXTRA_INSTALL_COMMANDS "${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}")

  # remove application files during uninstall
  list( APPEND CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "  SetShellVarContext current")
  list( APPEND CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "  RMDir /r \\\"$LOCALAPPDATA\\\\${VISE_NAME}\\\\store\\\"")
  list( APPEND CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "  RMDir /r \\\"$LOCALAPPDATA\\\\${VISE_NAME}\\\\asset\\\"")
  list( APPEND CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "  RMDir /r \\\"$LOCALAPPDATA\\\\${VISE_NAME}\\\\www\\\"")
  list( APPEND CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "  RMDir /r \\\"$LOCALAPPDATA\\\\${VISE_NAME}\\\"")
  string (REPLACE ";" "\n" CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "${CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS}")

  INCLUDE(CPack)
endif()
