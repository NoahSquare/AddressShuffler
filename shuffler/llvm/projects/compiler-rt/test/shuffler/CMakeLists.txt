set(SHUFFLER_LIT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(SHUFFLER_TESTSUITES)
set(SHUFFLER_TEST_DEPS ${SANITIZER_COMMON_LIT_TEST_DEPS})

macro(add_shuffler_testsuite test_mode sanitizer arch)
  set(SHUFFLER_LIT_TEST_MODE "${test_mode}")
  set(CONFIG_NAME ${SHUFFLER_LIT_TEST_MODE}-${arch})
  configure_lit_site_cfg(
    ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
    ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_NAME}/lit.site.cfg)
  list(APPEND SHUFFLER_TESTSUITES ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_NAME})
  if(NOT COMPILER_RT_STANDALONE_BUILD)
    list(APPEND SHUFFLER_TEST_DEPS ${sanitizer})
  endif()
endmacro()

set(SHUFFLER_TEST_ARCH ${SHUFFLER_SUPPORTED_ARCH})

foreach(arch ${SHUFFLER_TEST_ARCH})
  set(SHUFFLER_TEST_TARGET_ARCH ${arch})

  get_target_flags_for_arch(${arch} SHUFFLER_TEST_TARGET_CFLAGS)
  string(REPLACE ";" " " SHUFFLER_TEST_TARGET_CFLAGS "${SHUFFLER_TEST_TARGET_CFLAGS}")

  add_shuffler_testsuite("Standalone" shuffler ${arch})
endforeach()

add_lit_testsuite(check-shuffler "Running the Shuffler tests"
  ${SHUFFLER_TESTSUITES}
  DEPENDS ${SHUFFLER_TEST_DEPS})
set_target_properties(check-shuffler PROPERTIES FOLDER "Shuffler tests")
