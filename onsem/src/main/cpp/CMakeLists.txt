cmake_minimum_required(VERSION 3.4.1)
project("OnsemJni")

set(CMAKE_CXX_FLAGS "-std=gnu++17 -D_GLIBCXX_USE_CXX11_ABI=0" CACHE STRING "" FORCE)


option(
  BUILD_ONSEM_DATABASE
  "Build the linguistic databases."
  FALSE)

option(
        BUILD_ONSEM_CHATBOTPLANNER
        "Build the chatbot planner."
        TRUE)


option(
  BUILD_SEMANTIC
  "Build the semantic libraries for the robot."
  TRUE)

option(
        USE_BOOST_FROM_SYSTEM
        "Do not build with the dependency of the boost installed in the system."
        OFF)

option(
        BUILD_ONSEM_TESTS
        "Build the tests of onsem."
        FALSE)


set(BOOST_INCLUDE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/any/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/bind/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/foreach/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/format/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/multi_index/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/property_tree/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/range/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/serialization/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/type_index/include"

        "${CMAKE_CURRENT_SOURCE_DIR}/boost/align/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/assert/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/atomic/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/concept_check/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/config/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/container_hash/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/conversion/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/core/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/detail/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/filesystem/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/function_types/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/fusion/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/integer/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/io/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/iterator/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/move/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/mp11/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/mpl/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/optional/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/predef/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/preprocessor/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/smart_ptr/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/static_assert/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/system/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/throw_exception/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/tuple/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/typeof/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/type_traits/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/utility/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/variant2/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/boost/winapi/include"
        )




# Code to build the databases
# ===========================
if (BUILD_ONSEM_DATABASE)

  add_subdirectory(boost/align)
  add_subdirectory(boost/assert)
  add_subdirectory(boost/atomic)
  add_subdirectory(boost/throw_exception)
  add_subdirectory(boost/config)
  add_subdirectory(boost/container_hash)
  add_subdirectory(boost/core)
  add_subdirectory(boost/detail)
  add_subdirectory(boost/utility)
  add_subdirectory(boost/iterator)
  add_subdirectory(boost/preprocessor)
  add_subdirectory(boost/static_assert)
  add_subdirectory(boost/type_traits)
  add_subdirectory(boost/integer)
  add_subdirectory(boost/move)
  add_subdirectory(boost/mpl)
  add_subdirectory(boost/tuple)
  add_subdirectory(boost/optional)
  add_subdirectory(boost/io)
  add_subdirectory(boost/mp11)
  add_subdirectory(boost/smart_ptr)
  add_subdirectory(boost/system)
  add_subdirectory(boost/predef)
  add_subdirectory(boost/concept_check)
  add_subdirectory(boost/conversion)
  add_subdirectory(boost/typeof)
  add_subdirectory(boost/filesystem)
  add_subdirectory(boost/function_types)
  add_subdirectory(boost/fusion)
  add_subdirectory(boost/variant2)
  add_subdirectory(boost/winapi)

endif (BUILD_ONSEM_DATABASE)


  add_subdirectory(onsem)



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

