[![Build Status](https://travis-ci.com/DaanDeMeyer/bnl.svg?branch=master)](https://travis-ci.com/DaanDeMeyer/bnl)
[![Build status](https://ci.appveyor.com/api/projects/status/xui0l32td6w9h6mt/branch/master?svg=true)](https://ci.appveyor.com/project/DaanDeMeyer/bnl/branch/master)
[![Build Status](https://dev.azure.com/daanjdemeyer/bnl/_apis/build/status/DaanDeMeyer.bnl?branchName=master)](https://dev.azure.com/daanjdemeyer/bnl/_build/latest?definitionId=3&branchName=master)

# What is bnl?

bnl (Binary Networking Library) consists of a WIP HTTP/3 implementation and
(soon) an easy to use wrapper around
[ngtcp2](https://github.com/ngtcp2/ngtcp2)(a QUIC implementation).

All protocols implemented under bnl work on buffers of binary data and as such,
data can be transported between two endpoints in any way the user sees fit. One
immediate advantage of this approach is the possibility to verify application
layer protocols such as HTTP/3 in-memory before moving on to test the
implementation using its associated transport protocol (in the case of HTTP/3:
QUIC). Of course, eventually the goal is to provide common transports and other
required OS integrations as part of bnl (first and foremost being an UDP
transport to facililate transporting QUIC packets over the Internet).

## Build instructions

bnl uses the CMake build system. At least version 3.13 is required. All other
dependencies are automatically downloaded when running CMake for the first time.

```sh
git clone https://github.com/DaanDeMeyer/bnl.git
cmake -B build -S bnl -DBNL_TEST=ON
build/test # Run tests
```

## CMake options

### Developer

- `BNL_SANITIZERS`: Build with sanitizers (default: `OFF`).
- `BNL_TIDY`: Run clang-tidy when building (default: `OFF`).
- `BNL_WARNINGS_AS_ERRORS`: Add -Werror or equivalent to the compile flags and
  clang-tidy (default: `OFF`).
