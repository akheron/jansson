file (GLOB C_FILES *.c)

if (BUILD_SHARED_LIBS)

   add_library (jansson SHARED ${C_FILES} jansson.def)

   set_target_properties (jansson PROPERTIES
      VERSION ${JANSSON_VERSION}
      SOVERSION ${JANSSON_SOVERSION})

else ()

   add_library (jansson ${C_FILES})

endif ()

# LIBRARY for linux
# RUNTIME for windows (when building shared)
install (TARGETS jansson
   ARCHIVE DESTINATION lib
   LIBRARY DESTINATION lib
   RUNTIME DESTINATION bin
)
