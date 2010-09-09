# -*- cmake -*-

include(VersionControl)

macro (build_version _target)
  load_version_data()
  set(${_target}_VERSION "${VERSION_VIEWER}")
  if (${_target}_VERSION)
    message(STATUS "Version of ${_target} is ${${_target}_VERSION}")
  else (${_target}_VERSION)
    message(SEND_ERROR "Could not determine ${_target} version")
  endif (${_target}_VERSION)
endmacro (build_version)
