# Black Mist implementation status

## Repository and Engine reconnaissance

- Workspace inspected: `C:\Users\t-hoshino\Documents\BlackMist`.
- No `.uproject` file was present in this repository at implementation time.
- No existing `.uplugin` or source module was present at implementation time, so the implementation created a new Runtime plugin.
- For GitHub publication, the repository was normalized into a plugin-root layout with `BlackMist.uplugin`, `Source/BlackMistRuntime`, and `Shaders/Private` at the repository root.
- Target Engine was taken from `D:\Unreal\UE_5.7\Engine\Build\Build.version`:
  - UE 5.7.4
  - Changelist 51494982
  - Branch `++UE5+Release-5.7`

## UE 5.7 API notes

Headers inspected:

- `Engine/Source/Runtime/Engine/Public/SceneViewExtension.h`
- `Engine/Source/Runtime/Renderer/Public/PostProcess/PostProcessMaterialInputs.h`
- `Engine/Source/Runtime/Renderer/Public/ScreenPass.h`
- `Engine/Source/Runtime/RenderCore/Public/RenderGraphBuilder.h`

Key 5.7 signatures and behavior:

- Active callback overload:
  `SubscribeToPostProcessingPass(EPostProcessingPass Pass, const FSceneView& InView, FPostProcessingPassDelegateArray& InOutPassCallbacks, bool bIsPassEnabled)`.
- Deprecated overload:
  `SubscribeToPostProcessingPass(EPostProcessingPass Pass, FPostProcessingPassDelegateArray& InOutPassCallbacks, bool bIsPassEnabled)` is deprecated since 5.5 and not used.
- Post callback delegate:
  `FScreenPassTexture(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessMaterialInputs& Inputs)`.
- Scene color input is an `FScreenPassTextureSlice`; active effect converts it with `FScreenPassTexture::CopyFromSlice(GraphBuilder, SceneColorSlice)` without consuming `OverrideOutput`.
- Disabled/race passthrough uses `FPostProcessMaterialInputs::ReturnUntouchedSceneColorForPostProcessing(GraphBuilder)`.
- `FScreenPassTexture::CopyFromSlice` handles Texture2DArray slices and can copy into override output for passthrough.
- `FScreenPassTextureInput`, `FScreenTransform`, and `FScreenPassTextureViewportParameters` are used for ViewRect-aware sampling.
- Pre-exposure is not read from private `FViewInfo`; shaders bind `FViewUniformShaderParameters` and use `View.PreExposure`.

Existing Engine examples inspected:

- `Renderer/Private/Skinning/SkinnedMeshDebugView.cpp` for View-aware post callback registration and `OverrideOutput` handling.
- `Renderer/Private/PostProcess/PostProcessing.cpp` for callback ordering, pass translation, and `BL_SceneColorBeforeBloom` mapping.
- `Renderer/Private/PostProcess/PostProcessMaterial.cpp` for `ReturnUntouchedSceneColorForPostProcessing`.
- `Renderer/Private/ScreenPass.cpp` for `CopyFromSlice`.

## Integration decision

- View extension base: `FWorldSceneViewExtension`.
- Creation owner: `UBlackMistSubsystem : UWorldSubsystem`.
- Creation API: `FSceneViewExtensions::NewExtension<FBlackMistSceneViewExtension>(World)`.
- Default callback location: `ISceneViewExtension::EPostProcessingPass::MotionBlur`.
- Reason: in UE 5.7 this maps to the post sequence slot immediately before `BL_SceneColorBeforeBloom`, before tonemapping and native bloom. `AddAfterPassForSceneColorSlice(EPass::MotionBlur, SceneColorSlice)` is called after the motion blur section even when the actual motion blur pass is disabled, so Black Mist can still run with motion blur off.
- Development override: `r.BlackMist.ForcePassLocation=1` moves registration to `AfterDOF` for comparison.

## Implemented pass sequence

High quality mode always builds these eight RDG raster passes when active:

1. `BlackMist.PrefilterHalf`
2. `BlackMist.DownsampleQuarter`
3. `BlackMist.DownsampleEighth`
4. `BlackMist.DownsampleSixteenth`
5. `BlackMist.UpsampleEighth`
6. `BlackMist.UpsampleQuarter`
7. `BlackMist.UpsampleHalf`
8. `BlackMist.Composite`

Intermediate RDG textures are named:

- `BlackMist.D1.Half`
- `BlackMist.D2.Quarter`
- `BlackMist.D3.Eighth`
- `BlackMist.D4.Sixteenth`
- `BlackMist.U3.Eighth`
- `BlackMist.U2.Quarter`
- `BlackMist.U1.Half`
- `BlackMist.Output` when no override output is supplied

## Settings and threading

- UObject-facing settings live in `FBlackMistSettings`.
- Render-thread settings live in POD `FBlackMistRenderSettings`.
- `UBlackMistSubsystem` sanitizes and normalizes settings on the game thread, then transfers the snapshot with `ENQUEUE_RENDER_COMMAND`.
- Render thread does not access UObjects.
- No `FlushRenderingCommands` calls were added.
- UObject-facing `ScaleWeights` uses `FLinearColor` for UHT/Blueprint compatibility; it is converted to `FVector4f` in the render POD.

## Console variables

- `r.BlackMist.Enable`
- `r.BlackMist.Debug`
- `r.BlackMist.IntermediateFormat`
- `r.BlackMist.ForcePassLocation`

## Shader include correction

- Runtime shader source now includes Engine shader headers by absolute virtual path:
  - `/Engine/Private/Common.ush`
  - `/Engine/Private/ScreenPass.ush`
- This fixes plugin shader compilation failing at `/Plugin/BlackMist/Private/BlackMist.usf(1): File 'Common.ush' not found`.
- Verified there are no remaining relative `Common.ush` or `ScreenPass.ush` includes under `Shaders`.

## Build and validation performed

Command:

```text
D:\Unreal\UE_5.7\Engine\Build\BatchFiles\RunUAT.bat BuildPlugin -Plugin="C:\Users\t-hoshino\Documents\BlackMist\BlackMist.uplugin" -Package="C:\Users\t-hoshino\Documents\BlackMist\Saved\BlackMistPluginBuild" -TargetPlatforms=Win64 -Rocket
```

Result:

- `UnrealEditor Win64 Development`: succeeded.
- `UnrealGame Win64 Development`: succeeded.
- `UnrealGame Win64 Shipping`: succeeded.
- UHT completed with warnings treated as errors.
- Re-run after plugin-root publication layout normalization also succeeded with root `BlackMist.uplugin`.
- `Config/FilterPlugin.ini` packages README, LICENSE, third-party notices, and `Docs/...` into BuildPlugin output.

Additional validation after shader include correction:

- Created a temporary project under `Saved/BlackMistShaderHost` that references the local BlackMist plugin root.
- Built `BlackMistShaderHostEditor Win64 Development` with UE 5.7.4 `Build.bat`; result succeeded and rebuilt `BlackMistRuntime`.
- Attempted a D3D12 SM6 `UnrealEditor-Cmd.exe` startup for runtime global shader compilation. The process did not produce a usable project log before timeout and was stopped manually, so this is not counted as a passed runtime shader compile test.
- The consuming project `D:\Unreal\EditormodePlug\EditormodePlug.uproject` references this plugin through `AdditionalPluginDirectories`.
- Launched that project with `UnrealEditor-Cmd.exe -NullRHI -ExecCmds=quit`; log showed `Mounting External plugin BlackMist`, `PCD3D_SM6` shader autogen, engine initialization, and `Cmd: quit`.
- No `BlackMist` shader compiler errors, `Common.ush` lookup errors, or global shader fatal errors appeared in the post-fix log. The process stayed alive after `Cmd: quit` and was stopped manually after timeout.

## Not yet verified

This repository has no real target `.uproject`, map, viewport, or content, so the following remain unverified:

- Editor viewport visual result.
- PIE start/stop lifecycle.
- Standalone runtime behavior.
- `profilegpu`, `stat gpu`, and `DumpGPU` pass/extents.
- Actual GPU timings.
- Visual calibration against emissive test scenes.
- Split-screen, scene capture, dynamic resolution, TSR/AA matrix, alpha propagation, and MRQ tiling behavior.

These items must be tested in the real UE project that consumes this plugin.
