set(variant_dir Variants/F446VETX/)

set(stm32_mcu_define "STM32F446xx")

set(SOURCES
variant.h
variant.c
)
list(TRANSFORM SOURCES PREPEND "${variant_dir}/")

target_sources(${EXECUTABLE} PRIVATE ${SOURCES})

target_include_directories(${EXECUTABLE} PRIVATE ${variant_dir})