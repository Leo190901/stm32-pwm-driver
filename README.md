# STM32 PWM Driver

A reusable, register-level PWM driver for the STM32F401RE microcontroller, developed in Embedded C.

## Project Goal

The goal of this project is to develop a configurable PWM driver using direct register access without relying on STM32 HAL for PWM configuration.

The project follows a requirements-driven development approach, including software design, implementation, version control and hardware verification.

## Built with an AI Agent, on Purpose

This project is intentionally developed together with [Claude Code](https://claude.com/claude-code), Anthropic's agentic coding tool. It has two learning goals:

1. **Embedded development:** STM32 peripherals, timers, GPIO alternate functions and register-level programming against the RM0368 reference manual.
2. **Agentic coding:** how an AI agent plans, reads code, runs tools, builds and verifies, and how to steer it so that I still understand every line.

How the collaboration works:

- **[`CLAUDE.md`](CLAUDE.md)** gives the agent the project rules: no HAL for PWM, validate parameters before touching registers, no `float` (the FPU is not enabled), and a *learning mode*. In learning mode the agent explains every register and bit it touches, points out embedded pitfalls, and works in small steps.
- **[`STATUS.md`](STATUS.md)** is the handover document between sessions: the current step, a work log with decisions and host-checked arithmetic, open items and next steps. Each session starts by reading it.
- **Requirements first:** code comments reference IDs from [`requirements.md`](requirements.md) (e.g. `REQ-014`), so every change can be traced back to a requirement.
- **Human contributions:** for meaningful design decisions, the agent leaves a `TODO(human)` in the code and I implement that part myself.
- **Hardware verification stays with me:** the agent can build and analyze the firmware in a container, but the checks on the Nucleo board (LED, PulseView, debugger) are mine.
- Commits made with the agent carry a `Co-Authored-By: Claude` trailer, so the git history shows what was written together.

## Planned Features

- Support for TIM1, TIM2, TIM3 and TIM4
- Four PWM channels per timer
- Independent PWM frequencies for different timers
- Configurable PWM frequency from 1 Hz to 100 kHz
- Duty cycle adjustment from 0% to 100%
- Synchronized PWM updates
- Individual channel start and stop
- Error handling for invalid parameters

## Hardware

- STM32 NUCLEO-F401RE
- USB logic analyzer
- Breadboard, LEDs and resistors

## Development Tools

- STM32CubeIDE
- CMake + GNU Arm Embedded toolchain (for builds outside the IDE)
- Embedded C
- Git / GitHub
- PulseView
- Claude Code (AI coding agent)

## Building

In STM32CubeIDE, open the project and build as usual. Outside the IDE:

```bash
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake   # once
cmake --build build
```

## Documentation

- [Software Requirements](requirements.md): the complete software requirements specification
- [Project Status](STATUS.md): current step, work log and next steps
- [CLAUDE.md](CLAUDE.md): instructions and constraints for the AI agent

## Project Status

**Phase:** Driver implementation, step 1 (first PWM output on TIM2_CH1 / PA5)

- [x] Requirements engineering
- [x] STM32CubeIDE project created
- [x] Git repository and first commits
- [x] Driver API (`Inc/pwm.h`)
- [x] Timer init, channel init and start for TIM2_CH1 (PA5)
- [ ] Demo application: 1 Hz / 50 % on LED LD2
- [ ] Hardware verification
- [ ] Stop, duty/frequency update during operation
- [ ] All channels of TIM1–TIM4

See [STATUS.md](STATUS.md) for details.

## License

Not yet specified.