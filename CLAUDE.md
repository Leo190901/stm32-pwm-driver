# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Register-level PWM driver for the STM32F401RE (NUCLEO-F401RE), written in C against CMSIS headers. Development is requirements-driven: `requirements.md` defines REQ-xxx / CON-xxx / TC-xxx IDs, and code comments reference them (e.g. `REQ-014`). `STATUS.md` holds the current step, work log, open items and next steps — read it first and keep it updated when finishing a step.

## Build

Two parallel build systems:
- **STM32CubeIDE** (`.cproject`, `Debug/makefile`): the generated makefile uses absolute Windows paths and does not work in the container.
- **CMake** (use this in the container; same flags, defines and linker script as the IDE):

```bash
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake   # once
cmake --build build
```

Output: `build/stm32-pwm-driver.elf/.hex/.bin`, plus a memory-usage report. New source files must be added to `add_executable` in `CMakeLists.txt` (the IDE picks them up automatically). Container toolchain is GCC 12.2; CubeIDE uses GCC 14.3.

There is no test framework or linter. Verification is done on hardware (LD2 / PulseView logic analyzer) per the test cases in `requirements.md`; arithmetic such as PSC/ARR values is checked on the host and recorded in `STATUS.md`.

## Architecture & constraints

- Driver: `Inc/pwm.h` (public API, constants, `PWM_Status_t`) and `Src/pwm.c`. `Src/main.c` is the demo application. `Drivers/` is vendor CMSIS/HAL code — do not edit.
- **No HAL for PWM** (CON-001/002): access registers directly via CMSIS (`TIMx->...`, `RCC->...`) and use CMSIS bit-name macros (`RCC_APB1ENR_TIM2EN`, `TIM_CR1_ARPE`), not magic numbers.
- **Validate before touching registers** (REQ-014): every API function checks all parameters and returns an error status without modifying any register on failure.
- API flow: `PWM_InitTimer` (clock enable, PSC/ARR, ARPE, `EGR.UG`; counter *not* started) → `PWM_InitChannel` (GPIO AF + output-compare setup) → `PWM_Start` (`CCER.CCxE`, then `CR1.CEN`).
- Supported timers: TIM1–TIM4. TIM1 is on APB2 and is an advanced timer (needs `BDTR.MOE` for outputs); TIM2–TIM4 are on APB1. Only valid timer/channel/AF pin mappings may be used (CON-004).

## Clock and FPU (non-obvious)

- No clock configuration runs at startup and `SystemInit` is not defined (the weak call is linked as a no-op), so the MCU runs at reset defaults: HSI 16 MHz, all prescalers /1 → timer clock is **16 MHz** (`PWM_TIMER_CLOCK_HZ` in `pwm.c`). Update that constant if the clock tree changes.
- The build uses `-mfloat-abi=hard`, but nothing enables the FPU (`SCB->CPACR`), so any floating-point instruction faults. **Do not use `float`/`double`**: duty cycle is an integer `duty_permille` (0–1000). If floats ever become necessary, implement `SystemInit` with the FPU enable first.
- The `#warning "FPU is not initialized"` in `main.c` is from the CubeIDE template and expected.

## Repo notes

- `.settings/language.settings.xml` has intentional local IDE changes — do not commit it.
- Commits follow Conventional Commits (`feat:`, `chore:`, `docs:`).

## Learning mode

This is a learning project: the user wants to learn embedded development (STM32, peripherals, register-level programming) and understand how Claude works as an agent.

- Before each step, briefly say what you are about to do and why (which file, which tool, what you expect to find).
- When writing or changing code, explain the key lines: what each register/bit does and why these values were chosen (e.g. prescaler, period, clock source). Reference the relevant RM0368 (reference manual) / STM32F401 datasheet section when possible. HAL may be mentioned for comparison, but the driver itself stays register-level (CON-002).
- Point out common embedded pitfalls related to the code (`volatile`, ISR timing, blocking calls, race conditions, read-modify-write on shared registers).
- After finishing a task, give a short summary: what changed, what the user should understand from it, and one thing they could try themselves to deepen their understanding.
- Prefer small steps over big changes, so the user can follow along.
- If the user asks "why", explain in depth rather than just fixing things.
