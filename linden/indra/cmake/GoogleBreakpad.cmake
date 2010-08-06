# -*- cmake -*-
include(Prebuilt)

if (STANDALONE)
  find_package(GoogleBreakpad REQUIRED)
else (STANDALONE)
  use_prebuilt_binary(google_breakpad)
  if (DARWIN)
    set(BREAKPAD_EXCEPTION_HANDLER_LIBRARIES exception_handler)
  endif (DARWIN)
  if (LINUX)
    set(BREAKPAD_EXCEPTION_HANDLER_LIBRARIES breakpad_client)
  endif (LINUX)
  if (WINDOWS)
    set(BREAKPAD_EXCEPTION_HANDLER_LIBRARIES exception_handler crash_generation_client common)
  endif (WINDOWS)
endif (STANDALONE)

