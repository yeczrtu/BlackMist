# Repository instructions: UE5 Black Mist ViewExtension

## Scope

このリポジトリで、Unreal Engine の SceneViewExtension と Render Dependency Graph を使った高品質ブラックミスト・ポストエフェクトを実装する。リポジトリルートは `BlackMist.uplugin` を持つプラグインルートであり、通常は UE プロジェクトの `Plugins/BlackMist` として配置する。詳細仕様は `Docs/BlackMist/IMPLEMENTATION_PLAN.md`、完了条件は `Docs/BlackMist/ACCEPTANCE_CHECKLIST.md` を正とする。

## Hard constraints

- Engine ソースは変更しない。Runtime プラグインとして実装する。
- Material ベースのポストプロセスへ置き換えない。Global Shader と RDG Screen Pass を使用する。
- 高品質モードは必ず 8 GPU パスにする。
  1. HDR プリフィルタ兼 1/2 ダウンサンプル
  2. 1/4 ダウンサンプル
  3. 1/8 ダウンサンプル
  4. 1/16 ダウンサンプル
  5. 1/8 アップサンプル兼合成
  6. 1/4 アップサンプル兼合成
  7. 1/2 アップサンプル兼合成
  8. フル解像度最終合成
- 効果は原則として Scene Linear HDR、トーンマップ前で実行する。
- 無効時または強度ゼロ時はポストプロセス・デリゲートを登録せず、Black Mist の RDG パスを 1 本も生成しない。
- Render Thread から UObject を参照しない。ゲームスレッド設定は POD スナップショットへ変換し、Render Command で転送する。
- 毎フレーム `FlushRenderingCommands` を呼ばない。
- `Renderer/Private` ヘッダーに依存しない。公開ヘッダーで実装できない場合は、変更する前に理由を実装ノートへ記録する。
- unrelated なファイル変更、整形、リネームを行わない。

## Version and API verification

実装前に必ず以下を行う。

1. `.uproject`、関連 `.uplugin`、Engine の `Build.version` から対象 UE バージョンを確定する。
2. 対象 Engine の次の実ヘッダーを読む。
   - `Engine/Source/Runtime/Engine/Public/SceneViewExtension.h`
   - `Engine/Source/Runtime/Renderer/Public/PostProcess/PostProcessMaterialInputs.h`
   - `Engine/Source/Runtime/Renderer/Public/ScreenPass.h`
   - `Engine/Source/Runtime/RenderCore/Public/RenderGraphBuilder.h`
3. Engine ソース内で `SubscribeToPostProcessingPass` の既存実装例を検索する。
4. UE 5.8 では View 引数を持たない旧 3 引数 `SubscribeToPostProcessingPass` は非推奨かつ呼ばれない。対象版で利用可能な View 単位オーバーロードを使う。
5. 推測した API 名やシグネチャでコードを書かない。コンパイル対象の実ヘッダーを正とする。
6. API 差分と採用したコールバック位置を `Docs/BlackMist/IMPLEMENTATION_STATUS.md` に記録する。

## Architecture rules

- ViewExtension は可能なら `FWorldSceneViewExtension` を継承し、ワールド単位で有効化する。
- ViewExtension の生成は `FSceneViewExtensions::NewExtension` を使う。
- 設定とライフサイクルは `UWorldSubsystem` で管理する。
- シェーダーソースはプラグインの `Shaders/Private` に置き、モジュール起動時に仮想シェーダーパスを登録する。
- シェーダーを含む Runtime モジュールは、対象版で必要な早期 LoadingPhase を使用する。既存プロジェクト規約がなければ `PostConfigInit` を第一候補とする。
- RDG テクスチャはコールバック内で作成し、フレームをまたいで保持しない。
- 各 View の `ViewRect` を尊重する。画面全体を前提に UV や Extent を計算しない。
- 入力が Texture2DArray slice の場合を考慮し、対象版の `FScreenPassTextureSlice` / `CopyFromSlice` パターンを使う。
- `Inputs.OverrideOutput` が有効な場合、最終パスは必ずその出力へ書く。
- 無効時のパススルーは、対象版の `ReturnUntouchedSceneColorForPostProcessing` など公式ヘルパーを使用する。
- 中間テクスチャは RGB の正値 HDR。最終合成では入力 Scene Color の Alpha を保存する。

## Shader rules

- Global Shader は 4 種類を基本とする。
  - Prefilter/downsample shader
  - Reusable downsample shader
  - Reusable tent upsample shader
  - Composite shader
- 8 パスを維持しつつ、同じ shader class を複数パスで再利用してよい。
- Pass 1 はソフトニー付きハイライト抽出、プリエクスポージャ対応、anti-firefly、1/2 ダウンサンプルを同時に行う。
- Pass 2–4 は広がりを作るための低域ダウンサンプル。
- Pass 5–7 は 9 tap 相当の tent upsample とレベル別重み合成。
- Pass 8 は halo 加算、highlight core loss、コントラスト圧縮、控えめな shadow lift、alpha 保存を行う。
- RGB を sRGB として演算しない。scene-linear 前提で処理する。
- NaN/Inf を伝播させず、極端な radiance は設定値でクランプする。
- 画面端は clamp sampling とし、split-screen の隣接 View を読み込まない。

## Workflow

- 最初に現状を読み、既存のモジュール名、命名、設定方式、ビルド方法へ合わせる。
- 実装を小さなマイルストーンに分け、各マイルストーン後にビルドする。
- 最初は identity pass で ViewExtension の挿入位置と OverrideOutput 処理を確認し、その後 8 パスへ拡張する。
- 各 RDG パスに `BlackMist.*` の明確なイベント名を付ける。
- コンパイルエラーを broad include や private include で隠さない。依存モジュールと公開 API を正す。
- 新しい production dependency は追加しない。
- 実装中は `Docs/BlackMist/IMPLEMENTATION_STATUS.md` を更新する。

## Build and validation

- 対象プロジェクトの標準ビルド手順を優先する。
- Windows の標準例:
  - `Engine/Build/BatchFiles/Build.bat <ProjectName>Editor Win64 Development -Project="<ProjectPath>.uproject" -WaitMutex -NoHotReloadFromIDE`
- 最低限、Editor Development ビルドを通す。
- 可能なら Development Game と Shipping compile も確認する。
- Editor viewport、PIE、Standalone で On/Off とパラメータ変更を確認する。
- `profilegpu`、`stat gpu`、`DumpGPU` で 8 パスとテクスチャ解像度を確認する。
- `r.BlackMist.Debug` などの debug view を実装し、各段の出力を検証可能にする。
- テスト不能な項目は成功扱いにせず、未実施理由と再現手順を報告する。

## Completion report

完了時は以下を簡潔に報告する。

- 変更ファイル一覧
- 対象 UE バージョンと採用 API
- 採用した post-process 挿入位置と理由
- 実際の 8 パス一覧
- 実行したビルド／テストと結果
- GPU プロファイル結果
- 未確認事項、既知の制限、次の最適化候補
