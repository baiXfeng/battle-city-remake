file(GLOB lutok3_test_src
    "*.cpp"
    "*.h"
)

################### MAIN #################

find_package(lua 5.3 REQUIRED)

add_executable(
    lutok3_test
    ${lutok3_test_src}
)

target_link_libraries(
	lutok3_test
	lutok3
)
