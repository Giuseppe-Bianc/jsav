# jsav

[![ci](https://github.com/Giuseppe-Bianc/jsav/actions/workflows/ci.yml/badge.svg)](https://github.com/Giuseppe-Bianc/jsav/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/Giuseppe-Bianc/jsav/branch/main/graph/badge.svg)](https://codecov.io/gh/Giuseppe-Bianc/jsav)
[![CodeQL](https://github.com/Giuseppe-Bianc/jsav/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/Giuseppe-Bianc/jsav/actions/workflows/codeql-analysis.yml)
[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)

## About jsav

An operating system-independent compiler, written in C++23, designed to process source code files with the .vn extension. The compiler accepts these files as input, parsing their syntax and generating optimized executables compatible with the target operating system. It provides detailed diagnostic messages in case of errors and supports configurable compilation options, enabling both local development and deployment of standalone applications. The design aims for maximum portability and efficiency, ensuring a consistent and reliable compilation process across different platforms.

## Requirements

To run this program smoothly, please ensure the following prerequisites are in place:

- Any C++ compiler is necessary for converting the program code into a runnable format on your system. There's a wide range of C++ compilers to choose from, pick one that suits your needs best.

## More Details

- [Dependency Setup](README_dependencies.md): This section provides a comprehensive guide to setting up the necessary dependencies for Vandior.It walks you through each step of the process, ensuring that you have everything you need to get started.
- [Building Details](README_building.md): Here, you can find detailed instructions on how to build Vandior. Whether you're a seasoned developer or a novice, these instructions will help you get Vandior up and running on your machine.
- [Docker](README_docker.md): This section provides information on how to use Vandior with Docker. If you're looking to containerize your Vandior applications, this guide will be invaluable.

# Usage

```bash
jsav -i input_file.vn [-c] [-r] [-x] [-m]
```

Flags:

- --input (-i): the vn input source file;
