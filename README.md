# TRDPStackLinux

## Debian package (CMake + CPack)

From the repository root:

```bash
cmake -S trdp_3_0_0_0 -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=ON
cmake --build build
cpack --config build/CPackConfig.cmake
```

This produces a `.deb` that installs headers under `/usr/include/trdp` and a
CMake package config under `/usr/lib/cmake/TRDPStack`, so consumers can use:

```cmake
find_package(TRDPStack REQUIRED)
target_link_libraries(my_app PRIVATE trdp::trdp)
```
