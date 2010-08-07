# -*- cmake -*-
include(Prebuilt)

use_prebuilt_binary(Growl)

if (DARWIN)
    set(GROWL_INCLUDE_DIRS ${LIBS_PREBUILT_DIR}/include/Growl)
    set(GROWL_LIBRARY Growl)
endif (DARWIN)
