# libdot_cpp11 production package

A fuller C++11 implementation of the public `libdot` parser shape and the DOT format described by the upstream project.

## What this package adds

- production-oriented C++11 library layout
- installable static library and headers
- C compatibility shim for the public parser-style API
- file and stream parsing with LOB payload ingestion
- serializer with LOB round-trip support
- tests for tree parsing, selectors, duplicate attributes, escapes, and BLOB payloads
- example CLI
- CMake and Makefile-based builds

## Build

```bash
make
make test
sudo make install
```

Or with plain CMake:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

## Example DOT

```text
.html
..body @:body1 class:main #:ram,ddr4
...h1 .:Hello_World
```

## LOB payloads

LOBs are described by an operation line and then followed by exactly the declared byte count of payload data, followed by a newline.

```text
.lob @:lob1
@ lob1
# <:blob1,blob,application/octet-stream,6
ABCDEF
```

Use `parse_stream()` or `parse_file()` for documents containing BLOB/CLOB payloads.

## Scope notes

This package follows the published design and README model where edits are expressed by feeding DOT lines back into the parser. The upstream README explicitly says there is no separate mutation API and operations are intended to happen through `parse line function`, while the design document describes revision 4 marker selectors, duplicate attributes, and BLOB/CLOB support. citeturn270539view1turn270539view0

## License

BSD-3-Clause.
