cmake_minimum_required(VERSION 3.4.1)
project("OnsemJni")

set(CMAKE_CXX_FLAGS "-std=gnu++17 -D_GLIBCXX_USE_CXX11_ABI=0" CACHE STRING "" FORCE)


option(
  BUILD_ONSEM_DATABASE
  "Build the linguistic databases."
  FALSE)


option(
  BUILD_SEMANTIC
  "Build the semantic libraries for the robot."
  TRUE)

option(
        BUILD_ONSEM_TESTS
        "Build the tests of onsem."
        FALSE)


add_subdirectory(onsem)

include(${CMAKE_CURRENT_SOURCE_DIR}/onsem/subdirectories/boost/boost_property_tree_with_deps.cmake)


# Code to build the semantic C++ libraries for the robot
# ======================================================
if (NOT BUILD_ONSEM_DATABASE)

  # Specifies a library name, specifies whether the library is STATIC or
  # SHARED, and provides relative paths to the source code. You can
  # define multiple libraries by adding multiple add_library() commands,
  # and CMake builds them for you. When you build your app, Gradle
  # automatically packages shared libraries with your APK.

  if (ANDROID)

    find_library( # Sets the name of the path variable.
            android-lib

            # Specifies the name of the NDK library that
            # you want CMake to locate.
            android )

    add_library( # Specifies the name of the library.
          onsem-jni

          # Sets the library as a shared library.
          SHARED

          # Provides a relative path to your source file(s).
          "jni/androidlog.hpp"

          "jni/semanticenumsindexes.hpp"
          "jni/semanticenumsindexes.cpp"
          "jni/keytoassetstreams.hpp"
          "jni/jobjectstocpptypes.hpp"
          "jni/jobjectstocpptypes.cpp"
          "jni/onsem-jni.h"
          "jni/onsem-jni.cpp"
          "jni/linguisticdatabase-jni.hpp"
          "jni/linguisticdatabase-jni.cpp"
          "jni/recommendationsfinder-jni.cpp"
          "jni/semanticexpression-jni.hpp"
          "jni/semanticexpression-jni.cpp"
          "jni/semanticmemory-jni.hpp"
          "jni/semanticmemory-jni.cpp"
          "jni/textprocessingcontext-jni.hpp"
          "jni/textprocessingcontext-jni.cpp"
          "jni/triggers-jni.cpp"
    )

    if (COUT_TO_ANDROID_LOG)
      # Double-check Android log option consistency.
      if (NOT ANDROID)
        message(WARNING "COUT_TO_ANDROID_LOG is ON but ANDROID is OFF, option ignored")
        set(COUT_TO_ANDROID_LOG OFF)
      endif ()

      target_compile_definitions(onsem-jni PRIVATE COUT_TO_ANDROID_LOG)
      target_link_libraries(onsem-jni PRIVATE log)
    endif ()


    include_directories(
          ${ANDROID_INCLUDE_DIRS}
          ${BOOST_PROPERTY_TREE_WITH_DEPS_INCLUDE_DIRS}
          ${BOOST_INCLUDE_DIRS}
          ${ONSEMCOMMON_INCLUDE_DIRS}
          ${ONSEMTEXTTOSEMANTIC_INCLUDE_DIRS}
          ${ONSEMSEMANTICTOTEXT_INCLUDE_DIRS}
    )
    target_link_libraries(
          onsem-jni PRIVATE
          onsemcommon
          onsemtexttosemantic
          onsemsemantictotext
    )
    target_link_libraries(onsem-jni PUBLIC ${JNI_LIBRARIES} ${android-lib})


  endif (ANDROID)
endif (NOT BUILD_ONSEM_DATABASE)

