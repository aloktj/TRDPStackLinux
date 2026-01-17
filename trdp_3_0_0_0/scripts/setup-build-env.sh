#!/usr/bin/env bash
set -euo pipefail

# This script installs common dependencies used by the Makefile builds.
# Cross-toolchains (Yocto, VxWorks, PowerPC, etc.) must be installed separately.

sudo apt-get update
sudo apt-get install -y \
  build-essential \
  uuid-dev \
  gcc-multilib \
  g++-multilib \
  doxygen

cat <<'NOTE'

Additional toolchain setup (manual):

- LINUX_PPC_config
  * Requires the PowerPC toolchain at /scm/gbe/repository/toolchain/nrtos/1.0.0.0/x86-linux2/bin/
    with powerpc-linux-gnu-gcc in PATH (see config/LINUX_PPC_config).

- LINUX_imx7_config
  * Requires Yocto SDK and YOCTO_DIR to be set, with arm-poky-linux-gnueabi-gcc available
    under $YOCTO_DIR/build/tmp/sysroots/x86_64-linux/usr/bin/arm-poky-linux-gnueabi/.

- LINUX_sama5d27_config
  * Requires arm-poky-linux-gnueabi-gcc available in /usr/bin or PATH.

- OSX_* configs
  * Require macOS toolchain and UUID library (uuid_generate_time) for linking.

- VXWORKS_KMODE_PPC_config
  * Requires VxWorks toolchain at /opt/cross/vxworks-6.6 (ccppc, WIND_* envs).

- VXWORKS_PPC_config
  * Requires a compiler supporting -mlongcall (VxWorks PowerPC toolchain).

NOTE
