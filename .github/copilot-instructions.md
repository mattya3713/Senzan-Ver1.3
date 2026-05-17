# Copilot Instructions

## Project Guidelines
- User prefers using Visual Studio only and has downloaded Assimp via NuGet.
- Follow the coding style defined in `AssimpFBXViewer/Doc/CODING_STYLE.md`.
- Reorganize the implementation order in `FBXModel.cpp` to match the declaration order in `FBXModel.h`:
  - Lifecycle (constructor/destructor/Load/Update/Render/Release)
  - Public accessors (RayCast, Animation-related, Bone-related, Mesh-related)
  - Private helpers

## Language Preference
- 回答は日本語で提供する。

## Key Naming Conventions (Quick Reference)
- Member variables: `m_PascalCase` (e.g., `m_IndexCount`)
- Raw pointers: `m_pPascalCase` (e.g., `m_pDevice`)
- unique_ptr: `m_upPascalCase` (e.g., `m_upBuffer`)
- shared_ptr: `m_spPascalCase` (e.g., `m_spTexture`)
- weak_ptr: `m_wpPascalCase` (e.g., `m_wpParent`)
- static: `s_PascalCase` (e.g., `s_InstanceCount`)
- bool member: `m_IsPascalCase` (e.g., `m_IsActive`)
- Function arguments: `PascalCase` (e.g., `IndexCount`) - Use PascalCase for function arguments instead of camelCase.
- Local variables: `snake_case` (e.g., `vertex_count`)
- DirectX::XMVECTOR locals: `v_snake_case` (e.g., `v_position`) - Local DirectX::XMVECTOR variables use snake_case with v_ prefix (e.g., `v_to_origin`).
- Classes/Functions: `PascalCase`
- Constants (.cpp): `ALL_CAPS` (e.g., `MAX_COUNT`)
- Global: `g_PascalCase` (e.g., `g_Instance`)
- NEVER use `using namespace` (including `DirectX`) - Always explicitly use `DirectX::` for clarity.

## クラスヘッダコメント規則（追加）
- このアカウント利用時、クラスのヘッダコメント `@author` は `mattya3713.` を使用する。
- `@date` は固定値ではなく、作業当日の日付（`YYYY/MM/DD`）を記載する。
- デザインパターンを使うクラスは、`@date` の次行に `@pattern : <PatternName>.` を記載する。
- 提示済みのコメント規則・Set/Get規約・author/date/pattern規則は UIObject だけでなく全ファイル共通ルールとして適用する。

## Set/Get 規約
- コメントは `// <項目>の取得/設定.` 形式。
- 16byte以下は値渡し、16byte超過は `const T&`。
- getterは `const noexcept` を付ける。

## 関数コメント規則
- 対象: 引数が複数あり、かつ戻り値がある関数のみコメントを付与する。
  - 具体例（コメント不要）:
    - 引数が1つだけの関数（単一引数関数）はコメント不要。既存のコメントは削除対象とする。
    - 戻り値が `void` の関数はコメント不要（複数引数であっても戻り値が void の場合は不要）。
    - 例: `DrawFBXModel()` や単一引数関数はコメント削除対象とする。
- 公開関数か否かや関数の大きさのみでコメントを付与しない。必須条件は「複数引数かつ非void戻り値」であること。
- フォーマット（以下の形式を必ず使用）:
```
/****************************************
* @brief 説明.
* @param 引数名 説明.
* @param 引数名 説明.
* @return 戻り値の説明.
****************************************/
```
- 重要: タグは必ず `@brief`, `@param`, `@return` を使用すること。各説明の末尾に必ずピリオド (.) を付けること。

## Requirements for This File
- 重複する意味の指示が既にある場合は追加せず既存指示を強化する。
- 関連項目は適切な見出しの下にまとめる。
- 一般的指示を先に、具体的指示を後に配置する。
- 説明は簡潔で命令形を用いる（例: "Use X" ではなく "Use X" のように指示形を維持する）。
- 既存の書式（コードブロック、強調など）を維持する。

## その他
- 保持する既存コンテンツは編集・統合して明確化するが、本ファイルの指示体系と命名規約は尊重して維持する。
