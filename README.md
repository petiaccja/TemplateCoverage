# TemplateCoverage

A simple tool for including inline and template code in your code coverage reports.

When using instrumentation to collect code coverage, such as LLVM source-based coverage, uninstantiated templates or unused inline methods won't end up on your code coverage reports while they should actually be uncovered lines. TemplateCoverage uses Clang to parse your source and header files, and analyzes the AST to gather every executable line in your code, templated or not. You can then write out a code coverage report where all lines are uncovered, and merge that with your real report.

## Installing TemplateCoverage

### Grab the binaries from GitHub releases

The binaries of TemplateCoverage are published via GitHub releases. That's probably the easiest way to grab them, but you will have to install dependencies yourself. The dependencies are essentially only the compiler runtime (e.g. MSVC RT) and some very common libraries that LLVM uses.

### Building from source

You can clone this repo and build it from source:

```
cd TemplateCoverage
pip install conan
conan install . --build=missing -pr:h default -pr:b default
cmake -S . --preset conan-debug
cmake --build --preset conan-debug
cmake --install ./build/conan-debug --prefix <target-dir>
```

The build uses the conan package manager, therefore you have to install that and configure it. If you don't want to use the `default` conan build profile, you can find the CI profiles in the `.github/workflows/build_profiles` folder.

After settings up conan and install the dependencies, you can build with CMake using the generated presets, in this example, `conan-debug`.

Note that TemplateCoverage requires the LLVM libraries for building. The LLVM libraries can either be installed on your system, or be automatically downloaded and compiler by TemplateCoverage. In case you are using automatic compilation, the CMake configuration will take very long as it's building LLVM. You can set `EXTERNAL_LLVM_IGNORE_SYSTEM:BOOL=ON` when configuring with CMake to force TemplateCoverage to compile its own LLVM. Alternatively, you can compile or download LLVM yourself and specify the path to LLVM by setting the `Clang_DIR` and `LLVM_DIR` environment variables prior to CMake configuration. With the second method, you can also compile using any compatible LLVM version.


## Using TemplateCoverage

### Command line

TemplateCoverage is a Clang-based tool, and has an interface analogous to Clang-check or Clang-tidy. You can check out the documentation for those to get a general idea.

To give you an example, you can write the coverage report for a file like this:
```
template-coverage --format=sonar-xml --out-file=./coverage.xml source.cpp
```

For a full list of options, type `template-coverage --help`.

### Compile commands

Like all Clang tools, TemplateCoverage can also use the usual `compile_commands.json` file. By default, Clang tools search for this file in the parent directories of the source file that you specified. Alternatively, you can specify the folder in which your `compile_commands.json` file resides with the `-p` option.

In case you don't have a `compile_commands.json` file, you can specify the options that you use to compile your source file after the `--` argument:

```
template-coverage source.cpp -- Clang++ -O3 -o source.o source.cpp
```

### Running TemplateCoverage over all files in `compile_commands.json`

For this, there is a small utility in the `tools` folder named `run_on_compile_commands.py`. This script parses the `compile_commands.json` file, preprocesses it, and executes TemplateCoverage over all files in the `compile_commands.json`. You will get a single coverage report containing executable lines for all files.

Note that this script is generic, meaning it can run any Clang tool (i.e. Clang-check) over your compilation database, not just TemplateCoverage. As such, you have to pass the path to `template-coverage` to the script, or, if you want, just call it with Clang-check.

### Compile commands for header files

CMake does not emit compilation commands for header files, however, there is the [VERIFY_INTERFACE_HEADER_SETS](https://cmake.org/cmake/help/latest/prop_tgt/VERIFY_INTERFACE_HEADER_SETS.html#prop_tgt:VERIFY_INTERFACE_HEADER_SETS) feature. When enabled, CMake generates a `.cpp` file for each header file, which merely includes the original header. These additional generated `.cpp` files do appear in the `compile_commands.json`.

While this extended `compile_commands.json` still does not include commands for headers files, the `run_on_compile_commands.py` script will interpret them as if it were compilation commands for the original header files, giving you coverage for headers too.

### Delayed template parsing (Windows)

On Windows, the MSVC compiler only parses templates at the end of the compilation unit, and only if the templates were instantiated. To allow compatibility with Microsoft-supplied headers, Clang does the same by default. This means that the AST for uninstantiated templates is not available on Windows, therefore TemplateCoverage cannot analyze their AST for executable code. Currently, TemplateCoverage will flag the template function's declaration as an uncovered line, but not the body of the function. This will result in inaccurate coverage information, but at least it still flags uninstantiated template functions.

## How it works

TemplateCoverage is a Clang-based tool, so feel free to check out [LLVM's documentation](https://Clang.llvm.org/docs/LibTooling.html).

In short, you pass the source files and the compilation flags (either via `--` arguments or `compile_commands.json`), and the tool parses your source files using Clang. The resulting Clang AST is then analyzed. TemplateCoverage looks for executable code in the AST, and notes down the file and line number of each in order to create the coverage report. No code is actually compiled or run, only the parsing step is executed.

## Contributing

The entire application is about 200 lines of code, so there is not much to explain.

### Adding a new output format

Make a new function in `Reported.hpp`, add it to the array of file formats, and write a unit test for it.

### Improving the AST processing

The AST expressions that are considered executable are defined by the AST matchers in `Collector.cpp`. Since I hacked it together in 2 hours without even bothering to learn Clang AST matchers, it's probably not perfect. If you find anything odd in there, feel free to submit a fix.

