# uC/OS-II 8051 Port Copilot Instructions

This codebase is a port of the uC/OS-II real-time operating system (RTOS) for the 8051 microcontroller architecture.

## Architecture & Structure

- **Core Kernel (`source/`)**: Contains the platform-independent kernel code (`OS_CORE.C`, `OS_TASK.C`, etc.).
- **Port Layer (`port/`)**: Contains the 8051-specific hardware abstraction layer.
  - `OS_CPU.H`: Processor-specific data types and macros.
  - `OS_CPU_A.ASM`: Assembly language functions (context switching).
  - `OS_CPU_C.C`: C functions for stack initialization and hooks.
- **User Application (`user/`)**: Contains application-specific code and configuration.
  - `TEST.C`: Main entry point and task definitions.
  - `OS_CFG.H`: Kernel configuration (feature enable/disable).

## Critical Development Patterns

### 1. Task Definition
Tasks must be defined with the `RUNSISI_LARGE_REENTRANT` macro to ensure proper reentrancy handling on the 8051 architecture (likely targeting Keil C51).

```c
void TaskName(void *pdata) RUNSISI_LARGE_REENTRANT
{
    // Task initialization
    for (;;) {
        // Task body
        OSTimeDlyHMSM(0, 0, 1, 0); // Yield CPU
    }
}
```

### 2. Memory Management
- **Stack Allocation**: Stacks are allocated in `xdata` memory.
  ```c
  OS_STK xdata TaskStk[TASK_STK_SIZE];
  ```
- **Data Types**: Use uC/OS-II specific types for portability:
  - `INT8U` (unsigned 8-bit)
  - `INT16U` (unsigned 16-bit)
  - `OS_STK` (stack entry)

### 3. Critical Sections
Always use the provided macros for critical sections. Do not manipulate interrupt flags directly.

```c
OS_ENTER_CRITICAL();
// Critical code
OS_EXIT_CRITICAL();
```

### 4. Inter-Task Communication
Use kernel objects (Semaphores, Mailboxes, Queues) defined in `source/`.
- Check `OS_CFG.H` to ensure the mechanism is enabled (e.g., `OS_SEM_EN`).

## 8051 Specifics
- **Memory Model**: The project uses the **Large** memory model.
- **Interrupts**: `EA` bit is used for global interrupt control in `OS_CPU.H`.
- **Timer**: Timer 0 is typically used as the system tick source (`InitTimer0` in `TEST.C`).

## Build & Debug
- This project likely targets the Keil C51 compiler.
- `STARTUP.A51` is the startup code.
- Ensure `includes.h` is included in all source files to pull in necessary definitions.
