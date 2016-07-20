add_custom_target(shuffler)

set(SHUFFLER_SOURCES
	shuffler_malloc.cc
)

include_directories(..)

set(SHUFFLER_CFLAGS ${SHUFFLER_COMMON_CFLAGS})

if(APPLE)
  # Build universal binary on APPLE.
  add_compiler_rt_runtime(clang_rt.shuffler
    STATIC
    OS osx
    ARCHS ${SAFESTACK_SUPPORTED_ARCH}
    SOURCES ${SHUFFLER_SOURCES}
    CFLAGS ${SHUFFLER_COMMON_CFLAGS}
    PARENT_TARGET shuffler)
else()
  # Otherwise, build separate libraries for each target.
  foreach(arch ${SAFESTACK_SUPPORTED_ARCH})
    add_compiler_rt_runtime(clang_rt.shuffler
      STATIC
      ARCHS ${arch}
      SOURCES ${SHUFFLER_SOURCES}
      CFLAGS ${SHUFFLER_COMMON_CFLAGS}
      PARENT_TARGET shuffler)
  endforeach()
endif()