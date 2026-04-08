#pragma once
#include "Game/05_InputDevice/Input.h"
#include "System/Singleton/SingletonTemplate.h"

/**********************************************************************************
* @author    : 淵脇 未来.
* @date      : 2025/10/5.
* @brief     : 仮想パッド入力ラッパークラス.
*             キーボード/マウス/コントローラ等の入力を抽象化して
*             ゲーム内のアクションへマッピングする機能を提供します。
**********************************************************************************/
class VirtualPad final 
    : public Singleton<VirtualPad>
{
private:
    friend class Singleton<VirtualPad>;
    VirtualPad();

public:

    // ゲーム内アクション列挙.
    enum class eGameAction
    {
        None,
        MoveForward,
        MoveBackward,
        MoveRight,
        MoveLeft,
        Cancel,
        Attack,
        Parry,
        Dodge,
        Pause,
        SpecialAttack,

        // 軸入力用コンポーネント.
        Move_Axis_X,
        Move_Axis_Y,
        Camera_X,
        Camera_Y,
    };

    // 軸入力の種類.
    enum class eGameAxisAction
    {
        None,
        CameraMove,
        Move,
    };

    // アクションのタイプ（ボタンか軸か）。.
    enum class eActionType
    {
        Button,
        Axis
    };

    // 入力ソースを表す構造体。.
    // キーボード/マウス/コントローラ等の情報を保持します。
    struct InputSource
    {
        enum class eSourceType
        {
            KeyBorad,
            MouseButton,
            MouseMove,
            ControllerButton,
            ControllerStickDir,
            ControllerStickAxis,
            ControllerTriggerAxis
        };

        eSourceType Type;                        // 入力ソースの種類.
        int KeyCode = 0;                         // キーコード（キーボード用）.
        XInput::Key ControllerKey = XInput::Key::None; // コントローラボタン.

        XInput::StickState StickState = XInput::StickState::None; // スティック状態.

        enum class eStickTarget
        {
            None,
            Left,
            Right,
            LeftTrigger,
            RightTrigger
        };

        eStickTarget StickTarget = eStickTarget::None; // スティックの対象.
        
        float Scale = 1.0f;                       // 入力スケール（軸系で使用）.
    };

    // アクションにバインドされた入力群を表す構造体。.
    struct ActionBinding
    {
        eActionType Type = eActionType::Button;   // アクションタイプ.
        std::vector<InputSource> Sources;         // バインドされた入力ソース一覧.
    };

public:
    // アクション -> バインディングのマップ.
    // 外部から直接参照する必要があるためメンバとして公開しています。.
    std::map<eGameAction, ActionBinding> m_KeyMap;

public:
    ~VirtualPad() override = default;

    /**********************************************************
    * @brief 指定アクションが押された瞬間かを返す.
* @param action 判定するゲームアクション.
    **********************************************************/
    bool IsActionPress(eGameAction action) const;

    /**********************************************************
    * @brief 指定アクションが押されているかを返す（入力バッファ対応）.
* @param action 判定するゲームアクション.
* @param inputBufferTime バッファ時間（秒）.
    **********************************************************/
    bool IsActionDown(eGameAction action, float inputBufferTime = 0.0f) const;

    /**********************************************************
    * @brief 指定アクションが離された瞬間かを返す.
    **********************************************************/
    bool IsActionUp(eGameAction action) const;

    /**********************************************************
    * @brief 軸入力の取得.
* @param axisType 取得する軸タイプ.
* @return 2D軸値（X, Y）.
    **********************************************************/
    DirectX::XMFLOAT2 GetAxisInput(eGameAxisAction axisType) const;

    /**********************************************************
    * @brief デフォルトの入力バインディングをセットアップする.
    **********************************************************/
    void SetupDefaultBindings();

private:
    /**********************************************************
    * @brief アクションの状態をチェックする汎用ヘルパー.
    * @tparam KeyCheckFunc キー判定関数オブジェクト
    * @tparam ButtonCheckFunc ボタン判定関数オブジェクト
    **********************************************************/
    template <typename KeyCheckFunc, typename ButtonCheckFunc>
    bool checkActionState(eGameAction action,
        KeyCheckFunc&& keyCheck,
        ButtonCheckFunc&& buttonCheck) const;

    /**********************************************************
    * @brief 単一コンポーネント（例: Move_Axis_X 等）の軸値を取得する.
    **********************************************************/
    float GetSingleAxisValue(eGameAction componentAction) const;

private:
    // TODO: コヨーテタイム等の入力補正の実装予定.
    float m_CoyoteTimeTimer = 0.0f;
};
