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
- gcc-multilib / g++-multilib (needed for 32-bit Linux builds)
- doxygen (for `make doc`)

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
- LINUX_TSN_config
- LINUX_X86_64_HP_config
- LINUX_X86_64_HP_conform_config
- LINUX_X86_64_config
- LINUX_config
- POSIX_X86_config
- QNX_X86_config
- RASPIAN_config

### Failed (additional toolchains or libraries required)

- LINUX_PPC_config
  * Missing PowerPC toolchain path defined in config (`/scm/gbe/repository/.../powerpc-linux-gnu-gcc`).
- LINUX_X86_config
  * 32-bit link failed due to missing 32-bit UUID library (`-luuid` for 32-bit).
- LINUX_imx7_config
  * Missing Yocto SDK toolchain (`arm-poky-linux-gnueabi-gcc`).
- LINUX_sama5d27_config
  * Missing ARM toolchain (`arm-poky-linux-gnueabi-gcc`).
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
  * Provide the `powerpc-linux-gnu-` toolchain and set `TCPATH` or ensure it matches the config path.
- **Yocto (LINUX_imx7_config)**
  * Install Yocto SDK and set `YOCTO_DIR` so the compiler lives under
    `$YOCTO_DIR/build/tmp/sysroots/x86_64-linux/usr/bin/arm-poky-linux-gnueabi/`.
- **ARM (LINUX_sama5d27_config)**
  * Install `arm-poky-linux-gnueabi-gcc` (or adjust `TCPREFIX/TCPATH`).
- **macOS (OSX_* configs)**
  * Build on macOS with Xcode and the system UUID library providing `uuid_generate_time`.
- **VxWorks (VXWORKS_* configs)**
  * Install VxWorks toolchain and set the `WIND_*` environment variables used by the config.
