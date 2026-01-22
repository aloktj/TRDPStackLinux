# Integrating TRDPStackLinux into a Root Project

This guide explains how to add TRDPStackLinux as a Git submodule and integrate it into a root project using CMake. The goal is to make the TRDP and TAU APIs available to your root project with minimal configuration.

## 1) Add the repository as a submodule

From your root project directory:

```sh
git submodule add <TRDPSTACK_GIT_URL> external/TRDPStackLinux
git submodule update --init --recursive
```

Recommended layout:

```
<root>/
  external/
    TRDPStackLinux/
      trdp_3_0_0_0/
```

## 2) Add TRDPStack to your root CMake build

In your root `CMakeLists.txt`, include the TRDPStack subdirectory:

```cmake
add_subdirectory(external/TRDPStackLinux/trdp_3_0_0_0)
```

This will build the TRDP library target named `trdp` (with alias `trdp::trdp`).

## 3) Link your targets to TRDP

Link any root target that needs TRDP/TAU APIs:

```cmake
target_link_libraries(my_root_target PRIVATE trdp::trdp)
```

This automatically pulls in TRDP public headers via the `trdp` target’s include directories.

## 4) Optional CMake feature flags

Set TRDP build options before `add_subdirectory` if you need additional functionality:

```cmake
set(TRDP_ENABLE_MD ON CACHE BOOL "" FORCE)
set(TRDP_FULL_BUILD ON CACHE BOOL "" FORCE)
set(TRDP_ENABLE_SOA ON CACHE BOOL "" FORCE)
set(TRDP_ENABLE_TSN OFF CACHE BOOL "" FORCE)
```

Common options:

- `TRDP_ENABLE_MD`: enable Message Data (MD) support.
- `TRDP_FULL_BUILD`: enable XML/marshalling-related functionality.
- `TRDP_ENABLE_SOA`: enable service-oriented APIs.
- `TRDP_ENABLE_TSN`: enable TSN support.

## 5) Example root project snippet

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyRootProject C CXX)

set(TRDP_ENABLE_MD ON CACHE BOOL "" FORCE)
set(TRDP_FULL_BUILD ON CACHE BOOL "" FORCE)

add_subdirectory(external/TRDPStackLinux/trdp_3_0_0_0)

add_executable(root_app src/main.cpp)
target_link_libraries(root_app PRIVATE trdp::trdp)
```

## 6) Notes and best practices

- Keep the submodule pinned to a known-good commit or tag.
- When upgrading TRDPStackLinux, update the submodule commit and rebuild.
- If your build system uses presets, consider adding a preset for TRDP-enabled builds.

## 7) Optional: Build TRDP tests in the submodule

If you want to build the TRDPStack tests:

```sh
cmake -S external/TRDPStackLinux/trdp_3_0_0_0 -B build/trdp -DTRDP_BUILD_TESTS=ON
cmake --build build/trdp
ctest --test-dir build/trdp --output-on-failure
```

## 8) Troubleshooting

- **Headers not found**: verify you linked against `trdp::trdp` (not just `trdp`).
- **Missing MD APIs**: ensure `TRDP_ENABLE_MD=ON` is set before `add_subdirectory`.
- **XML/marshalling symbols missing**: set `TRDP_FULL_BUILD=ON` before `add_subdirectory`.
