# TemplateCoverage


## Introduction

TemplateCoverage analyzes your source code and generates a code coverage report listing all executable lines as uncovered. Unlike llvm-cov, TemplateCoverage includes uninstantiated templates and unused inline methods as well, this way you can get an accurate code coverage for header-only template libraries. Note that TemplateCoverage does not profile your application, it generates only a blank coverage report with everything not covered. You have to measure the actual coverage using llvm-cov or similar, and merge it with the results from TemplateCoverage.


## Installing TemplateCoverage

### Get the binary distribution

You can download the binaries of TemplateCoverage from [GitHub releases](https://github.com/petiaccja/TemplateCoverage/releases). Currently, you have to install the dependencies manually, which boils down to the C++ runtime libraries and a few common libraries required by LLVM. Most likely, you already have it all installed.

### Building from source

Requirements:
- CMake
- Python3 & the conan package manager
- A C++20-compatible compiler (MSVC, GCC, Clang)
- Make, ninja, or similar build tool
- LLVM libraries (read below)

Once you have the requirements installed, you can follow the standard conan + CMake procedure:

```
git clone https://github.com/petiaccja/TemplateCoverage.git
cd TemplateCoverage
conan install . --build=missing -pr:h default -pr:b default
cmake -S . --preset conan-debug
cmake --build --preset conan-debug
cmake --install ./build/conan-debug --prefix <target-dir>
```

Note that here I used the `default` profile for conan, but you can use any that you like. You can also check out the `.github/workflows/build_profiles` folder that contains the CI's build profiles, which can also be used locally. The appropriate CMake preset (in this case `conan-debug`) is provided by your conan profile.

#### LLVM libraries

The LLVM libraries are automatically downloaded and compiled when building TemplateCoverage. This is done during the configuration step of CMake, which, as a result, can take 30 minutes or more.

By default, TemplateCoverage looks for the LLVM libraries on your system. You can prevent this behaviour by defining `EXTERNAL_LLVM_IGNORE_SYSTEM:BOOL=ON` during the CMake configuration. In this case, TemplateCoverage will fall back to downloading LLVM. If you use the system's LLVM libraries, make sure that your system LLVM was build with exceptions and RTTI. (This is not the case for most LLVM packages!)

If you want TemplateCoverage to use a specific LLVM installation, you can specify the `Clang_DIR` and `LLVM_DIR` environment variables (or CMake variables) to point to the CMake config folder within your LLVM installation This way you can change the build flags for LLVM or override the LLVM version.

## Uisng TemplateCoverage

### Command line

To analyze `source.cpp` and write the executable lines in `coverage.xml`, invoke `template-coverage` like this:

```
template-coverage --format=sonar-xml --out-file=./coverage.xml source.cpp
```

The compilation flags used to build `source.cpp` should also be specified. You have two options for this.

First, you can specify the compilation flags manually after the `--` seperator on the command line:

```
template-coverage --format=sonar-xml --out-file=./coverage.xml source.cpp -- clang++ -O3
```

Second, you can put a `compile_commands.json` file in any of the parent folders of `source.cpp`. TemplateCoverage will find the compilation DB and search for flags for the analyzed source files. You can use CMake to generate the compilation DB for you.

For more information, use the `--help` flag, or check out the reference for clang-check. TemplateCoverage and clang-check are both LLVM-based tools, and they share the same command line interface.

### Running TemplateCoverage over all files in `compile_commands.json`

You can find the utility named `run_on_compile_commands.py` in the `tools` folder or inside the binary distributions.

To run TemplateCoverage over all files, you can type:

```
python
    run_on_compile_commands.py
    -p
    ./path/to/compile_commands.json template-coverage
    --
    --out-file=./coverage.xml
```

Note that this utility will let you run clang-check or clang-tidy as well, just replace `template-coverage` with the path to the LibTooling-based tool your want to run. (It does not work for clang-format!)

### Compile commands for header files

CMake does not emit compilation commands for header files, however, you can use the [VERIFY_INTERFACE_HEADER_SETS](https://cmake.org/cmake/help/latest/prop_tgt/VERIFY_INTERFACE_HEADER_SETS.html#prop_tgt:VERIFY_INTERFACE_HEADER_SETS) feature. When enabled, CMake generates a `.cpp` file for each header file, which merely includes the original header. These additional generated `.cpp` files do appear in the `compile_commands.json`.

While this extended `compile_commands.json` still does not include commands for headers files, the `run_on_compile_commands.py` script will interpret them as if it were compilation commands for the original header files, giving you coverage for headers too.

### Delayed template parsing (Windows)

On Windows, the MSVC compiler only parses templates at the end of the compilation unit, and only if the templates were instantiated. To allow compatibility with Microsoft-supplied headers, Clang does the same by default. This means that the AST for uninstantiated templates is not available on Windows, therefore TemplateCoverage cannot analyze their AST for executable code. Currently, TemplateCoverage will flag the template function's declaration as an uncovered line, but not the body of the function. This will result in inaccurate coverage information, but at least it still flags uninstantiated template functions.

### <stddef.h> not found

If you run into this problem, you need to specify the resource directory to template-coverage.

You can get the resource directory from clang:

```
clang++ -print-resource-dir
# Prints: /usr/lib/...
```

You can then specify the resource directory when invoking template-coverage:

```
template-coverage -extra-arg=-resource-dir=/usr/lib/... source.cpp
```

This should solve the issue. Try to use the same clang version when getting the resource directory as the one template-coverage was compiled with.


## How it works

TemplateCoverage (like clang-check) is based on LLVM's LibTooling, for which you can look at [LLVM's documentation](https://Clang.llvm.org/docs/LibTooling.html).

In short, what happens is that TemplateCoverage parses your source code into the Clang AST, then analyzes the AST for executable lines of code using Clang's AST matchers. If an executable line of code is found, it is noted down, and later emitted into the resulting coverage report.


## Contributing

The application is about 200 lines of code and is really simple. You should be able to get started right away. There are also really only two things that can be added to it.

### Adding a new output format

Make a new function in `Reported.hpp`, add it to the array of file formats, and write a unit test for it.

### Fixing/improving the AST processing

The AST expressions that are considered executable are defined by the AST matchers in `Collector.cpp`. I put it together in about 2 hours without learning Clang's AST matchers or the Clang AST, so there may be some problems with it. Feel free to issue a fix and a unit test if TemplateCoverage does not give the desired results on your code.


## License

TemplateCoverage is available under the MIT license, and is free for both commercial and non-commercial use.



