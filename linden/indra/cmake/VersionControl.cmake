# -*- cmake -*-
#
# Utility macros for getting info from the version control system.

macro(vcs_get_revision _output_variable)
  FIND_PROGRAM(Git_EXECUTABLE git git.cmd)
  if (Git_EXECUTABLE)
    execute_process(
      COMMAND ${Git_EXECUTABLE} describe
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/..
      OUTPUT_VARIABLE _git_describe_output
      ERROR_VARIABLE _git_describe_error
      RESULT_VARIABLE _git_describe_result
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_STRIP_TRAILING_WHITESPACE
      )
    if (NOT ${_git_describe_result})
      message(STATUS "git describe failed: ${_git_describe_error}")
      set(${_output_variable} "0.0.0.0")
    else (NOT ${_git_describe_result})
      set(KV_FULL_REGEX "v([0-9]+)\\.([0-9]+)\\.([0-9]+)-([0-9]+)")
      if (NOT "${_git_describe_output}" MATCHES "${KV_FULL_REGEX}")
        string(REGEX REPLACE 
          "v([0-9]+)\\.([0-9]+)\\.([0-9]+)"
          "\\1.\\2.\\3.0"
          ${_output_variable}
          ${_git_describe_output})
      else (NOT "${_git_describe_output}" MATCHES "${KV_FULL_REGEX}")
        string(REGEX REPLACE 
          "v([0-9]+)\\.([0-9]+)\\.([0-9]+)-([0-9]+)-.*"
          "\\1.\\2.\\3.\\4"
          ${_output_variable}
          ${_git_describe_output})
      endif (NOT "${_git_describe_output}" MATCHES "${KV_FULL_REGEX}")
    endif (NOT ${_git_describe_result})
  else (Git_EXECUTABLE)
    set(${_output_variable} "0.0.0.0")
  endif (Git_EXECUTABLE)
endmacro(vcs_get_revision)

# Sets the following from git.
# VIEWER_VERSION
# VIEWER_VERSION_MAJOR
# VIEWER_VERSION_MINOR
# VIEWER_VERSION_PATCH
# VIEWER_VERSION_BUILD
macro(load_version_data)
  if (SERVER)
    message(SEND_ERROR "We can't do server builds.")
  endif (SERVER)
  if (VIEWER)
    vcs_get_revision(VERSION_VIEWER)
    split_version_parts(
      "${VERSION_VIEWER}"
      VERSION_VIEWER_MAJOR
      VERSION_VIEWER_MINOR
      VERSION_VIEWER_PATCH
      VERSION_VIEWER_BUILD
      )
  endif (VIEWER)
endmacro(load_version_data)

# Split the version into major, minor and patch parts, and assign them to the
# variable names supplied to the macro.
macro(split_version_parts _version _major _minor _patch _build)
  set(INDRA_VERSION_RE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
  if (NOT "${_version}" MATCHES "${INDRA_VERSION_RE}")
    message(FATAL_ERROR "${_version} is not a valid version string. It should be a 4 part version, MAJOR.MINOR.PATCH.BUILD. Example: 2.1.49.4")
  endif (NOT "${_version}" MATCHES "${INDRA_VERSION_RE}")
  string(REGEX REPLACE "${INDRA_VERSION_RE}" "\\1" ${_major} "${_version}")
  string(REGEX REPLACE "${INDRA_VERSION_RE}" "\\2" ${_minor} "${_version}")
  string(REGEX REPLACE "${INDRA_VERSION_RE}" "\\3" ${_patch} "${_version}")
  string(REGEX REPLACE "${INDRA_VERSION_RE}" "\\4" ${_build} "${_version}")
endmacro(split_version_parts)
