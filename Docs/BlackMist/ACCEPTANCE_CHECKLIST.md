# UE5 Black Mist acceptance checklist

Codex は、実施した項目だけを `[x]` にする。未実施項目を推測で完了扱いにしない。

## A. Version and API

- [ ] `.uproject` と Engine から対象 UE バージョンを確定した。
- [ ] 対象版の `SceneViewExtension.h` を読んだ。
- [ ] 対象版の `PostProcessMaterialInputs.h` を読んだ。
- [ ] 対象版の `ScreenPass.h` を読んだ。
- [ ] Engine 内の `SubscribeToPostProcessingPass` 実装例を確認した。
- [ ] View-aware overload が必要な版ではそれを使用している。
- [ ] 採用した callback 位置が HDR・トーンマップ前であることを確認した。
- [ ] API 差分を `IMPLEMENTATION_STATUS.md` に記録した。

## B. Plugin and module

- [ ] Engine ソースを変更していない。
- [ ] Runtime プラグインまたは既存 Runtime モジュール内に実装した。
- [ ] Shader virtual path を適切な LoadingPhase で登録した。
- [ ] `RenderCore`、`RHI`、`Renderer` などの依存が最小限である。
- [ ] `Renderer/Private` include path を追加していない。
- [ ] Editor-only module を Runtime から参照していない。

## C. Lifecycle and threading

- [ ] World 単位の ViewExtension である。
- [ ] `FSceneViewExtensions::NewExtension` で生成している。
- [ ] WorldSubsystem が生成と破棄を管理している。
- [ ] Render Thread は UObject を参照しない。
- [ ] 設定は sanitization 済み POD として Render Thread へ転送される。
- [ ] 毎フレーム `FlushRenderingCommands` を呼ばない。
- [ ] PIE の開始／停止を複数回行ってもクラッシュしない。
- [ ] モジュール／World 終了時に stale callback や raw pointer が残らない。

## D. Activation

- [ ] Global enable CVar がある。
- [ ] `bEnabled == false` で callback を登録しない。
- [ ] `Intensity <= epsilon` で callback を登録しない。
- [ ] 無効時に `BlackMist.*` GPU pass が 0 本である。
- [ ] 対象外 World/View をフィルタしている。
- [ ] SceneCapture は既定で対象外である。
- [ ] unsupported feature level は安全にパススルーする。

## E. Eight-pass graph

- [ ] Pass 1 `BlackMist.PrefilterHalf` が存在する。
- [ ] Pass 2 `BlackMist.DownsampleQuarter` が存在する。
- [ ] Pass 3 `BlackMist.DownsampleEighth` が存在する。
- [ ] Pass 4 `BlackMist.DownsampleSixteenth` が存在する。
- [ ] Pass 5 `BlackMist.UpsampleEighth` が存在する。
- [ ] Pass 6 `BlackMist.UpsampleQuarter` が存在する。
- [ ] Pass 7 `BlackMist.UpsampleHalf` が存在する。
- [ ] Pass 8 `BlackMist.Composite` が存在する。
- [ ] 高品質モードの Black Mist GPU pass は正確に 8 本である。
- [ ] 各 intermediate texture の extent が期待値である。
- [ ] odd resolution と 1 pixel 近辺の小 View でゼロ extent を作らない。

## F. Prefilter and color science

- [ ] scene-linear HDR で処理する。
- [ ] soft-knee threshold がある。
- [ ] threshold は pre-exposure に対して安定している。
- [ ] MaxScatterRadiance による anti-firefly がある。
- [ ] NaN/Inf を伝播させない。
- [ ] scatter source は RGB chroma を保持する。
- [ ] input ViewRect 外をサンプルしない。

## G. Downsample/upsample quality

- [ ] Pass 1 は高品質 downsample kernel を使う。
- [ ] Pass 2–4 は low-pass downsample である。
- [ ] Pass 5–7 は tent upsample である。
- [ ] level weights は正規化されている。
- [ ] 解像度変更で halo の見かけ半径が大きく変化しない。
- [ ] 点光源に hard ring、stair-step、強い ringing が出ない。

## H. Composite

- [ ] halo を full-resolution SceneColor へ合成する。
- [ ] highlight core loss がある。
- [ ] core loss は同じ exposure-stable mask を使う。
- [ ] contrast adjustment が過剰な白濁を起こさない。
- [ ] shadow lift は低輝度マスク付きである。
- [ ] halo tint が設定可能である。
- [ ] 出力 RGB を負値にしない。
- [ ] SceneColor alpha を保存する。
- [ ] `Inputs.OverrideOutput` が有効な場合にそこへ書く。
- [ ] callback は有効な `FScreenPassTexture` を返す。

## I. View and output correctness

- [ ] `FScreenPassTextureSlice` / Texture2DArray input を対象版の公式パターンで扱う。
- [ ] Dynamic resolution または screen percentage で破綻しない。
- [ ] non-zero ViewRect を正しく扱う。
- [ ] split-screen で隣 View の色を halo に取り込まない。
- [ ] viewport resize 後も古い texture reference を保持しない。
- [ ] alpha propagation を有効にした構成を確認した、または未対応を明記した。

## J. Diagnostics

- [ ] `r.BlackMist.Debug` または同等機能がある。
- [ ] scatter mask を表示できる。
- [ ] D1、D2、D3、D4 を表示できる。
- [ ] accumulated halo を表示できる。
- [ ] final、halo-only、core-loss-only を比較できる。
- [ ] Debug 表示のための 9 本目の常設 pass を追加していない。
- [ ] RDG texture/event 名が識別しやすい。

## K. Build and runtime tests

- [ ] Editor Development build が成功した。
- [ ] Shader compile が成功した。
- [ ] Editor viewport で動作した。
- [ ] PIE で動作した。
- [ ] Standalone で動作した。
- [ ] Effect On/Off を runtime で切り替えた。
- [ ] Auto exposure 変化を試験した。
- [ ] 極端に明るい emissive を試験した。
- [ ] black frame / white frame を試験した。
- [ ] camera cut または急な露出変化を試験した。
- [ ] プロジェクトが使う AA/TSR 構成を試験した。
- [ ] SceneCapture が既定で影響を受けないことを確認した。

## L. Profiling

- [ ] `stat gpu` を取得した。
- [ ] `profilegpu` で 8 pass を確認した。
- [ ] DumpGPU で intermediate extent を確認した。
- [ ] GPU、RHI、解像度、screen percentage を記録した。
- [ ] 各 pass と合計 GPU time を記録した。
- [ ] Intermediate format を記録した。
- [ ] RGBA16F 以外を採用した場合、画質比較を記録した。

## M. Documentation and final report

- [ ] Enable/setup 手順を書いた。
- [ ] 全パラメータの単位、範囲、既定値を書いた。
- [ ] 8 pass の役割を書いた。
- [ ] callback 位置と理由を書いた。
- [ ] 対象 UE 版固有の注意を書いた。
- [ ] MRQ tiled、VR、mobile などの制限を正直に書いた。
- [ ] 変更ファイル一覧を報告した。
- [ ] 実行したコマンドと結果を報告した。
- [ ] 未実施項目を報告した。
