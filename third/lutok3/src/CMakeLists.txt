file(GLOB lutok3_src
    "lutok3/*."
    "lutok3/*.h"
    "lutok3/*.hpp"
    "lutok3/*.ipp"
)

################### MAIN #################

find_package(lua 5.3 REQUIRED)

add_library(lutok3 INTERFACE)
target_sources(lutok3 INTERFACE ${lutok3_src})

target_include_directories(lutok3 INTERFACE
    "${lutok3_SOURCE_DIR}/src/lutok3"
    "${LUA_INCLUDE_DIR}"
)

target_link_libraries(lutok3 INTERFACE ${LUA_LIBRARY})

################### INSTALL ##############

#install(
#	EXPORT lutok3
#)
