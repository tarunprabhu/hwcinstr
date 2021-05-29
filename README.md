# hwcinstr
Clang + LLVM plugin to automatically add PAPI instrumentation to code

# Build

## Requirements

- PAPI
- libyaml
- libopenssl

## Build instructions
It is not recommended to build in the source directory. One suggestion to build
would be:

```
$ git clone git@github.com:tarunprabhu/hwcinstr.git
$ mkdir build
$ cd build
$ cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_INSTALL_PREFIX=/path/to/install /path/to/hwcinstr
```

If clang, libyaml, PAPI etc. are in non-standard paths, CMAKE_PREFIX_PATH may
be set to the appropriate directories

# Usage

hwcc and hwc++ are the drivers that can be found in /path/to/install/bin
They can mostly be used as drop-in replacements of a regular compiler. 
A config file must be specified that contains the names of the functions 
to be instrumented and the hardware counters to collect. If no counters are specified 
in the config file, the cumulative execution time and the number of occurences
will be recorded. At runtime, the output file for the counters must be 
specified using the HWCINSTR environment variable. If the file provided is "-",
the output will be written to stdout. The time is measured in nanoseconds
using C++11's chrono library.

```
$ hwcc --conf /path/to/conf/file <regular compiler arguments>
$ HWCINSTR=- ./a.out
{
  "functions": {
    "9230498223143": {
      "source": "func",
      "Time": 309323,
      "Occrs": 10,
      "Total instructions": 19932
    }
  }
}
```

# Config file

The list of available counters on the current system can be obtained from
PAPI:

$ papi_avail -a

The output of this command may look something like this

```
PAPI_L2_DCM  0x80000002  Yes  Level 2 data cache misses
PAPI_L1_TCM  0x80000006  Yes  Level 1 cache misses
...
... <more counters follow>
```

The counters specified in the config file must be the same as those in the 
first column of the output above without the `PAPI_` prefix

The config file is a YAML file. An example config file can be found in the 
sample directory


# TODO

- Support GCC as well by writing a GCC plugin that does similar things
- Support C++11 function attributes that can be used instead of a config file
- Allow different counters to be captured for each function
- Support counters around arbitrary regions
- Support turning on and off counters so that calls will only be instrumented
  once capture has been explicitly turned on
