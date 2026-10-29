# STM32 PWM Driver

A reusable, register-level PWM driver for the STM32F401RE microcontroller, developed in Embedded C.

## Project Goal

The goal of this project is to develop a configurable PWM driver using direct register access without relying on STM32 HAL for PWM configuration.

The project follows a requirements-driven development approach, including software design, implementation, version control and hardware verification.

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
- Embedded C
- Git / GitHub
- PulseView

## Documentation

See [Software Requirements](requirements.md) for the complete software requirements specification.

## Project Status

**Phase:** Software design preparation

- [x] Requirements engineering
- [x] STM32CubeIDE project created
- [x] Local Git repository initialized
- [ ] First Git commit
- [ ] Software architecture
- [ ] PWM driver implementation
- [ ] Hardware verification

## License

Not yet specified.