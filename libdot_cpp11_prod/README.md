# libdot_cpp11 production package

A fuller C++11 implementation of the public `libdot` parser shape and the DOT format described by the upstream project, updated to use semicolon-terminated attribute values instead of underscore-for-space encoding.

## What this package adds

- production-oriented C++11 library layout
- installable static library and headers
- C compatibility shim for the public parser-style API
- file and stream parsing with LOB payload ingestion
- serializer with LOB round-trip support
- tests for tree parsing, selectors, duplicate attributes, semicolon escaping, multiline text appends, and BLOB payloads
- example CLI
- CMake and Makefile-based builds
- updated `DESIGN.txt` describing the semicolon-terminated value format

## Updated text/value syntax

Attribute values now keep spaces verbatim.
A value ends when an unescaped semicolon is encountered.
To include a literal semicolon inside a value, escape it as `\;`.
To include a literal backslash, escape it as `\\`.

Example:

```text
.html
..body .:This is a text data and it is supposed to
...+ .:be multiline\; so, that it can be easily read.;
```

This produces one combined text attribute on `body`:

```text
This is a text data and it is supposed tobe multiline; so, that it can be easily read.
```

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
..body @:body1; class:main; #:ram,ddr4;
...h1 .:Hello World;
```

## LOB payloads

LOBs are described by an operation line and then followed by exactly the declared byte count of payload data, followed by a newline.

```text
.lob @:lob1;
@ lob1
# <:blob1,blob,application/octet-stream,6
ABCDEF
```

Use `parse_stream()` or `parse_file()` for documents containing BLOB/CLOB payloads.

## License

BSD-3-Clause.
