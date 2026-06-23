# BlackMist quality-upgrade acceptance checklist

Mark only evidence-backed items complete.

## A. Baseline

- [x] Starting branch and commit recorded.
- [x] Working tree was clean or pre-existing changes were documented.
- [x] Installed UE 5.7/5.8 APIs were inspected.
- [x] Baseline screenshots and GPU timing were recorded, or inability was documented.

## B. Intensity and identity

- [x] `Intensity=0` produces original RGB and alpha through standard zero-pass activation and shader blend logic.
- [x] Standard activation creates zero Black Mist passes at zero intensity.
- [x] Contrast and shadow/veiling changes are included in the intensity blend.
- [ ] Intensity sweep near zero is continuous in a rendered pixel-diff capture.
- [x] Intensity values above one have documented semantics.

## C. Scatter-energy model

- [x] One shared scatter-source definition is used by prefilter and composite.
- [x] `ScatterAmount` affects both halo source and direct removal.
- [x] `CoreLoss` is applied to scattered source, not independently to all highlighted color.
- [x] Kernels and scale weights are normalized.
- [x] Energy implications of `HaloStrength` and tint are documented.

## D. Prefilter

- [x] Each tap is sanitized before averaging.
- [x] Each tap is soft-limited before averaging.
- [x] Each tap is thresholded before averaging.
- [x] Threshold and limiter are pre-exposure stable.
- [ ] Subpixel emitter motion does not visibly flicker.
- [ ] Extreme fireflies do not contaminate the wide halo in a rendered stress scene.

## E. Settings

- [x] Every scalar is finite before range clamping.
- [x] Tint and weight components are finite.
- [x] Invalid values fall back to documented defaults.
- [x] Weights remain non-negative and sum to one.
- [x] New settings are included in Project Settings reset customization.
- [x] Blueprint/API compilation succeeds.
- [x] Migration behavior is documented.

## F. Optical quality

- [x] Diffusion radius is adjustable without adding passes.
- [x] Wide-tail behavior is adjustable and normalized.
- [x] Hybrid scatter metric handles saturated-emitter detection in source.
- [x] Base scatter defaults to zero and is clamped to a small non-negative range.
- [x] Local veiling affects regions with halo rather than grading the entire frame by default.
- [ ] Point-source profile has no hard rings or severe axis artifacts in rendered validation.

## G. Composite and RDG

- [x] Final path remains exactly eight named passes.
- [x] Final composite binds only SceneColor and U1 plus required constants.
- [x] D1-D4 are not bound in normal Final mode.
- [x] Debug output remains in Pass 8 without a ninth pass.
- [x] `CoreLossOnly` displays removed energy or is renamed accurately.
- [ ] Composite filter modes were visually and temporally compared.
- [ ] Default filter was selected using visual and timing evidence.
- [ ] DumpGPU confirms expected extents and resource lifetimes.

## H. Intermediate format

- [x] Mode 0 implements real Auto behavior.
- [x] Auto checks public RHI pixel-format capabilities for texture sampling and render target use.
- [x] RGBA16F fallback is implemented.
- [ ] R11G11B10F quality was compared where used.
- [ ] Selected format is recorded in profiling results.

## I. Rendering correctness

- [x] Scene-linear HDR and pre-tonemap placement remain implemented through the existing callback location.
- [x] Alpha is preserved in the final shader.
- [x] OverrideOutput handling remains implemented.
- [x] `FScreenPassTextureSlice` input handling remains implemented with `CopyFromSlice`.
- [ ] Non-zero and odd ViewRects were rendered and inspected.
- [ ] Dynamic resolution/screen percentage were tested.
- [ ] Split-screen does not bleed between views.
- [ ] SceneCapture behavior follows settings.
- [ ] Path Tracing behavior follows settings in rendered validation.

## J. Builds and tests

- [x] UE 5.7 Editor Development build succeeds.
- [x] UE 5.7 Game Development build succeeds.
- [x] UE 5.7 Shipping build succeeds.
- [x] UE 5.8 Editor Development build succeeds.
- [x] UE 5.8 Game Development build succeeds.
- [x] UE 5.8 Shipping build succeeds.
- [ ] Shader compilation succeeds on the final target RHI outside `-NullRHI`.
- [x] CPU automation tests pass.
- [ ] PIE start/stop lifecycle was exercised repeatedly.
- [ ] Standalone was tested.

## K. Visual and exposure validation

- [ ] Black, gray, and white frames tested.
- [ ] HDR emitter range tested.
- [ ] Saturated color emitters tested.
- [ ] Fixed exposure tested.
- [ ] Auto exposure adaptation tested.
- [ ] Camera cut tested.
- [ ] Bloom-only, Black-Mist-only, and combined captures compared after this upgrade.

## L. Profiling and documentation

- [ ] Per-pass and total GPU timing recorded.
- [ ] Resolution, screen percentage, GPU, driver, RHI, Engine, and format recorded.
- [ ] Baseline and upgraded timings compared.
- [x] README updated.
- [x] Implementation status updated.
- [x] Unverified configurations remain explicitly listed.
- [x] Final report includes changed files and exact commands run.
