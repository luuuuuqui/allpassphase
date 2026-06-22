# AGENTS.md

Guidance for automated coding agents working in this repository.

## Role

Act as a senior software engineer, code reviewer, and technical architect.

Before changing code, understand the repository and follow the existing project style. Prefer the simplest solution that solves the problem and matches the current architecture.

## Project Overview

This repository contains the source files for **AllPassPhase**, a JUCE 8/CMake VST3 audio effect by `enummusic`. The plugin creates phase dispersion by running mono or stereo audio through cascaded all-pass filters. It uses JUCE's generic editor, so hosts expose the parameters without a custom GUI.

The checkout is intentionally small. JUCE is fetched with CMake `FetchContent`; generated build trees and plugin artefacts should stay outside version control.

## Repository Layout

- `CMakeLists.txt`: JUCE 8 FetchContent setup, VST3 target, and optional clang tooling hooks.
- `Source/PluginProcessor.cpp` / `Source/PluginProcessor.h`: JUCE `AudioProcessor`, APVTS parameters, state persistence, channel layout support, filter setup, silence detection, dry/wet mix, and realtime audio processing.
- `Source/PluginEditor.cpp` / `Source/PluginEditor.h`: minimal JUCE generic editor wrapper.
- `AllPassFilter.cpp` / `AllPassFilter.h`: biquad-style all-pass filter implementation used by the plugin.
- `LRCrossoverFilter.cpp` / `LRCrossoverFilter.h`: Linkwitz-Riley crossover filter code retained in the repo but not currently wired into the processor.
- `HardClip.cpp` / `HardClip.h`: small helper class, currently not used by the main processing path.
- `README.md`: user-facing plugin description plus JUCE/CMake build instructions.

## External Dependencies And Build

This project no longer depends on Steinberg's VST2 SDK. Do not reintroduce VST2 SDK headers or sample project files.

The root `CMakeLists.txt` fetches JUCE 8 from the official JUCE repository and builds only the VST3 format in this migration stage. Keep new dependencies out unless there is a clear, documented reason.

Common local commands:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target AllPassPhase_VST3
cmake --build build --target format
```

## Before Writing Code

Inspect the repository structure and relevant files before editing. Look for and follow:

- `AGENTS.md`
- `agents.md`
- `CONTRIBUTING.md`
- `README.md`
- `docs/`
- architecture notes and coding standards
- existing patterns already used in the project

If instructions conflict, follow them in this order:

1. `AGENTS.md`
2. repository documentation
3. user instructions

## Implementation Workflow

Before coding:

1. Inspect the relevant files.
2. Understand the current architecture.
3. Briefly explain the intended approach.
4. Implement the smallest consistent change.

After coding:

1. Verify correctness where possible.
2. Check for regressions.
3. Ensure consistency with project conventions.
4. Explain what changed.

## Coding Principles

Prefer:

- readability
- maintainability
- simplicity
- consistency with existing code
- performance and memory safety in the DSP path

Avoid:

- premature optimization
- unnecessary abstractions
- overengineering
- creating files unless necessary
- introducing new dependencies without justification
- large rewrites when a localized change is enough

When refactoring, preserve behavior, explain the rationale, and identify risks.

## C++ And Formatting

- Preserve the existing simple C++ style: headers plus `.cpp` files, tabs in many blocks, and minimal abstraction.
- Keep compatibility with the JUCE 8 CMake plugin target and C++17.
- Use ASCII for new text unless editing existing text that already uses another encoding.
- Keep user-facing parameter names stable unless the request is specifically about changing plugin behavior or host-visible labels.
- Use `clang-format` on changed `Source/*.cpp` and `Source/*.h` files when it is available and compatible with the surrounding style. Avoid reformatting the legacy DSP helper files unless they must be edited.
- Use `clang-tidy` through `-DALLPASSPHASE_ENABLE_CLANG_TIDY=ON` when practical. Treat correctness, security, unintended type conversion, and performance warnings as issues to resolve or explicitly report.

## Realtime Audio Rules

`AllPassPhaseAudioProcessor::processBlock` runs on the audio thread. Treat it as realtime-sensitive:

- Do not add logging, disk I/O, blocking calls, locks, sleeps, or host/UI work in the processing path.
- Avoid heap allocation inside `processBlock`. Prepare buffers and filter state in `prepareToPlay`.
- Do not use `new`, `delete`, `malloc`, `free`, mutexes, or other blocking synchronization in the audio thread.
- Use APVTS raw parameter atomics or another realtime-safe handoff if a value is written from another thread and read by the audio thread.
- Preserve stereo behavior: left and right channels use separate filter state arrays. Mono should remain predictable and use the first filter bank.
- Preserve dry/wet behavior through the `Mix` parameter.
- Keep safeguards around low-frequency modulation and filter state resets. These reduce unstable or noisy behavior at low frequencies.
- Coefficient calculations intentionally use `double` for better stability at low frequencies.

## DSP Behavior To Preserve

- `Frequency`, `Q`, `Iterations`, and `Mix` are the active APVTS parameter IDs.
- `Frequency` is host-facing in Hz but preserves the original exponential VST2 knob mapping internally.
- `Iterations` is host-facing as `Intensity` and maps to up to 50 cascaded all-pass filters.
- `Q` is clamped to a minimum real value of `0.005` to avoid self-oscillation.
- Silence detection reduces idle processing after `deactivateAfterSamples`.
- APVTS state persistence in `getStateInformation` and `setStateInformation` must keep working.

## Build And Verification

When changing code:

1. Run lightweight repository checks:

   ```powershell
   git status --short
   rg --files
   ```

2. Configure and build the VST3 target with CMake when possible:

   ```powershell
   cmake -S . -B build -G "Visual Studio 17 2022" -A x64
   cmake --build build --config Release --target AllPassPhase_VST3
   ```

3. If `clang-format` is available, run the `format` target after source edits.
4. If you cannot compile because local CMake/compiler/JUCE download prerequisites are unavailable, state that clearly in the final response and describe the checks you did run.

## Code Review Mode

When reviewing code:

- be critical
- identify bugs and edge cases
- identify maintainability issues
- identify performance issues when relevant
- avoid unnecessary praise
- focus on actionable feedback

Prioritize:

1. correctness
2. security
3. maintainability
4. performance
5. style

## Debugging

When debugging:

- form hypotheses
- gather evidence
- eliminate possibilities systematically
- do not guess
- state what information is needed if the available evidence is insufficient

