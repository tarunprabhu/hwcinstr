set(SOURCES
  ClangPlugin.cpp
  ConvertConstants.cpp
  GenerateSymbolsPass.cpp
  GenerateWrappersPass.cpp
  CFEContext.cpp
  ../common/Conf.cpp
  ../common/Formatting.cpp
  ../common/PAPIContext.cpp
  ../common/SymbolNames.cpp
)

add_definitions(${LLVM_DEFINITIONS} ${CLANG_DEFINITIONS})
# When using a custom LLVM, it may not be installed into a system directory
# Treat it as it is else it may end up causing additional warnings
include_directories(SYSTEM
  ${LLVM_INCLUDE_DIR}
  ${CLANG_INCLUDE_DIRS}
  ${OPENSSL_INCLUDEDIR}
  ${LIBYAML_INCLUDEDIR}
  ${PAPI_INCLUDEDIR})

set(CFE HWCInstrClangPlugin)
add_library(${CFE} SHARED ${SOURCES})
target_link_options(${CFE} PUBLIC -rdynamic)
target_link_directories(${CFE} PUBLIC
  ${LLVM_LIBRARY_DIR}
  ${OPENSSL_LIBDIR}
  ${LIBYAML_LIBDIR}
  ${PAPI_LIBDIR})
target_link_libraries(${CFE}
  ${OPENSSL_LIBRARIES}
  ${LIBYAML_LIBRARIES}
  ${PAPI_LIBRARIES})
set_target_properties(${CFE}
  PROPERTIES
  SHARED_OUTPUT_DIRECTORY ${CMAKE_PROJECT_LIBDIR})
