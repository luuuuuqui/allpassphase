# AGENTS.md

Guidance for automated coding agents working in this repository.

## Project Overview

This repository contains the source files for **AllPassPhase**, a VST2 audio effect by `enummusic`. The plugin creates phase dispersion by running stereo audio through cascaded all-pass filters. It has no custom GUI; the host provides the interface.

The checkout is intentionally small and does not include Steinberg's VST2 SDK or generated IDE project files.

## Repository Layout

- `AllPassPhase.cpp` / `AllPassPhase.h`: main VST2 effect class, parameters, program state, filter setup, and realtime audio processing.
- `AllPassFilter.cpp` / `AllPassFilter.h`: biquad-style all-pass filter implementation used by the plugin.
- `LRCrossoverFilter.cpp` / `LRCrossoverFilter.h`: Linkwitz-Riley crossover filter code retained in the repo but not currently wired into `AllPassPhase`.
- `HardClip.cpp` / `HardClip.h`: small helper class, currently not used by the main processing path.
- `adelaymain.cpp`: VST2 factory entry point expected by the SDK sample project layout.
- `README.md`: user-facing plugin description plus Windows/macOS build instructions.

## External Dependencies

The source depends on the VST2 SDK header:

```cpp
#include "public.sdk/source/vst2.x/audioeffectx.h"
```

Do not add the VST2 SDK to this repository. The README explains that it cannot be redistributed here. Build and validation usually happen after copying these files into a VST2 SDK sample project.

## Build And Verification

There is no standalone build script, project file, package manifest, or test runner in this checkout.

When changing code:

1. Run lightweight repository checks:

   ```powershell
   git status --short
   rg --files
   ```

2. If the VST2 SDK project is available outside this repo, validate through the host IDE flow described in `README.md`:
   - Windows: Visual Studio project under the copied VST2 `aDelay` sample.
   - macOS: Xcode project under the VST2 sample workspace.

3. If you cannot compile because the SDK is absent, state that clearly in the final response and describe the checks you did run.

## Coding Conventions

- Preserve the existing simple C++ style: headers plus `.cpp` files, tabs in many blocks, and minimal abstraction.
- Keep compatibility with the old VST2 SDK sample environment. Avoid modern build-system assumptions unless the user explicitly asks for them.
- Use ASCII for new text unless editing existing copyright/comment text that already uses another encoding.
- Keep user-facing parameter names stable unless the request is specifically about changing plugin behavior or host-visible labels.
- Be careful with VST string APIs and fixed-size buffers such as `char name[24]`.

## Realtime Audio Notes

`AllPassPhase::processReplacing` runs on the audio thread. Treat it as realtime-sensitive:

- Avoid adding logging, disk I/O, blocking calls, locks, sleeps, or host/UI work in the processing path.
- Avoid new heap allocation inside `processReplacing` when possible. The current code allocates temporary buffers per block; do not make that pattern worse without a clear reason.
- Preserve stereo behavior: left and right channels use separate filter state arrays.
- Preserve dry/wet behavior through `fMix`.
- Keep safeguards around low-frequency modulation and filter state resets. These are there to reduce unstable/noisy behavior at low frequencies.
- Coefficient calculations intentionally use `double` for better stability at low frequencies.

## DSP Behavior To Preserve

- `kFrequency`, `kQ`, `kIterations`, and `kMix` are the active parameters.
- `fIterations` maps to up to 50 cascaded all-pass filters.
- `knobToFrequency` is exponential and host-facing display depends on it.
- `Q` is clamped to a minimum true value of `0.005` to avoid self-oscillation.
- Silence detection reduces idle processing after `deactivateAfterSamples`.

## Git Guidance

- The repository history uses short, descriptive commit messages.
- Commit every repository change made during a task before handing control back to the user, unless the user explicitly asks not to commit.
- Before committing, check `git status --short` and avoid including unrelated user changes.
- A good commit message for documentation-only agent guidance is:

  ```text
  add agent guidance
  ```
