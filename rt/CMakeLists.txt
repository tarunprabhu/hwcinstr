set(SOURCES
  API.cpp
  FunctionStats.cpp
  RTContext.cpp
  RegionStats.cpp
  Stats.cpp
  ../common/Formatting.cpp
  ../common/PAPIContext.cpp
  ../common/SymbolNames.cpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(SYSTEM ${PAPI_INCLUDEDIR})

set(RT HWCInstrRt)
add_library(${RT} SHARED ${SOURCES})
target_link_options(${RT} PUBLIC -rdynamic)
target_link_directories(${RT} PUBLIC ${PAPI_LIBDIR})
target_link_libraries(${RT} ${PAPI_LIBRARIES})
set_target_properties(${RT}
  PROPERTIES
  SHARED_OUTPUT_DIRECTORY ${CMAKE_PROJECT_LIBDIR})
