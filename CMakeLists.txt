cmake_minimum_required(VERSION 3.22)

# Setup cmake module path and compiler settings
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")
message("Build type: "              ${CMAKE_BUILD_TYPE})
set(CMAKE_C_STANDARD                11)
set(CMAKE_C_STANDARD_REQUIRED       ON)
set(CMAKE_C_EXTENSIONS              ON)
set(CMAKE_CXX_STANDARD              17)
set(CMAKE_CXX_STANDARD_REQUIRED     ON)
set(CMAKE_CXX_EXTENSIONS            ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS	ON)
set(PROJ_PATH                       ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_TOOLCHAIN_FILE            gcc-arm-none-eabi)

project(STM32_HIDBL)
enable_language(C CXX ASM)

set(EXECUTABLE                      ${CMAKE_PROJECT_NAME})

set(SOURCES
Src/main.cpp
)

add_executable(${EXECUTABLE} ${SOURCES})
target_include_directories(${EXECUTABLE} PRIVATE Inc)

set(VARIANT "F401CEUX" CACHE STRING "Variant")
set(VARIANT_DIR "${PROJ_PATH}/Variants/${VARIANT}")
if(EXISTS ${VARIANT_DIR})
    message(STATUS "Configure for variant: ${VARIANT} - ${VARIANT_DIR}")
    include(${VARIANT_DIR}/CMakeLists.txt)

	set(SOURCES
	variant.h
	variant.cpp
	)
	list(TRANSFORM SOURCES PREPEND ${VARIANT_DIR}/)
	target_sources(${EXECUTABLE} PRIVATE ${SOURCES})
	target_include_directories(${EXECUTABLE} PRIVATE ${VARIANT_DIR})
	target_sources(${EXECUTABLE} PRIVATE ${SOURCES})
    set(linker_script_SRC ${VARIANT_DIR}/ldscript.ld)
else()
    message(FATAL_ERROR "Variant dir not found: ${VARIANT_DIR}")
endif()

set(compiler_define ${compiler_define}
    "USE_HAL_DRIVER"
    ${stm32_mcu_define}
	HSE_VALUE=${stm32_hse}
)

if(${stm32_fam_define} STREQUAL "STM32F4")
	add_subdirectory("Core/STM32F4")
elseif(${stm32_fam_define} STREQUAL "STM32H7")
	add_subdirectory("Core/STM32H7")
else()
	message(FATAL_ERROR "Unsupported STM32 family set: ${stm32_fam_define}")
endif()

target_compile_definitions(${EXECUTABLE} PRIVATE ${compiler_define})

target_compile_options(${EXECUTABLE} PRIVATE
	${CPU_PARAMETERS}
	-Wall
#	-Wpedantic
	-Wno-unused-parameter
)

set_property(TARGET ${EXECUTABLE} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
target_link_options(${EXECUTABLE} PRIVATE
	-T${linker_script_SRC}
	${CPU_PARAMETERS}
	-Wl,-Map=${CMAKE_PROJECT_NAME}.map
	--specs=nosys.specs
	#-u _printf_float                # STDIO float formatting support
	-Wl,--start-group
	-lc
	-lm
	-lstdc++
	-lsupc++
	-Wl,--end-group
	-Wl,--print-memory-usage
)

# Execute post-build to print size
add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
	COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${EXECUTABLE}>
)

# Convert output to hex and binary
add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
	COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${EXECUTABLE}> ${EXECUTABLE}.hex
)

# Convert to bin file -> add conditional check?
add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
	COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${EXECUTABLE}> ${EXECUTABLE}.bin
)