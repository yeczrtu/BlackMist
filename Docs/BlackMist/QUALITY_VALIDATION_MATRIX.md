# BlackMist quality validation matrix

Date: 2026-06-23

| Area | Status | Evidence |
| --- | --- | --- |
| UE 5.7.4 Win64 package build | Passed | `Saved\QualityUpgradeValidation\UE5.7_BuildPlugin.log` |
| UE 5.8.0 Win64 package build | Passed | `Saved\QualityUpgradeValidation\UE5.8_BuildPlugin.log` |
| Settings sanitization automation, UE 5.7.4 | Passed | `Saved\QualityUpgradeValidation\UE5.7_Automation_BlackMistSettings_Abs.log`, `Result={Success}` |
| Settings sanitization automation, UE 5.8.0 | Passed | `Saved\QualityUpgradeValidation\UE5.8_Automation_BlackMistSettings_Abs.log`, `Result={Success}` |
| Intensity identity and blend | Source verified | `IsEffectActive()` skips standard zero-intensity activation; shader final path blends original scene color to the unit effect. No pixel-diff capture was run. |
| Per-tap prefilter order | Source verified | `BlackMistPrefilterTap` sanitizes, soft-limits, thresholds, and emits scatter before `BlackMistPrefilter13` averages taps. |
| Shared scatter source | Source verified | `BlackMistScatterSource` is used by prefilter and full-resolution composite core removal. |
| Final-path debug resource lifetime | Source verified | `FBlackMistCompositePS` final parameters omit D1-D4; `FBlackMistDebugCompositePS` owns debug inputs. DumpGPU not run. |
| Intermediate Auto behavior | Source verified and build verified | `GetIntermediateFormat()` checks `RHIPixelFormatHasCapabilities` for `Texture2D`, `TextureSample`, and `RenderTarget`. Runtime selected format not captured. |
| Shader compile on target RHI | Not verified | Automation was run with `-NullRHI`; final project RHI shader compile must be checked in the consuming project. |
| GPU pass count and timings | Not verified | `profilegpu`, `stat gpu`, and `DumpGPU` were not run in this environment. |
| Visual quality and exposure | Not verified | Requires real scenes and screenshot/profile captures. |
| PIE and Standalone lifecycle | Not verified | Requires real consuming project startup and repeated play/stop passes. |
