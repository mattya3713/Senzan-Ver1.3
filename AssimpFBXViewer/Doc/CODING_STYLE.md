コーディング規約（C++ / DirectX）
# コーディング規約（C++ / DirectX）

## 1. コメント規則

### ■ ファイル・クラス・構造体
宣言直前に **Doxygen / Javadoc形式ブロックコメント** を記載する。

```h
/**********************************************************
* @author      : 淵脇未来.
* @date        : 2025/03/17.
* @brief       : デバッグ描画クラス.
*              : レイやボーンをラインで表示する.
**********************************************************/
```

使用タグ: @author, @date, @brief
各行末尾は必ず .

■ 関数（引数複数 / 公開関数）
/**
* @brief 更新処理.
* @param DeltaTime 経過時間.
* @return 成功したかどうか.
*/
使用タグ: @brief, @param, @return
各説明末尾は必ず .

■ 関数（引数1つ / なし）
// 更新処理.
void Update();

■ Set / Get
// Speedの取得.
float GetSpeed() const noexcept;

// Speedの設定.
void SetSpeed(float Speed);
16byte以下の型
inline void SetSpeed(float Speed) { m_Speed = Speed; }
inline float GetSpeed() const noexcept { return m_Speed; }
16byte超過の型
inline void SetPosition(const DirectX::XMMATRIX& Position) { m_Position = Position; }
inline const DirectX::XMMATRIX& GetPosition() const noexcept { return m_Position; }

■ 言語
コメントはすべて日本語とする

2. 命名規則
■ 基本命名
種類
規則
クラス
PascalCase
ファイル(.h)
クラス名と一致
関数
PascalCase
関数引数
メンバ変数の m_ を除いた PascalCase
通常変数
PascalCase
ローカル変数
snake_case
ローカルポインタ
p_ + snake_case
constローカルポインタ
cp_ + snake_case
DirectX::XMVECTORローカル
v_ + snake_case


■ メンバ変数接頭辞（クラス）
種類
規則
例
通常
m_ + PascalCase
m_Speed
生ポインタ
m_p + PascalCase
m_pDevice
unique_ptr
m_up + PascalCase
m_upBuffer
shared_ptr
m_sp + PascalCase
m_spTexture
weak_ptr
m_wp + PascalCase
m_wpParent
static
s_ + PascalCase
s_InstanceCount
bool
m_Is + PascalCase
m_IsActive
配列
m_ + PascalCase + s
m_Values
MyComPtr
m_cp + PascalCase
m_cpVS


■ 構造体メンバ
プレフィックスなし PascalCase
クラスメンバの m_ と明確に区別する
struct Vertex
{
	DirectX::XMFLOAT3 Position;
	float TexU;
};

■ 関数引数
種類
規則
出力引数
Out + PascalCase
その他
PascalCase

例:
bool RayCast(const Ray& Ray, HitResult& OutHit);

■ bool規則（強制）
メンバ変数: m_IsPascalCase
関数: bool IsPascalCase() const
状態を変更しない bool 戻り値関数は必ず const
禁止
Get
Check
Should

■ その他命名
種類
規則
定数(.cpp)
全大文字 + _
グローバル
g_ + PascalCase
typedef / using
PascalCase
マクロ
全大文字 + _
Interface
I + PascalCase
抽象基底
PascalCase + Base


3. クラス設計規則
■ virtual / final 強制
すべてのクラスは以下のいずれか。
virtual デストラクタを持つ（継承前提）
final を付ける
曖昧な拡張可能クラスは禁止。

■ Base命名条件
他クラスの基底になる
自身は実体にならない（抽象クラス）

■ Interface規則
class IRenderable
{
public:
	virtual ~IRenderable() = default;
	virtual void Draw() = 0;
};
I プレフィックス必須
純粋仮想のみ
メンバ変数禁止
virtualデストラクタ必須

4. コピー・ムーブ規則
所有クラスはコピー禁止
ムーブ可能なら明示
ムーブは必ず noexcept
仮想関数を持つクラスは virtualデストラクタ必須
class Texture final
{
public:
	Texture() = default;
	~Texture() noexcept = default;

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	Texture(Texture&&) noexcept = default;
	Texture& operator=(Texture&&) noexcept = default;
};

5. explicit規則
単一引数コンストラクタは必ず explicit
コピー / ムーブは除外
暗黙変換許可時は理由コメント必須

6. const規則
■ メンバ関数
取得系は必ず const
bool判定関数も const
getterは noexcept
■ 引数
16byte以下: 値渡し
16byte超過: const T&
■ ローカル
再代入しない変数は const
■ 初期化
ローカルの構造体/配列/PODは宣言時に `= {};` で値初期化する
例: `cbMaterial cb_material = {};`

7. noexcept規則
ムーブコンストラクタ
デストラクタ
単純getter
上記は必ず noexcept

8. enum規則
基本 enum class
サイズ明示（例: : uint8_t）
ビットフラグは 1 << n

9. namespace規則
グローバル空間では原則使用しない
State系など内部整理用途のみ可
using namespace 禁止
DirectXは常にフルネーム記述
DirectX::XMVECTOR

10. インデント・括弧
タブインデント（4相当）
短い処理は同一行可
if (!p_device) { return false; }
通常処理は改行

11. アクセス指定子順
public:    // struct / enum
public:    // 外部API
protected: // 派生用関数
private:   // 内部処理
private:   // メンバ変数

12. ファイル規則
1ファイル1クラス
#pragma once 必須
include順
自クラスヘッダ
プロジェクト内ヘッダ
サードパーティ（DirectX等）
標準ライブラリ

13. 実装・リソース管理
■ 所有の明確化
用途
使用
単独所有
unique_ptr
共有
shared_ptr
参照のみ
rawポインタ


■ デストラクタ
nullptr 代入禁止
RAII徹底
raw所有禁止
DirectXリソースは MyComPtr 推奨

■ mutable規則
原則禁止
キャッシュ用途のみ可
使用時はコメント必須

■ friend規則
原則禁止
強い内部結合時のみ許可
class単位で指定

■ 毎フレーム処理禁止事項
new / malloc 禁止
push_back（リサイズ発生）禁止
