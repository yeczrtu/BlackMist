# Codex task: Implement an 8-pass Black Mist ViewExtension for UE5

このリポジトリに、`Docs/BlackMist/IMPLEMENTATION_PLAN.md` と `Docs/BlackMist/ACCEPTANCE_CHECKLIST.md` に従って、高品質ブラックミストを実装してください。

## 最初に行うこと

1. リポジトリ構成と `.uproject` を調査し、プロジェクト名、対象 UE バージョン、既存プラグイン規約、標準ビルドコマンドを確定してください。
2. 対象 Engine の `SceneViewExtension.h`、`PostProcessMaterialInputs.h`、`ScreenPass.h` を直接読み、実際に利用できる API シグネチャを確認してください。
3. `SubscribeToPostProcessingPass` の Engine 内実装例を検索し、同じバージョンのパターンへ合わせてください。
4. `Docs/BlackMist/IMPLEMENTATION_STATUS.md` を作成し、検出結果、採用設計、マイルストーン、ビルド結果を記録してください。

## 実装要件

- Engine 改造なしの Runtime プラグインとして実装する。
- `FWorldSceneViewExtension` または対象版で相当する ViewExtension を使う。
- WorldSubsystem でライフサイクルと設定を管理する。
- ゲームスレッド設定を render-thread-safe な POD スナップショットへ転送する。
- Scene Linear HDR、トーンマップ前に実行する。
- デフォルト挿入位置は Motion Blur 後・Tonemap 前を第一候補とする。ただし、対象 Engine の実際のパス順、SceneColor format、解像度を確認し、適さない場合は AfterDOF などへ変更して理由を記録する。
- 高品質モードは次の 8 パスを必ず生成する。
  1. Prefilter + 1/2 downsample
  2. 1/4 downsample
  3. 1/8 downsample
  4. 1/16 downsample
  5. 1/16 → 1/8 tent upsample + combine
  6. 1/8 → 1/4 tent upsample + combine
  7. 1/4 → 1/2 tent upsample + combine
  8. Full-resolution composite
- Pass 1 は soft-knee highlight extraction、pre-exposure compensation、anti-firefly clamp を含める。
- Pass 8 は halo、core loss、contrast、shadow lift、halo tint、alpha preservation を含める。
- 無効時または intensity 0 の場合は callback を登録せず、BlackMist RDG pass を生成しない。
- `Inputs.OverrideOutput`、ViewRect、split-screen、dynamic resolution、Texture2DArray slice を正しく扱う。
- private Renderer header は使用しない。
- 各パスに `BlackMist.PrefilterHalf` などの RDG event 名を付ける。
- Debug mode で Prefilter、各 downsample、最終 halo、mask、final の確認を可能にする。

## 進め方

計画書のマイルストーン順に実装し、各段階でビルドしてください。コンパイル API を推測せず、対象 Engine ヘッダーと既存 Engine 実装を正としてください。問題が起きても、要件を黙って削除せず、原因、試した修正、残課題を `IMPLEMENTATION_STATUS.md` に記録してください。

## 完了条件

`ACCEPTANCE_CHECKLIST.md` の必須項目を満たし、最後に次を報告してください。

- 変更ファイル
- 8 パスの実装場所と役割
- ビルド／テスト結果
- GPU capture または `profilegpu` で確認したパス名と解像度
- 対象 UE バージョン固有の API 対応
- 既知の制限と未実施項目
