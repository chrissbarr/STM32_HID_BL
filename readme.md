## Building

```
mkdir build
cd build
cmake .. --preset Release -D VARIANT="F446VETX"
cmake --build Release
```

## Debugging
- Build in debug
- Flash to board
- Start OpenOCD with `openocd -f ..\scripts\interface/stlink.cfg -f ..\scripts\target\stm32f4x.cfg`
- Start GDB with `arm-none-eabi-gdb .\STM32_HIDBL.elf`
- Connect GDB to OpenOCD with `target extended-remote localhost:3333`