set(SOURCES
Src/system_stm32f4xx.c
Src/stm32f4xx_hal_msp.c
Src/stm32f4xx_it.c
Src/syscalls.c
Inc/stm32f4xx_it.h
Inc/stm32f4xx_hal_conf.h
)
target_sources(${EXECUTABLE} PRIVATE ${SOURCES})
target_include_directories(${EXECUTABLE} PRIVATE Inc)