#!/usr/bin/env bash
set -euo pipefail

# This script installs common dependencies used by the Makefile builds.
# Optional cross-toolchain setup is controlled via environment variables.

INSTALL_CROSS=${INSTALL_CROSS:-0}
INSTALL_MULTILIB=${INSTALL_MULTILIB:-1}

sudo apt-get update
sudo apt-get install -y \
  build-essential \
  uuid-dev \
  doxygen

if [[ "$INSTALL_MULTILIB" == "1" ]]; then
  sudo apt-get install -y gcc-multilib g++-multilib
  sudo dpkg --add-architecture i386
  sudo apt-get update
  sudo apt-get install -y uuid-dev:i386
fi

if [[ "$INSTALL_CROSS" == "1" ]]; then
  sudo apt-get install -y \
    gcc-powerpc-linux-gnu \
    g++-powerpc-linux-gnu \
    gcc-arm-linux-gnueabihf \
    g++-arm-linux-gnueabihf

  sudo mkdir -p /scm/gbe/repository/toolchain/nrtos/1.0.0.0/x86-linux2/bin
  sudo ln -sf /usr/bin/powerpc-linux-gnu-gcc /scm/gbe/repository/toolchain/nrtos/1.0.0.0/x86-linux2/bin/powerpc-linux-gnu-gcc
  sudo ln -sf /usr/bin/powerpc-linux-gnu-g++ /scm/gbe/repository/toolchain/nrtos/1.0.0.0/x86-linux2/bin/powerpc-linux-gnu-g++
  sudo ln -sf /usr/bin/powerpc-linux-gnu-ar /scm/gbe/repository/toolchain/nrtos/1.0.0.0/x86-linux2/bin/powerpc-linux-gnu-ar
  sudo ln -sf /usr/bin/powerpc-linux-gnu-strip /scm/gbe/repository/toolchain/nrtos/1.0.0.0/x86-linux2/bin/powerpc-linux-gnu-strip

  sudo ln -sf /usr/bin/arm-linux-gnueabihf-gcc /usr/bin/arm-poky-linux-gnueabi-gcc
  sudo ln -sf /usr/bin/arm-linux-gnueabihf-g++ /usr/bin/arm-poky-linux-gnueabi-g++
  sudo ln -sf /usr/bin/arm-linux-gnueabihf-ar /usr/bin/arm-poky-linux-gnueabi-ar
  sudo ln -sf /usr/bin/arm-linux-gnueabihf-strip /usr/bin/arm-poky-linux-gnueabi-strip

  sudo mkdir -p /build/tmp/sysroots/x86_64-linux/usr/bin/arm-poky-linux-gnueabi
  sudo ln -sf /usr/bin/arm-poky-linux-gnueabi-gcc /build/tmp/sysroots/x86_64-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc
  sudo ln -sf /usr/bin/arm-poky-linux-gnueabi-g++ /build/tmp/sysroots/x86_64-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-g++
  sudo ln -sf /usr/bin/arm-poky-linux-gnueabi-ar /build/tmp/sysroots/x86_64-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-ar
  sudo ln -sf /usr/bin/arm-poky-linux-gnueabi-strip /build/tmp/sysroots/x86_64-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-strip
fi

cat <<'NOTE'

Usage notes:

- The multilib toolchain conflicts with the cross toolchains on Ubuntu. If you need both,
  install the cross toolchains first (INSTALL_CROSS=1) and rebuild multilib afterward
  (INSTALL_MULTILIB=1).

Additional toolchain setup (manual):

- LINUX_PPC_config
  * Requires the PowerPC toolchain path used by the config file. This script provides
    symlinks to /scm/gbe/repository/toolchain/nrtos/1.0.0.0/x86-linux2/bin.

- LINUX_imx7_config
  * Requires Yocto SDK and YOCTO_DIR if you want an exact Yocto layout. This script provides
    a minimal compatibility symlink under /build/tmp/sysroots/... for the GCC toolchain.

- LINUX_sama5d27_config
  * Requires arm-poky-linux-gnueabi-* tools. This script provides symlinks to the Debian
    arm-linux-gnueabihf toolchain.

- OSX_* configs
  * Require macOS toolchain and UUID library (uuid_generate_time) for linking.

- VXWORKS_KMODE_PPC_config / VXWORKS_PPC_config
  * Require VxWorks toolchain and WIND_* environment variables from the vendor SDK.

NOTE
