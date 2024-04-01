# TemplateCoverage


## Introduction

TemplateCoverage analyzes your source code and generates a code coverage report for all executable lines. Unlike llvm-cov, TemplateCoverage includes uninstantiated templates and unused inline methods as well, this way you can get an accurate code coverage for header-only template libraries. Note that TemplateCoverage does not profile your application, it only output a blank coverage report that contains all lines. You have to measure the actual coverage using llvm-cov or something similar, and merge it with the results from TemplateCoverage.


## Installing TemplateCoverage

### Using the binaries

You can download the binaries of TemplateCoverage from [GitHub releases](https://github.com/petiaccja/TemplateCoverage/releases). Keep it mind that you need to manually install the dependencies to your system, which includes the C++ runtimes of your platform and a few common packages. Most likely all is already present on your machine.

### Building from source

TemplateCoverage uses the conan package manager and the CMake build system. Additionally, TemplateCoverage relies on LLVM, which is automatically downloaded and built from the CMake build script. Keep in mind that if you opt to build LLVM, the CMake configuration step can easily take half an hour or more.

First, install your preferred C++20 compiler, then install conan by typing this:

```
pip install conan
```

Afterwards, you can follow the standard conan & CMake procedure:

```
git clone https://github.com/petiaccja/TemplateCoverage.git
cd TemplateCoverage
conan install . --build=missing -pr:h default -pr:b default
cmake -S . --preset conan-debug
cmake --build --preset conan-debug
cmake --install ./build/conan-debug --prefix <target-dir>
```

Note that here I used the `default` profile for conan. You're of course free to use any that you wish, or you can also check out the `.github/workflows/build_profiles` folder that contains the CI's build profiles. You can also use those locally. The appropriate CMake preset (in this case `conan-debug`) is determined by your conan profile.

#### Customizing the LLVM dependency

TemplateCoverage builds its own LLVM dependency if it cannot find any on the system. It's important to note that TemplateCoverage **requires LLVM to be built with RTTI and exceptions**. (This is because TemplateCoverage uses both exceptions and RTTI, and it won't otherwise link to LLVM.)

To force TemplateCoverage to build LLVM instead of using the system's, you can use the `EXTERNAL_LLVM_IGNORE_SYSTEM:BOOL=ON` switch on your CMake configure command. This is useful if your system LLVM was not built with the correct flags, which is the case for most LLVM packages.

If you want to use the system LLVM, or want to use a custom-built LLVM, you can specify the `Clang_DIR` and `LLVM_DIR` environment variables (or CMake variables) to point to the CMake config folder within your LLVM installation This way you can change the build flags for LLVM or change the LLVM version.


## Using TemplateCoverage

### Command line

To analyze `source.cpp` and write the executable lines in `coverage.xml`, you would use the following command:

```
template-coverage --format=sonar-xml --out-file=./coverage.xml source.cpp
```

This will search for the `compile_commands.json` in the parents of `source.cpp`, and infer the compilation commands from there.

In case you want to specify the compilation commands yourself, you can do it like so:

```
template-coverage --format=sonar-xml --out-file=./coverage.xml source.cpp -- clang++ -O3
```

For further options, use the `--help` flag, or check out the reference for clang-check. TemplateCoverage and clang-check are both LLVM-based tools, and they share the same command line interface.

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

If you run into this problem, you need to specify the proper resource directory to template-coverage.

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

The application is about 200 lines of code and is really simple. You should be able to get started right away. There are also only really two things that can be added to it.

### Adding a new output format

Make a new function in `Reported.hpp`, add it to the array of file formats, and write a unit test for it.

### Fixing/improving the AST processing

The AST expressions that are considered executable are defined by the AST matchers in `Collector.cpp`. I put it together in about 2 hours without learning Clang's AST matchers or the Clang AST, so there may be some problems with it. Feel free to issue a fix and a unit test if TemplateCoverage does not give the desired results on your code.


## License

TemplateCoverage is available under the MIT license, and is free for both commercial and non-commercial use.



