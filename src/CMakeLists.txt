set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(BINIO_GENERATED_INCLUDE_DIR "${GENERATED_DIR}/include")
set(BINIO_H_FILE "${BINIO_GENERATED_INCLUDE_DIR}/binio.h")
set(BINIO_CONFIG_FILE "${GENERATED_DIR}/cmake/libbinioConfigVersion.cmake")

set(SOURCES
  binio.cpp
  binfile.cpp
  binwrap.cpp
  binstr.cpp
)

set(HEADERS
  binfile.h
  binwrap.h
  binstr.h
)

configure_file(binio.h.in "${BINIO_H_FILE}")

add_library(libbinio)
add_library(libbinio::libbinio ALIAS libbinio)

set_target_properties(libbinio PROPERTIES
                      VERSION ${libbinio_VERSION}
                      SOVERSION ${libbinio_VERSION_MAJOR})

target_sources(libbinio
  PRIVATE
    ${SOURCES}
    ${HEADERS}
    "${BINIO_H_FILE}"
)

set(ABSOLUTE_HEADERS ${HEADERS})
list(TRANSFORM ABSOLUTE_HEADERS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
list(APPEND ABSOLUTE_HEADERS "${BINIO_H_FILE}")

set_property(TARGET libbinio PROPERTY PUBLIC_HEADER ${ABSOLUTE_HEADERS})

target_include_directories(libbinio
  PUBLIC "$<BUILD_INTERFACE:${BINIO_GENERATED_INCLUDE_DIR}>"
  INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

if(BUILD_SHARED_LIBS)
  include(GenerateExportHeader)
  generate_export_header(libbinio
    EXPORT_FILE_NAME "${BINIO_GENERATED_INCLUDE_DIR}/libbinio_exports.h"
  )
endif()

if(HAVE_LIB_M)
  target_link_libraries(libbinio PUBLIC m)
endif()

