# Mark3 Low-overhead logger

This repo implements a sophisticated C++-based data-logging framework suitable for use in embedded system running the Mark3 real-time kernel.

It is designed with the following goals in mind:

- Simplicity and familiarity

Facilitate printf-style logging operations using a familiar format-string + argument list format.

- Efficiency:

Avoid the overheads associated with multiple string-parsing and argument-formatting operations.
As much as possible, leverage the preprocessor, compiler, and host-side tools to do these operations.
Avoid use of dynamic memory allocations

- Determinism:

The cost of logging is bound, and is not significantly implacted by the content being logged

- Security

Logs are transmitted in a binary format, and can only be translated into a human-readable format with the aid of data stored in the .elf file.

- Thread and interrupt safety

Log macros can be called from both thread and interrupt contexts without running into race conditions or

## How it works:

The logging framework uses a combination of compiler directorives, preprocessor magic, and knowledge of the .elf file format to allow a target to perform
rich token-based logging at runtime in an efficient, deterministic way.

### Preprocessor magic:

The logger makes use of a sliding-rule variadic macro to provide a single macro capable of logging any number
of arguments.  Other logging frameworks require separate macros based on the number of arguments being logged.
As a result, a single DEBUG_LOG() macro can be used for all logging calls in the target system, regardless of the argument count.

### Logging macros:

The EMIT_DBG_HEADER() macro emits per-file metadata and generates an FNV1a32 hash of the current file, providing file-specific metadata

The DEBUG_LOG() macro is designed to perform two jobs -- one at build time, one at runtime.
- At build-time: The macro stores the format string and metadata sufficient to uniquely identify the log-line to a special non-exectuable section in the .elf file output using compiler tricks involving volatile variable declarations
- At run-time: Interpret the binary data from the macro and write it to the logger, along with metadata sufficiently to uniquely identify the log-line, and the argument formats/data.

### Elf file magic:

- After the executable has been built, the .logger section of the .elf binary can be stripped out and parsed
by a host-side utility.  This data can then be used to reassemble binary logs read from the target into a human-
readable format by a tool communicating with the target device.

## Example Usage:

See /example/main.cpp for an example demonstrating the logger in action.

## Interpreting the logs

The /host directory contains example code and scripts that can be used to parse .logger sections from .elf files to create tools capable of interpreting log streams from a target.
