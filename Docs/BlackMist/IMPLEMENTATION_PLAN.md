# UE5 ViewExtension Black Mist — 8-pass implementation plan for Codex

## 0. Document status

- Baseline API reference: Unreal Engine 5.8
- Actual authority: the Engine headers used by the target repository
- Delivery form: Runtime plugin, no Engine source modifications
- Primary rendering path: desktop SM5/SM6, scene-linear HDR, pre-tonemap
- Quality target: four-level diffusion pyramid, eight GPU passes

UE 5.8 is only the reference point. Codex must detect the project's actual UE version and adapt to its real API. In UE 5.8, the old `SubscribeToPostProcessingPass(Pass, Callbacks, bEnabled)` overload is deprecated and no longer called; the View-aware overload is the expected integration point.

---

## 1. Goal

Implement a production-oriented Black Mist post effect that behaves as optical diffusion rather than a conventional additive bloom. The effect must:

- spread bright radiance over multiple spatial scales;
- softly reduce the highlight core as light is redistributed;
- slightly compress local/global contrast without washing the entire frame white;
- preserve shadow detail and source color;
- run before tonemapping in HDR;
- preserve alpha and respect the active ViewRect;
- cost zero Black Mist GPU passes when disabled;
- expose runtime settings and diagnostic views;
- remain entirely inside a plugin.

### Explicit non-goals for the first implementation

- physical polarization, diffraction, ghosting, chromatic aberration, or lens coating simulation;
- temporal accumulation of the halo;
- a full camera/lens calibration workflow;
- mobile renderer support;
- guaranteed tiled Movie Render Queue seam-free output without tile overlap;
- engine-native integration into the built-in Bloom UI.

These can be separate follow-up work. Do not weaken the 8-pass core to include them.

---

## 2. Fixed architectural decisions

### 2.1 Plugin form

Create or extend a Runtime plugin. Suggested name if the repository has no established naming convention:

```text
Plugins/BlackMist/
```

The plugin should contain one runtime module, suggested name `BlackMistRuntime`. If an existing rendering plugin is the better home, follow the existing module layout instead of creating a duplicate.

### 2.2 ViewExtension scope

Prefer:

```cpp
class FBlackMistSceneViewExtension : public FWorldSceneViewExtension
```

This binds activation to a specific `UWorld` and avoids running the effect in unrelated editor preview worlds. Construct it with `FSceneViewExtensions::NewExtension` and store the shared pointer in a `UWorldSubsystem`.

If the target UE version does not expose `FWorldSceneViewExtension`, derive from `FSceneViewExtensionBase` and implement an explicit world/context activation filter.

### 2.3 Post-process insertion point

The desired semantic location is:

```text
scene color after opaque/translucency/DOF/motion integration
    → Black Mist diffusion
    → native exposure/local exposure/bloom interactions as applicable
    → tonemap
```

First candidate: `EPostProcessingPass::MotionBlur`, because current UE post-processing commonly performs bloom-like lens processing after motion blur and before tonemapping.

However, callback semantics and intermediate resolution can change by engine version and project configuration. Codex must verify all of the following in the installed Engine source:

- whether the callback is invoked before or after the named pass;
- whether the input SceneColor is still scene-linear HDR;
- whether it can be half/quarter resolution under TSR/motion blur settings;
- whether after-DOF translucency is already included;
- whether `OverrideOutput` can be active at that point.

Fallback candidate: `EPostProcessingPass::AfterDOF`. Record the final decision and evidence in `IMPLEMENTATION_STATUS.md`.

Do not use `Tonemap` if the callback sees already-tonemapped LDR color. Do not replace the tonemapper.

### 2.4 Rendering implementation

Use RDG raster screen passes and Global Pixel Shaders. This is the initial production path because it:

- integrates naturally with `FScreenPassTexture` and `FScreenPassRenderTarget`;
- avoids UAV format restrictions;
- supports render-target override output;
- makes ViewRect and viewport transforms explicit;
- is easy to inspect in `profilegpu` and DumpGPU.

A compute implementation can be evaluated later, but it is not part of the first Definition of Done.

---

## 3. Final eight-pass render graph

```text
HDR SceneColor
   |
   | Pass 1: prefilter + 1/2 downsample
   v
D1: Half resolution
   |
   | Pass 2: 1/4 downsample
   v
D2: Quarter resolution
   |
   | Pass 3: 1/8 downsample
   v
D3: Eighth resolution
   |
   | Pass 4: 1/16 downsample
   v
D4: Sixteenth resolution
   |
   | Pass 5: tent upsample to 1/8 + D3 weighted combine
   v
U3: Eighth-resolution accumulated halo
   |
   | Pass 6: tent upsample to 1/4 + D2 weighted combine
   v
U2: Quarter-resolution accumulated halo
   |
   | Pass 7: tent upsample to 1/2 + D1 weighted combine
   v
U1: Half-resolution full multi-scale halo
   |
   | Pass 8: full-resolution composite with original HDR SceneColor
   v
Black Mist HDR output
```

### Required RDG event names

```text
BlackMist.PrefilterHalf
BlackMist.DownsampleQuarter
BlackMist.DownsampleEighth
BlackMist.DownsampleSixteenth
BlackMist.UpsampleEighth
BlackMist.UpsampleQuarter
BlackMist.UpsampleHalf
BlackMist.Composite
```

High-quality mode must show these eight events. Shader classes may be reused across passes.

---

## 4. Pass-by-pass specification

### Pass 1 — `BlackMist.PrefilterHalf`

**Input:** full input SceneColor for the current View

**Output:** `D1`, exact half-resolution dedicated texture, zero-based ViewRect

**Responsibilities:**

1. Sample the input with a quality downsample kernel, nominally 13 taps.
2. Respect the source `ViewRect`; never sample an adjacent split-screen View.
3. Sanitize NaN/Inf.
4. Apply an anti-firefly radiance clamp before averaging.
5. Compute luminance in scene-linear RGB.
6. Convert the user threshold to the input's pre-exposed domain, or un-pre-expose samples consistently.
7. Apply a soft-knee highlight/scatter mask.
8. Preserve source chroma by multiplying the RGB source by the scalar mask.
9. Apply overall scatter source strength.
10. Write positive HDR RGB; alpha is not needed in diffusion levels.

Suggested mask model:

```text
Y  = luminance(sceneLinearColor)
T  = threshold in the same exposure domain as Y
K  = max(T * softKnee, epsilon)
S  = saturate((Y - T + K) / (2*K))
S  = S*S*(3 - 2*S)
H  = max(Y - T, 0) / max(Y, epsilon)
M  = saturate(max(S, H))
scatterSource = sanitizedColor * M * scatterAmount
```

The exact curve may be adjusted during visual calibration, but it must be continuous around the threshold and exposure-stable.

**Anti-firefly:**

Use a configurable positive radiance ceiling. Apply the ceiling in the same pre-exposure domain as the SceneColor. Do not simply clamp after the full pyramid, because one extreme source pixel will already have contaminated the large halo.

### Pass 2 — `BlackMist.DownsampleQuarter`

**Input:** `D1`

**Output:** `D2`

**Responsibilities:**

- 13-tap or equivalent wide low-pass downsample;
- clamp addressing;
- no second threshold operation;
- maintain energy predictably;
- remove high-frequency stair-stepping from Pass 1.

### Pass 3 — `BlackMist.DownsampleEighth`

Same shader class as Pass 2, with `D2 → D3` extents and viewport parameters.

### Pass 4 — `BlackMist.DownsampleSixteenth`

Same shader class as Pass 2, with `D3 → D4` extents and viewport parameters.

The 1/16 level supplies the wide, low-amplitude veiling halo characteristic of mist filters. Do not over-weight it; otherwise the result becomes white fog.

### Pass 5 — `BlackMist.UpsampleEighth`

**Inputs:** `D4`, `D3`

**Output:** `U3`

**Responsibilities:**

- tent-filter `D4` into the `D3` resolution;
- combine with `D3` using explicit scale weights;
- avoid additive brightness that changes when levels are added or removed.

Recommended initial scale weights:

```text
Half        W1 = 0.34
Quarter     W2 = 0.30
Eighth      W3 = 0.22
Sixteenth   W4 = 0.14
Sum             1.00
```

Pass 5:

```text
U3 = D3 * W3 + TentUpsample(D4) * W4
```

### Pass 6 — `BlackMist.UpsampleQuarter`

```text
U2 = D2 * W2 + TentUpsample(U3)
```

### Pass 7 — `BlackMist.UpsampleHalf`

```text
U1 = D1 * W1 + TentUpsample(U2)
```

Use a 9-tap tent or an equivalent kernel whose radius is defined in source texels. Bilinear filtering can reduce physical samples, but the effective kernel and weights must remain documented.

### Pass 8 — `BlackMist.Composite`

**Inputs:** original input SceneColor and `U1`

**Output:** `Inputs.OverrideOutput` when valid; otherwise a new full-size RDG output

**Responsibilities:**

1. Sample original HDR SceneColor at full input resolution.
2. Recompute the same soft highlight mask used for core loss, using the same exposure convention as Pass 1.
3. Reduce the highlight core slightly:

```text
core = scene - scene * mask * coreLoss * intensity
```

4. Upsample and tint the multi-scale halo:

```text
halo = TentOrBilinear(U1) * haloTint * haloStrength * intensity
```

5. Combine:

```text
misted = max(core + halo, 0)
```

6. Apply a gentle contrast operation around a scene-linear pivot. Keep this subtle; the spatial halo should do most of the visual work.
7. Apply shadow lift through a low-luminance mask rather than adding a constant to every pixel.
8. Preserve original SceneColor alpha.
9. Support debug output selection without adding a ninth pass.
10. Write the exact output ViewRect expected by the post chain.

Suggested contrast form:

```text
misted = pivot + (misted - pivot) * contrast
```

Suggested shadow mask:

```text
shadowMask = 1 - smoothstep(shadowStart, shadowEnd, luminance(misted))
misted += shadowMask * shadowLift
```

Default Black Mist should not globally milk the blacks. `shadowLift` starts near zero.

---

## 5. Exposure and color-space requirements

### 5.1 Scene linear only

All diffusion levels and composite calculations operate on scene-linear HDR color. No sRGB gamma-space blur or thresholding.

### 5.2 Pre-exposure

UE may store SceneColor in a pre-exposed domain. Threshold, anti-firefly ceiling, and any luminance-dependent curve must be exposure-stable.

Two acceptable strategies:

**Strategy A — scale parameters into pre-exposed space**

```text
thresholdGPU = thresholdScene * preExposure
maxRadianceGPU = maxRadianceScene * preExposure
```

**Strategy B — un-pre-expose samples for mask calculations**

```text
unpreexposed = sample / max(preExposure, epsilon)
```

Then multiply the generated scatter back into the stored domain.

Strategy A is generally cheaper. Codex must confirm how pre-exposure is exposed in the target version: View uniform buffer, `FSceneView` field, or existing renderer helper. Do not hardcode exposure to 1.

### 5.3 Alpha

Intermediate textures can be RGB-only semantically. The composite output must use the original scene alpha so projects using `r.PostProcessing.PropagateAlpha` are not broken.

---

## 6. UE integration architecture

### 6.1 Suggested file tree

```text
Plugins/BlackMist/
├─ BlackMist.uplugin
├─ Shaders/
│  └─ Private/
│     ├─ BlackMistCommon.ush
│     └─ BlackMist.usf
└─ Source/
   └─ BlackMistRuntime/
      ├─ BlackMistRuntime.Build.cs
      ├─ Public/
      │  ├─ BlackMistSettings.h
      │  ├─ BlackMistSubsystem.h
      │  └─ BlackMistBlueprintLibrary.h          # optional but recommended
      └─ Private/
         ├─ BlackMistRuntimeModule.cpp
         ├─ BlackMistSceneViewExtension.h
         ├─ BlackMistSceneViewExtension.cpp
         ├─ BlackMistShaders.h
         ├─ BlackMistShaders.cpp
         ├─ BlackMistPasses.h
         ├─ BlackMistPasses.cpp
         ├─ BlackMistSubsystem.cpp
         └─ BlackMistBlueprintLibrary.cpp
```

Adapt names to the repository's conventions.

### 6.2 Module responsibilities

`FBlackMistRuntimeModule`:

- locate the plugin;
- map `/Plugin/BlackMist` to the physical `Shaders` directory using the target version's shader source mapping API;
- register console variables;
- avoid constructing world-specific ViewExtensions globally;
- unregister cleanly at module shutdown where required.

### 6.3 Build.cs dependencies

Expected minimum, subject to target version and existing module layout:

```text
Public:
- Core
- CoreUObject
- Engine

Private:
- Projects
- RenderCore
- RHI
- Renderer
```

Do not add `Renderer` private include paths. Add only dependencies proven necessary by compilation.

### 6.4 `.uplugin`

- Type: Runtime
- EnabledByDefault: according to repository policy
- LoadingPhase: early enough to register Global Shaders before shader compilation; use the target version's recommended phase. `PostConfigInit` is the baseline candidate.
- No editor-only dependency in the runtime module.

### 6.5 Settings types

Use two representations.

#### UObject-facing settings

```text
FBlackMistSettings
```

Suggested fields:

```text
bEnabled                  bool       default true
Intensity                 float      0..2, default 0.45
Threshold                 float      >=0, default 1.0 scene units
SoftKnee                  float      0..1, default 0.65
ScatterAmount             float      0..1, default 0.12
HaloStrength              float      0..4, default 0.75
CoreLoss                  float      0..1, default 0.30
Contrast                  float      0.75..1.05, default 0.96
ContrastPivot             float      >=0, default 0.18
ShadowLift                float      0..0.1, default 0.004
ShadowStart               float      default 0.03
ShadowEnd                 float      default 0.30
HaloTint                  FLinearColor, default near-neutral warm
MaxScatterRadiance        float      default 64.0 scene units
ScaleWeights              FVector4f  default (0.34,0.30,0.22,0.14)
DebugMode                 enum
bAffectSceneCaptures      bool       default false
bAffectEditorViewports    bool       default true
```

All values require explicit clamping/sanitization before transfer to the render thread.

#### Render-thread POD settings

```text
FBlackMistRenderSettings
```

No UObject pointers, delegates, dynamic arrays, or strings. Store packed floats, flags, debug mode, and normalized scale weights.

### 6.6 WorldSubsystem

`UBlackMistSubsystem : UWorldSubsystem`:

- creates one `FBlackMistSceneViewExtension` in `Initialize`;
- releases it in `Deinitialize`;
- stores the game-thread settings;
- exposes setters/getters and optional Blueprint functions;
- converts settings to sanitized POD;
- enqueues render-thread updates only when values change;
- does not tick unless a project requirement needs continuous blending.

Suggested update pattern:

```text
Game thread:
  UObject settings
    → sanitize/copy into FBlackMistRenderSettings
    → ENQUEUE_RENDER_COMMAND
      → ViewExtension.SetRenderSettings_RenderThread(snapshot)
```

Capture a thread-safe shared reference or otherwise guarantee the extension lifetime until the command executes. Never capture a raw UObject pointer in the render command.

### 6.7 ViewExtension

Required overrides depend on the target UE version, but typically include no-op lifecycle methods required by the interface plus:

```text
SubscribeToPostProcessingPass(...)
PostProcessPass_RenderThread(...)
```

Activation/filtering responsibilities:

- only the owning world;
- skip null scene/view family;
- skip reflection captures and thumbnails;
- skip scene captures unless enabled;
- skip unsupported feature levels;
- skip when `bEnabled == false`, `Intensity <= epsilon`, or global CVar is off;
- avoid duplicate callback registration for the same selected pass;
- use the View-aware overload in versions where the old overload is deprecated/not called.

The post callback returns `FScreenPassTexture` in current APIs. When disabled after registration because of a race or late setting change, use the official untouched SceneColor helper rather than returning an invalid texture.

---

## 7. ScreenPass and RDG handling

### 7.1 Input conversion

Current APIs expose SceneColor as an `FScreenPassTextureSlice`, which may represent a Texture2D or a slice of a Texture2DArray. Use the target version's official conversion path.

Baseline pattern:

```text
InputSlice = Inputs.GetInput(EPostProcessMaterialInput::SceneColor)
Input2D = FScreenPassTexture::CopyFromSlice(...)
```

Do not pass `Inputs.OverrideOutput` to the input conversion for an active effect; the final composite owns the override target. Determine `MultiViewCount` from the target Engine's existing callback examples. Passing a guessed stereo count is not acceptable.

### 7.2 Dedicated pyramid extents

Create each intermediate texture with an extent based on the current View's input rect size, not the global backbuffer size.

```text
FullSize = Input.ViewRect.Size()
Half     = max(ceil(FullSize / 2), 1)
Quarter  = max(ceil(FullSize / 4), 1)
Eighth   = max(ceil(FullSize / 8), 1)
Sixteenth= max(ceil(FullSize /16), 1)
```

All pyramid ViewRects are zero-based:

```text
[0,0] → LevelExtent
```

The final pass maps from the zero-based half halo viewport to the original SceneColor ViewRect.

### 7.3 Texture descriptors

Intermediates require:

- 2D texture;
- ShaderResource;
- RenderTargetable;
- no unnecessary mip chain;
- clear value appropriate for black;
- positive HDR format.

High-quality baseline: `PF_FloatRGBA` if supported by the target path. After correctness is established, add an optional automatic `PF_FloatR11G11B10` path for memory/bandwidth if platform capabilities and image tests pass. Do not silently reduce precision in the initial high-quality implementation.

### 7.4 Output target

If `Inputs.OverrideOutput.IsValid()`:

- bind it to Pass 8;
- use its viewport rect and load action expected by the callback contract;
- return it as the resulting `FScreenPassTexture`.

Otherwise:

- clone only the necessary full-resolution descriptor characteristics from the input;
- create a new RDG render target;
- use the input ViewRect;
- return the new texture.

The callback must never ignore a valid override output.

### 7.5 RDG lifetime

- allocate pass parameters via `GraphBuilder.AllocParameters`;
- declare all read/write texture dependencies in shader parameter structs;
- never retain `FRDGTextureRef` outside the callback or graph lifetime;
- use RDG event scopes and texture names prefixed with `BlackMist`;
- let RDG cull or alias transient resources; do not manually pool them in phase one.

---

## 8. Shader organization

### 8.1 Common include — `BlackMistCommon.ush`

Recommended helpers:

```text
BlackMistLuminance
BlackMistSanitizeColor
BlackMistSoftThreshold
BlackMistClampRadiance
BlackMistSampleClampedToViewport
BlackMistTent9
```

Do not duplicate the threshold curve between Pass 1 and Pass 8. Put the shared mask function in the common include.

### 8.2 Global shader classes

Suggested classes:

```text
FBlackMistPrefilterPS
FBlackMistDownsamplePS
FBlackMistUpsamplePS
FBlackMistCompositePS
```

Each class:

- derives from `FGlobalShader`;
- uses `SHADER_USE_PARAMETER_STRUCT`;
- defines only required parameters;
- implements `ShouldCompilePermutation` for supported desktop feature levels;
- binds the current View uniform buffer if required for exposure or viewport behavior;
- is registered with the plugin virtual shader path.

### 8.3 Prefilter parameters

Expected data:

```text
Input texture/SRV and sampler
Input viewport parameters
Output viewport size / inverse size
Threshold in GPU exposure domain
SoftKnee
ScatterAmount
MaxScatterRadiance in GPU exposure domain
PreExposure or View uniform buffer
Render target binding
```

### 8.4 Downsample parameters

```text
Input texture and sampler
Input/output viewport parameters
Input inverse extent
Render target binding
```

### 8.5 Upsample parameters

```text
Low-resolution accumulated input
Current-level base texture
Samplers
Both viewport parameter sets
CurrentLevelWeight
LowLevelWeight, normally already accumulated
Render target binding
```

### 8.6 Composite parameters

```text
Original SceneColor
Half-resolution halo
Scene and halo viewport parameters
Threshold / SoftKnee
PreExposure
Intensity
HaloStrength
CoreLoss
Contrast / pivot
Shadow lift range
HaloTint
DebugMode
Render target binding
```

### 8.7 Kernel quality

Pass 1 and downsample passes should integrate a broad enough footprint to avoid box-filter artifacts. A common quality target is an effective 13-tap bloom downsample. Upsample passes should use an effective 9-tap tent.

Bilinear hardware filtering may be used to collapse taps, but document the effective sample positions and weights in code comments. Validate with single-pixel and small-disc light test patterns.

---

## 9. Runtime controls and diagnostics

### 9.1 Console variables

Suggested names:

```text
r.BlackMist.Enable                 0/1
r.BlackMist.Debug                  0..N
r.BlackMist.IntermediateFormat     0 Auto, 1 RGBA16F, 2 R11G11B10
r.BlackMist.ForcePassLocation      development-only override
```

Project settings remain the primary artistic controls. CVars are for global enable, debugging, profiling, and emergency fallback.

### 9.2 Debug modes

Debug selection should be implemented inside existing passes, especially Pass 8, so it does not add a ninth GPU pass.

Minimum modes:

```text
0 Final
1 Scatter mask
2 D1 half prefilter
3 D2 quarter
4 D3 eighth
5 D4 sixteenth
6 Accumulated U1 halo
7 Core-loss-only comparison
8 Halo-only
```

When showing a low-resolution level, upscale it in Pass 8.

### 9.3 Presets

After correctness, add optional presets without changing the core renderer:

```text
Black Mist 1/8
Black Mist 1/4
Black Mist 1/2
Black Mist 1
```

Presets should alter settings only. They do not change pass count in high-quality mode.

---

## 10. Implementation milestones for Codex

### Milestone 0 — Repository and Engine reconnaissance

Tasks:

- identify project and Engine version;
- inspect existing plugins and naming conventions;
- inspect actual API headers;
- find Engine examples of ViewExtension post callbacks;
- determine build command;
- create `IMPLEMENTATION_STATUS.md`.

Exit criteria:

- target API signatures copied into status notes;
- selected candidate post-process event documented;
- no source changes beyond notes.

### Milestone 1 — Plugin and shader bootstrap

Tasks:

- create plugin/module or integrate with existing module;
- configure dependencies and loading phase;
- register shader virtual path;
- create minimal pass-through Global Shader compile test.

Exit criteria:

- Editor target builds;
- plugin loads;
- shader compiles without private includes.

### Milestone 2 — Subsystem, settings, and thread-safe state

Tasks:

- implement UObject-facing settings;
- implement POD render settings and sanitization;
- implement WorldSubsystem lifecycle;
- construct/release ViewExtension;
- transfer settings with render commands.

Exit criteria:

- repeated PIE start/stop has no crash or stale extension;
- Render Thread does not touch UObjects;
- settings change reaches the extension.

### Milestone 3 — Identity ViewExtension callback

Tasks:

- subscribe at the candidate pass;
- return untouched SceneColor correctly;
- exercise `OverrideOutput` path;
- add activation filters and global enable CVar;
- add RDG event marker for temporary identity pass.

Exit criteria:

- image is byte/visually unchanged when identity is active;
- no invalid texture assertion;
- editor, PIE, and standalone render;
- selected callback input is confirmed HDR/pre-tonemap.

Remove the temporary identity pass when the real graph is added.

### Milestone 4 — Pass 1 prefilter

Tasks:

- implement common luminance, sanitize, threshold helpers;
- implement pre-exposure handling;
- create half-resolution target;
- implement quality downsample and anti-firefly;
- add debug display.

Exit criteria:

- debug mask is smooth around threshold;
- exposure changes do not shift the threshold materially;
- no NaN sparkle from extreme emissives.

### Milestone 5 — Passes 2–4 downsample pyramid

Tasks:

- implement reusable downsample shader;
- generate quarter/eighth/sixteenth targets;
- validate extents for odd resolutions and tiny Views;
- add debug views.

Exit criteria:

- `profilegpu`/DumpGPU shows 4 down passes total including prefilter;
- each level has expected extent;
- no cross-View sampling in split-screen.

### Milestone 6 — Passes 5–7 upsample pyramid

Tasks:

- implement tent upsample shader;
- normalize scale weights;
- combine D4→D3, U3→D2, U2→D1;
- add accumulated halo debug view.

Exit criteria:

- halo contains near, medium, and broad components;
- changing output resolution does not substantially change apparent radius;
- no ringing or hard stair steps.

### Milestone 7 — Pass 8 composite

Tasks:

- implement core loss, halo addition, tint, contrast, shadow lift;
- preserve alpha;
- write to `OverrideOutput` when required;
- return valid `FScreenPassTexture`;
- integrate debug output modes.

Exit criteria:

- exact eight `BlackMist.*` events in high-quality mode;
- final result is pre-tonemap and exposure-stable;
- effect disabled yields zero BlackMist GPU passes;
- valid OverrideOutput path is tested.

### Milestone 8 — Robustness matrix

Test:

- Editor viewport;
- PIE multiple times;
- Standalone;
- dynamic resolution/screen percentage;
- odd viewport sizes;
- split-screen if project supports it;
- scene captures off by default;
- forward and deferred path if both are supported;
- TSR/TAA/FXAA configurations used by the project;
- alpha propagation if enabled;
- camera cuts and exposure changes;
- very bright emissive pixels;
- black frame and white frame;
- resolution changes at runtime.

Exit criteria:

- no crash, assertion, RDG validation error, or obvious border bleed;
- unsupported configurations are explicitly gated and documented.

### Milestone 9 — Profiling and optimization

Tasks:

- capture `stat gpu` and `profilegpu` numbers on available hardware;
- record intermediate memory estimates;
- test optional R11G11B10 intermediates against RGBA16F;
- ensure pass parameters do not bind unused resources;
- confirm RDG can alias transient resources;
- inspect shader instruction/tap count.

Exit criteria:

- correctness remains identical within accepted image tolerance;
- performance results and hardware/resolution are documented;
- high-quality default remains 8 passes.

### Milestone 10 — Documentation and final review

Tasks:

- write user-facing enable/setup instructions;
- document settings and recommended presets;
- complete acceptance checklist;
- review for Engine-private dependencies, threading hazards, and stale raw pointers;
- produce final implementation report.

---

## 11. Acceptance test scenes

Create or reuse a deterministic test map containing:

1. small white point lights against a dark background;
2. warm and cool emissive signs;
3. skin-like midtones with specular highlights;
4. a bright window against a dark interior;
5. deep black objects near a light source;
6. moving high-intensity emissive object;
7. fine high-contrast text or line grid;
8. split-screen boundary test if supported.

Capture reference frames for:

```text
Effect off
Intensity 0.25
Intensity 0.50
Intensity 1.00
Debug mask
Debug halo
```

Visual success criteria:

- light sources gain a broad soft halo;
- highlight cores are slightly softened, not simply doubled in brightness;
- black areas remain substantially black;
- fine details outside bright regions stay sharp;
- no obvious color desaturation or halo hue flip;
- no temporal shimmer from static lights;
- no edge seam or neighboring View contamination.

---

## 12. Profiling and debugging plan

### Required tools

- `stat gpu`
- `profilegpu`
- `DumpGPU`
- RenderDoc/PIX when available
- shader compiler log
- RDG validation in non-shipping builds

### Capture record

For every performance capture, record:

```text
GPU
Driver
RHI
UE version
Resolution
Screen percentage
AA/upscaler
Motion blur setting
Intermediate format
Black Mist parameters
Per-pass timings
Total Black Mist GPU time
```

Do not report a GPU time without this context.

### Expected resource sequence

DumpGPU must show:

```text
BlackMist.D1.Half
BlackMist.D2.Quarter
BlackMist.D3.Eighth
BlackMist.D4.Sixteenth
BlackMist.U3.Eighth
BlackMist.U2.Quarter
BlackMist.U1.Half
BlackMist.Output or OverrideOutput
```

Names may follow repository style, but must identify level and direction.

---

## 13. Risks and mitigations

### Risk: wrong callback overload

**Symptom:** effect never runs on UE 5.8 or later.

**Mitigation:** use View-aware `SubscribeToPostProcessingPass`; inspect actual header and Engine examples.

### Risk: callback at LDR stage

**Symptom:** threshold shifts with tonemap, clipped highlights produce weak halos.

**Mitigation:** verify input values/debug view before implementing the full graph; move to an earlier pass.

### Risk: motion blur output is reduced resolution

**Symptom:** unexpected overall resolution, softness, or output contract mismatch.

**Mitigation:** inspect the current input ViewRect/extent; make the pyramid relative to actual input; verify subsequent tonemap/upscale chain. Use `AfterDOF` if the project's post stack makes MotionBlur callback unsuitable.

### Risk: Texture2DArray input

**Symptom:** shader binding failure, wrong eye/slice, or RDG assertion.

**Mitigation:** use `FScreenPassTextureSlice` conversion pattern from the target Engine. Explicitly gate unvalidated stereo modes.

### Risk: ignored OverrideOutput

**Symptom:** black frame or missing final output when the extension is last in chain.

**Mitigation:** final composite writes directly to valid override target; test this path deliberately.

### Risk: exposure pumping

**Symptom:** halo threshold changes as auto exposure adapts.

**Mitigation:** scale threshold and radiance clamp by pre-exposure or un-pre-expose consistently.

### Risk: excessive white veil

**Symptom:** effect resembles fog/white mist.

**Mitigation:** reduce W4, shadow lift, contrast compression; keep scatter highlight-biased; normalize weights.

### Risk: additive energy explosion

**Symptom:** output brightens dramatically as levels accumulate.

**Mitigation:** normalized level weights and separate `HaloStrength`; test constant-color frames.

### Risk: GPU memory at 4K

**Symptom:** transient memory spike with RGBA16F.

**Mitigation:** profile RDG aliasing, then evaluate R11G11B10 with image comparisons. Do not sacrifice correctness before measurement.

### Risk: shutdown race

**Symptom:** crash when stopping PIE or unloading module.

**Mitigation:** shared/thread-safe extension lifetime, render-command-safe teardown, no raw UObject capture.

### Risk: MRQ tiling seams

**Symptom:** halo is clipped at tile boundaries.

**Mitigation:** document tile overlap requirement or gate tiled rendering until a padded tile strategy exists.

---

## 14. Definition of Done

The task is complete only when all of the following are true:

1. Runtime plugin builds against the repository's target UE version.
2. No Engine source modifications and no Renderer private includes.
3. View-aware post-process subscription is used where required by the target version.
4. High-quality mode produces exactly the eight named RDG passes.
5. Processing is scene-linear HDR and exposure-stable.
6. Pass 1 includes soft-knee extraction and anti-firefly.
7. Passes 5–7 build a normalized multi-scale halo.
8. Pass 8 includes core loss and preserves alpha.
9. Valid `OverrideOutput` is honored.
10. Disabled/zero-intensity state adds no BlackMist GPU passes.
11. Settings cross threads without UObject access on the Render Thread.
12. Editor, PIE, and Standalone tests pass.
13. Debug views expose each important stage.
14. DumpGPU/profilegpu evidence confirms extents and pass order.
15. Known unsupported modes are gated and documented.
16. `ACCEPTANCE_CHECKLIST.md` is completed honestly.

---

## 15. Final report template for Codex

```markdown
# Black Mist implementation report

## Environment
- UE version:
- Project/module:
- RHI and test platform:

## Integration
- ViewExtension base:
- Subscription overload:
- Selected post-process event:
- Reason:

## Eight passes
1.
2.
3.
4.
5.
6.
7.
8.

## Settings/threading
- Settings owner:
- Render-thread transfer:
- Shutdown handling:

## Verification
- Build commands and results:
- Runtime modes tested:
- DumpGPU/profilegpu findings:
- Per-pass and total GPU time:

## Changed files
- ...

## Known limits
- ...

## Remaining work
- ...
```

---

## 16. Primary references to inspect

Use the documentation matching the target Engine version, then verify the actual installed headers.

- Epic Games, `FSceneViewExtensions` / `FWorldSceneViewExtension`
- Epic Games, `ISceneViewExtension::SubscribeToPostProcessingPass`
- Epic Games, `ISceneViewExtension::EPostProcessingPass`
- Epic Games, `FPostProcessMaterialInputs`
- Epic Games, `FScreenPassTexture` and `FScreenPassTextureSlice`
- Epic Games, `AddDrawScreenPass`
- Epic Games, Render Dependency Graph programming guide
- Epic Games, Shaders in Plugins
- Epic Games, DumpGPU Viewer
- OpenAI, Codex custom instructions with `AGENTS.md`

Reference URLs:

```text
https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Engine/FSceneViewExtensions
https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Engine/FWorldSceneViewExtension
https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Engine/ISceneViewExtension/SubscribeToPostProcessingPass
https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Engine/ISceneViewExtension/EPostProcessingPass
https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Renderer/FPostProcessMaterialInputs
https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Renderer/FScreenPassTexture
https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Renderer/FScreenPassTextureSlice
https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Renderer/AddDrawScreenPass
https://dev.epicgames.com/documentation/unreal-engine/render-dependency-graph-in-unreal-engine
https://dev.epicgames.com/documentation/en-us/unreal-engine/shaders-in-plugins-for-unreal-engine
https://dev.epicgames.com/documentation/unreal-engine/gpudump-viewer-tool-in-unreal-engine
https://developers.openai.com/codex/guides/agents-md
```
