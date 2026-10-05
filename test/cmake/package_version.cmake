# The installed package must use the public version, not the shared library version.
function(check_package_version requested_version expected_match)
   set(PACKAGE_FIND_VERSION "${requested_version}")
   include("${VERSION_FILE}")

   if (NOT PACKAGE_VERSION STREQUAL DISPLAY_VERSION)
      message(FATAL_ERROR "Package version ${PACKAGE_VERSION} differs from ${DISPLAY_VERSION}")
   endif ()

   if (expected_match)
      if (NOT PACKAGE_VERSION_EXACT OR NOT PACKAGE_VERSION_COMPATIBLE)
         message(FATAL_ERROR "Public version ${requested_version} was not accepted")
      endif ()
   elseif (PACKAGE_VERSION_EXACT OR PACKAGE_VERSION_COMPATIBLE)
      message(FATAL_ERROR "Library version ${requested_version} was accepted")
   endif ()
endfunction()

check_package_version("${DISPLAY_VERSION}" TRUE)
if (NOT LIBRARY_VERSION STREQUAL DISPLAY_VERSION)
   check_package_version("${LIBRARY_VERSION}" FALSE)
endif ()
