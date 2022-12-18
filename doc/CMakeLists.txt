option(libbinio_BUILD_DOCUMENTATION "Build documentation" "${libbinio_IS_TOP_LEVEL}")
if(NOT libbinio_BUILD_DOCUMENTATION)
  return()
endif()

find_program(MAKEINFO_EXECUTABLE
  NAMES makeinfo
  DOC "makeinfo executable location"
)

if(NOT MAKEINFO_EXECUTABLE)
  if(NOT WIN32)
    message(STATUS "makeinfo not found")
  endif()
  return()
endif()

if(CMAKE_VERSION VERSION_LESS 3.20)
  function(convert_to_tex result path)
    get_filename_component(base_name "${file}" NAME_WLE)
    set(tex_name "${base_name}.tex")
    set(${result} "${tex_name}" PARENT_SCOPE)
  endfunction()

  function(get_input_path result path)
    get_filename_component(output_path "${path}" ABSOLUTE)
    set(${result} "${output_path}" PARENT_SCOPE)
  endfunction()
else()
  function(convert_to_tex result path)
    cmake_path(GET path STEM LAST_ONLY base_name)
    set(tex_name "${base_name}.tex")
    set(${result} "${tex_name}" PARENT_SCOPE)
  endfunction()

  function(get_input_path result path)
    cmake_path(ABSOLUTE_PATH path OUTPUT_VARIABLE output_path)
    set(${result} "${output_path}" PARENT_SCOPE)
  endfunction()
endif()


set(LIBBINIO_INFO_FILE "${CMAKE_CURRENT_BINARY_DIR}/libbinio.info")
set(LIBBINIO_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/version.texi")

file(TIMESTAMP "${CMAKE_CURRENT_SOURCE_DIR}/libbinio.texi" LIBBINIO_TEXI_UPDATED "%d %B %Y" UTC)
file(TIMESTAMP "${CMAKE_CURRENT_SOURCE_DIR}/libbinio.texi" LIBBINIO_TEXI_MONTH_UPDATED "%B %Y" UTC)

set(at @)

configure_file(version.texi.in "${LIBBINIO_VERSION_FILE}")

set(SOURCES libbinio.texi fdl.texi "${LIBBINIO_VERSION_FILE}")

get_input_path(input_f libbinio.texi)

add_custom_command(
  COMMAND
    "${MAKEINFO_EXECUTABLE}" -o "${LIBBINIO_INFO_FILE}" "${input_f}"
  OUTPUT "${LIBBINIO_INFO_FILE}"
  DEPENDS ${SOURCES}
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "Building documentation"
  VERBATIM
)

add_custom_target(libbinio_doc ALL DEPENDS "${LIBBINIO_INFO_FILE}")

