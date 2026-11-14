cmake_minimum_required(VERSION 3.20)

project(stm32-pwm-driver C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)   # gnu11 wie in STM32CubeIDE
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# MCU-Optionen (identisch zu den STM32CubeIDE-Einstellungen)
set(MCU_FLAGS
  -mcpu=cortex-m4
  -mthumb
  -mfpu=fpv4-sp-d16
  -mfloat-abi=hard
)

add_executable(${PROJECT_NAME}
  Src/main.c
  Src/pwm.c
  Src/syscalls.c
  Src/sysmem.c
  Startup/startup_stm32f401retx.s
)
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".elf")

target_include_directories(${PROJECT_NAME} PRIVATE
  Inc
  Drivers/CMSIS/Include
  Drivers/CMSIS/Device/ST/STM32F4xx/Include
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  STM32 STM32F4 STM32F401xE STM32F401RETx NUCLEO_F401RE
  $<$<CONFIG:Debug>:DEBUG>
)

target_compile_options(${PROJECT_NAME} PRIVATE
  ${MCU_FLAGS}
  --specs=nano.specs
  -ffunction-sections
  -fdata-sections
  $<$<COMPILE_LANGUAGE:C>:-Wall>
  $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
  $<$<CONFIG:Debug>:-O0 -g3>
  $<$<CONFIG:Release>:-Os>
)

target_link_options(${PROJECT_NAME} PRIVATE
  ${MCU_FLAGS}
  --specs=nano.specs
  --specs=nosys.specs
  -T${CMAKE_SOURCE_DIR}/STM32F401RETX_FLASH.ld
  -Wl,--gc-sections
  -Wl,-Map=${PROJECT_NAME}.map
  -Wl,--print-memory-usage
)

# Nach dem Linken: .hex/.bin erzeugen und Größe ausgeben
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin
  COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
)
