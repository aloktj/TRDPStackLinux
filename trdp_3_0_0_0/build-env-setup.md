# Build environment setup and configuration results

This document summarizes the dependencies installed for the build environment, the build results for
all Makefile configurations, and the additional toolchains required for non-native targets.

## Base dependencies installed

Use the setup script below to install the common build dependencies for native Linux builds:

```bash
./scripts/setup-build-env.sh
```

The script installs:

- build-essential
- uuid-dev
- doxygen (for `make doc`)

### Multilib and cross-toolchain options

The script supports optional cross-toolchain setup and multilib installs. Note that Ubuntu packages
for multilib and cross toolchains conflict; install cross toolchains first, then re-enable multilib
if you need 32-bit builds afterward.

```bash
INSTALL_CROSS=1 INSTALL_MULTILIB=0 ./scripts/setup-build-env.sh
INSTALL_MULTILIB=1 ./scripts/setup-build-env.sh
```

Multilib installs also add `i386` architecture and install `uuid-dev:i386` for `LINUX_X86_config`.

## Build results by configuration

All configs in `config/*_config` were attempted via:

```bash
make distclean
make <CONFIG>
make -j1
```

### Succeeded

- CENTOS_X86_64_config
- LINUX_HP10_config
- LINUX_HP2_config
- LINUX_PPC_config (via symlinked PowerPC toolchain)
- LINUX_TSN_config
- LINUX_X86_64_HP_config
- LINUX_X86_64_HP_conform_config
- LINUX_X86_64_config
- LINUX_X86_config (after multilib + uuid-dev:i386)
- LINUX_config
- LINUX_imx7_config (via toolchain symlinks)
- LINUX_sama5d27_config (via toolchain symlinks)
- POSIX_X86_config
- QNX_X86_config
- RASPIAN_config

### Failed (additional toolchains or OS-specific SDKs required)

- OSX_64_TSN_config
  * Missing macOS UUID symbol `uuid_generate_time` (requires macOS toolchain/runtime libs).
- OSX_X86_64_HP_config
  * Missing macOS UUID symbol `uuid_generate_time` (requires macOS toolchain/runtime libs).
- OSX_X86_64_config
  * Missing macOS UUID symbol `uuid_generate_time` (requires macOS toolchain/runtime libs).
- OSX_X86_SOA_TSN_config
  * Missing macOS UUID symbol `uuid_generate_time` (requires macOS toolchain/runtime libs).
- VXWORKS_KMODE_PPC_config
  * Missing VxWorks toolchain binary (`ccppc`).
- VXWORKS_PPC_config
  * Host GCC does not support `-mlongcall` (requires VxWorks PPC toolchain).

## Notes for additional toolchains

The configuration files reference external toolchains that must be installed and available before the
above failed configs can succeed:

- **PowerPC (LINUX_PPC_config)**
  * Requires `powerpc-linux-gnu-*` tools. The setup script creates symlinks under
    `/scm/gbe/repository/toolchain/nrtos/1.0.0.0/x86-linux2/bin` to match the config path.
- **Yocto (LINUX_imx7_config)**
  * Requires Yocto SDK and `YOCTO_DIR` for a full Yocto environment. The setup script provides
    a minimal compatibility symlink under `/build/tmp/sysroots/...` for GCC-based builds.
- **ARM (LINUX_sama5d27_config)**
  * Requires `arm-poky-linux-gnueabi-*` tools. The setup script provides symlinks to the Debian
    `arm-linux-gnueabihf` toolchain.
- **macOS (OSX_* configs)**
  * Build on macOS with Xcode and the system UUID library providing `uuid_generate_time`.
- **VxWorks (VXWORKS_* configs)**
  * Install the VxWorks toolchain and set the `WIND_*` environment variables used by the config.
