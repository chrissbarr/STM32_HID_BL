set(variant_dir Variants/F407VETX/)

set(stm32_mcu_define "STM32F407xx")

set(SOURCES
variant.h
variant.c
)
list(TRANSFORM SOURCES PREPEND "${variant_dir}/")

target_sources(${EXECUTABLE} PRIVATE ${SOURCES})

target_include_directories(${EXECUTABLE} PRIVATE ${variant_dir})