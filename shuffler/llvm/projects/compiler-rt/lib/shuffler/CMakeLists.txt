add_custom_target(shuffler)

set(SHUFFLER_SOURCES
	shuffler_malloc.cc
)

include_directories(..)

set(SHUFFLER_CFLAGS ${SHUFFLER_COMMON_CFLAGS})

if(APPLE)
  # Build universal binary on APPLE.
  add_compiler_rt_osx_static_runtime(clang_rt.shuffler_osx
    ARCH ${CVER_SUPPORTED_ARCH}
    SOURCES ${SHUFFLER_SOURCES}
            $<TARGET_OBJECTS:RTShufflerCommon.osx>
    CFLAGS ${SHUFLLER_CFLAGS})
  add_dependencies(shuffler clang_rt.shuffler_osx)
else()
  # Build separate libraries for each target.
  foreach(arch ${CVER_SUPPORTED_ARCH})
    # Main Cver runtime.
    add_compiler_rt_runtime(clang_rt.shuffler-${arch} ${arch} STATIC
      SOURCES ${SHUFFLER_SOURCES}
      CFLAGS ${SHUFLLER_CFLAGS})
    add_dependencies(shuffler
      clang_rt.shuffer-${arch})
    if (UNIX AND NOT ${arch} STREQUAL "i386")
      add_sanitizer_rt_symbols(clang_rt.shuffler-${arch} shuffler.syms.extra)
      add_dependencies(shuffler
        clang_rt.shuffler-${arch}-symbols)
    endif()
  endforeach()
endif()