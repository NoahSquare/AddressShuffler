set(SHUFFLER_LIT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(SHUFFLER_TESTSUITES)

set(SHUFFLER_TEST_ARCH ${SHUFFLER_SUPPORTED_ARCH})

foreach(arch ${SHUFFLER_TEST_ARCH})
  set(SHUFFLER_TEST_TARGET_ARCH ${arch})
  string(TOLOWER "-${arch}" SHUFFLER_TEST_CONFIG_SUFFIX)
  get_target_flags_for_arch(${arch} SHUFFLER_TEST_TARGET_CFLAGS)
  string(REPLACE ";" " " SHUFFLER_TEST_TARGET_CFLAGS "${SHUFFLER_TEST_TARGET_CFLAGS}")

  string(TOUPPER ${arch} ARCH_UPPER_CASE)
  set(CONFIG_NAME ${ARCH_UPPER_CASE}ShufflerConfig)

  configure_lit_site_cfg(
    ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
    ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_NAME}/lit.site.cfg)
  list(APPEND SHUFFLER_TESTSUITES ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_NAME})
endforeach()

set(SHUFFLER_TEST_DEPS ${SANITIZER_COMMON_LIT_TEST_DEPS})

add_lit_testsuite(check-shuffler "Running the Shuffler tests"
  ${SHUFFLER_TESTSUITES}
  DEPENDS ${SHUFFLER_TEST_DEPS})
set_target_properties(check-shuffler PROPERTIES FOLDER "Shuffler tests")
