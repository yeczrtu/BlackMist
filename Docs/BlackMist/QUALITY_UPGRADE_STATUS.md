# BlackMist quality upgrade status

Date: 2026-06-23

Task package:

```text
C:\Users\t-hoshino\Downloads\BlackMist_Codex_Quality_Upgrade_Package\BlackMist_Codex_Quality_Upgrade_Package
```

Starting revision:

```text
3e998a4f08cd6cfde614e603b6163fccde790a2b
```

Package baseline reference:

```text
d68ce6b26d961f5cd7dd519bc1bfd8daff78a95f
```

Initial state: `main...origin/main`, clean working tree before implementation.

## Implemented

- Continuous `Intensity` semantics with exact identity through standard zero-intensity activation and full-effect blending for `0..1`.
- Shared scatter source for halo generation and direct core removal.
- Per-tap prefilter sanitize, soft radiance limit, threshold mask, and scatter conversion before averaging.
- `CoreLossOnly` debug output now shows removed radiance.
- New controls: `DiffusionRadius`, `WideTail`, `BaseScatter`, `ScatterMetric`, `ChromaSensitivity`, and `LocalVeilingStrength`.
- Finite settings sanitization for scalars, tint, and scale weights.
- Derived normalized scale weights from `WideTail`, while preserving normalized custom `ScaleWeights`.
- Normal final composite no longer binds D1-D4 debug inputs.
- Debug composite remains pass 8 and binds D1-D4 only when a debug mode is active.
- `r.BlackMist.CompositeFilter` added for one-sample, four-tap, and nine-tap full-resolution halo reconstruction.
- `r.BlackMist.IntermediateFormat=0` now performs real Auto selection using public RHI pixel-format capabilities.
- `Plugins.BlackMist.Settings.Sanitization` automation test added.

## Build Evidence

UE 5.7.4:

- `RunUAT BuildPlugin` succeeded for Win64 `UnrealEditor Development`, `UnrealGame Development`, and `UnrealGame Shipping`.
- Log: `C:\Users\t-hoshino\Documents\BlackMist\Saved\QualityUpgradeValidation\UE5.7_BuildPlugin.log`.

UE 5.8.0:

- `RunUAT BuildPlugin` succeeded for Win64 `UnrealEditor Development`, `UnrealGame Development`, and `UnrealGame Shipping`.
- Log: `C:\Users\t-hoshino\Documents\BlackMist\Saved\QualityUpgradeValidation\UE5.8_BuildPlugin.log`.

Automation:

- UE 5.7.4 `Plugins.BlackMist.Settings.Sanitization` passed with `Result={Success}`.
- Log: `C:\Users\t-hoshino\Documents\BlackMist\Saved\QualityUpgradeValidation\UE5.7_Automation_BlackMistSettings_Abs.log`.
- UE 5.8.0 `Plugins.BlackMist.Settings.Sanitization` passed with `Result={Success}`.
- Log: `C:\Users\t-hoshino\Documents\BlackMist\Saved\QualityUpgradeValidation\UE5.8_Automation_BlackMistSettings_Abs.log`.

## Not Verified Here

- Real viewport visual quality, including point-source profile, saturated emitters, and camera motion.
- Target-RHI shader compilation outside the `-NullRHI` automation run.
- `profilegpu`, `stat gpu`, or `DumpGPU` pass timing and resource lifetime evidence.
- PIE, Standalone, MRQ, SceneCapture, split-screen, dynamic resolution, TSR/AA matrix, and tiled rendering.

These require the real consuming UE project, map, RHI, and GPU capture setup.
