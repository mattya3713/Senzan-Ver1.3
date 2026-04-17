#pragma once
#include "Game/05_InputDevice/Input.h"
#include "Engine/Utility/SingletonTemplate/SingletonTemplate.h"

/**********************************************************************************
* @author    : 未定.
* @date      : 2025/10/5.
* @brief     : 仮想パッドクラス.
*            : キーボード/マウス/コントローラー入力を統合して
*            : ゲーム内アクションへマッピングする.
**********************************************************************************/
class VirtualPad final 
    : public Singleton<VirtualPad>
{
private:
    friend class Singleton<VirtualPad>;
    VirtualPad();

public:

    // ゲーム側アクション種別.
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

    // 軸入力アクション種別.
    enum class eGameAxisAction
    {
        None,
        CameraMove,
        Move,
    };

    // アクションの入力タイプ.
    enum class eActionType
    {
        Button,
        Axis
    };

    // 入力ソースを表す構造体.
    // キーボード/マウス/コントローラーの情報を保持する.
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

        eSourceType Type;                        // 入力ソースの種別.
        int KeyCode = 0;                         // キーコード(キーボード用).
        XInput::Key ControllerKey = XInput::Key::None; // コントローラーボタン.

        XInput::StickState StickState = XInput::StickState::None; // スティック方向.

        enum class eStickTarget
        {
            None,
            Left,
            Right,
            LeftTrigger,
            RightTrigger
        };

        eStickTarget StickTarget = eStickTarget::None; // スティック対象.
        
        float Scale = 1.0f;                       // 入力スケール(軸入力で使用).
    };

    // アクションにバインドされた入力定義.
    struct ActionBinding
    {
        eActionType Type = eActionType::Button;   // アクションタイプ.
        std::vector<InputSource> Sources;         // バインドされた入力ソース一覧.
    };

public:
    // アクション -> 入力バインディングのマップ.
    // 必要に応じてメンバとして公開利用する.
    std::map<eGameAction, ActionBinding> m_KeyMap;

public:
    ~VirtualPad() override = default;

    /**********************************************************
    * @brief 指定アクションが押されているか判定する.
    * @param action 判定するゲームアクション.
    **********************************************************/
    bool IsActionPress(eGameAction action) const;

    /**********************************************************
    * @brief 指定アクションが押された瞬間か判定する(入力バッファ対応).
    * @param action 判定するゲームアクション.
    * @param inputBufferTime 入力バッファ時間(秒).
    **********************************************************/
    bool IsActionDown(eGameAction action, float inputBufferTime = 0.0f) const;

    /**********************************************************
    * @brief 指定アクションが離された瞬間か判定する.
    **********************************************************/
    bool IsActionUp(eGameAction action) const;

    /**********************************************************
    * @brief 軸入力を取得する.
    * @param axisType 取得する軸タイプ.
    * @return 2D入力値(X, Y).
    **********************************************************/
    DirectX::XMFLOAT2 GetAxisInput(eGameAxisAction axisType) const;

    /**********************************************************
    * @brief デフォルトの入力バインディングを設定する.
    **********************************************************/
    void SetupDefaultBindings();

private:
    /**********************************************************
    * @brief アクション状態をチェックする共通ヘルパー.
    * @tparam KeyCheckFunc キー判定関数オブジェクト.
    * @tparam ButtonCheckFunc ボタン判定関数オブジェクト.
    **********************************************************/
    template <typename KeyCheckFunc, typename ButtonCheckFunc>
    bool checkActionState(eGameAction action,
        KeyCheckFunc&& keyCheck,
        ButtonCheckFunc&& buttonCheck) const;

    /**********************************************************
    * @brief 単一コンポーネント(例: Move_Axis_X)の軸値を取得する.
    **********************************************************/
    float GetSingleAxisValue(eGameAction componentAction) const;

private:
    // TODO: コヨーテタイム用の入力補正処理を追加.
    float m_CoyoteTimeTimer = 0.0f;
};

