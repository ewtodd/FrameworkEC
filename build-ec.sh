# Disable all problematic warnings
export CFLAGS="-Wno-error=array-bounds -Wno-array-bounds -Wno-error=address -Wno-address -Wno-error=stringop-truncation -Wno-stringop-truncation"
export CPPFLAGS="$CFLAGS"
export HOSTCFLAGS="$CFLAGS"
export BUILDCFLAGS="$CFLAGS"

# Clear cross-compile for host tools
unset HOST_CROSS_COMPILE

exec make BOARD=banshee \
     CROSS_COMPILE=arm-none-eabi- \
     BUILDCC=gcc \
     HOSTCC=gcc \
     WERROR= \
     ALLOW_CONFIG=1 \
     EXTRA_CFLAGS="$CFLAGS" \
     -j \
     "$@"
