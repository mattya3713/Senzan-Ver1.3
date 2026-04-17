#include "VirtualPad.h"
#include <iostream> 
#include <cmath> // std::sqrt, std::min, std::max に必要.

// --------------------------------------------------------------------------------.
// コンストラクタ.
// --------------------------------------------------------------------------------.

VirtualPad::VirtualPad()
{
}

// --------------------------------------------------------------------------------.
// プライベート テンプレートヘルパーの実装 (checkActionState).
// --------------------------------------------------------------------------------.

// テンプレート関数はヘッダーにインライン展開するのが理想ですが、ここでは.cppに記述.
template <typename KeyCheckFunc, typename ButtonCheckFunc>
bool VirtualPad::checkActionState(eGameAction action,
    KeyCheckFunc&& keyCheck,
    ButtonCheckFunc&& buttonCheck) const
{
    // アクションのバインディングを取得.
    auto it = m_KeyMap.find(action);
    if (it == m_KeyMap.end() || it->second.Type != eActionType::Button)
    {
        // 軸アクションが間違って渡された場合のログ (Debug::Warning が定義されている前提).
        // if (it != m_KeyMap.end() && it->second.type == eActionType::Axis) { Debug::Warning("Button check called for Axis action."); }.
        return false;
    }

    const ActionBinding& binding = it->second;

    for (const auto& source : binding.Sources)
    {
        switch (source.Type)
        {
            case InputSource::eSourceType::KeyBorad:
            case InputSource::eSourceType::MouseButton:
                if (keyCheck(source.KeyCode))
                {
                    return true;
                }
                break;

            case InputSource::eSourceType::ControllerButton:
                if (buttonCheck(source.ControllerKey, source.KeyCode))
                {
                    return true;
                }
                break;

                // コントローラートリガー軸をボタンとして扱う場合の特殊なケース.
            case InputSource::eSourceType::ControllerTriggerAxis:
            {
                Input& input = Input::GetInstance();
                float triggerValue = 0.0f;
                if (source.StickTarget == InputSource::eStickTarget::LeftTrigger) {
                    triggerValue = input.GetLTrigger();
                }
                else if (source.StickTarget == InputSource::eStickTarget::RightTrigger) {
                    triggerValue = input.GetRTrigger();
                }

                // トリガーが閾値 (0.1f) 以上であれば押されていると判定.
                if (triggerValue >= 0.4f) return true;
                break;
            }

            default:
                break;
        }
    }

    return false;
}

// 押され続けているか.
bool VirtualPad::IsActionPress(eGameAction action) const
{
    Input& input = Input::GetInstance();

    // KeyBorad/MouseButton.
    auto keyCheck = [&input](const int& code) {
        return input.IsKeyRepeat(code);
        };

    // ControllerButton.
    auto buttonCheck = [&input](XInput::Key key, const int code) {
        return input.IsButtonRepeat(key);
        };

    return checkActionState(action, keyCheck, buttonCheck);
}

// 押された瞬間か.
bool VirtualPad::IsActionDown(eGameAction action, float inputBufferTime) const
{
    Input& input = Input::GetInstance();

    // KeyBorad/MouseButton.
    auto keyCheck = [&input](const int& code) {
        // TODO : inputBufferTime 入れるならここに追加.
        return input.IsKeyDown(code);
        };

    // ControllerButton.
    auto buttonCheck = [&input](XInput::Key key, const int code) {
        return input.IsButtonDown(key);
        };

    return checkActionState(action, keyCheck, buttonCheck);
}

// 離された瞬間か.
bool VirtualPad::IsActionUp(eGameAction action) const
{
    Input& input = Input::GetInstance();

    // KeyBorad/MouseButton.
    auto keyCheck = [&input](const int& code) {
        return input.IsKeyUp(code);
        };

    // ControllerButton.
    auto buttonCheck = [&input](XInput::Key key, const int code) {
        return input.IsButtonUp(key);
        };

    return checkActionState(action, keyCheck, buttonCheck);
}

// 軸アクションの合計値を取得.
float VirtualPad::GetSingleAxisValue(eGameAction componentAction) const
{
    auto it = m_KeyMap.find(componentAction);
    if (it == m_KeyMap.end() || it->second.Type != eActionType::Axis)
    {
        return 0.0f;
    }

    const ActionBinding& binding = it->second;
    Input& input = Input::GetInstance();
    float total_value = 0.0f;

    for (const auto& source : binding.Sources)
    {
        float value = 0.0f;

        switch (source.Type)
        {
            case InputSource::eSourceType::KeyBorad:
                if (input.IsKeyRepeat(source.KeyCode))
                {
                    value = 1.0f;
                }
                break;

            case InputSource::eSourceType::ControllerStickAxis:
            case InputSource::eSourceType::ControllerTriggerAxis:
                if (source.StickTarget == InputSource::eStickTarget::Left)
                {
                    value = (componentAction == eGameAction::Move_Axis_X || componentAction == eGameAction::Camera_X) ?
                        input.GetLStickDirection().x : input.GetLStickDirection().y;
                }
                else if (source.StickTarget == InputSource::eStickTarget::Right)
                {
                    value = (componentAction == eGameAction::Move_Axis_X || componentAction == eGameAction::Camera_X) ?
                        input.GetRStickDirection().x : input.GetRStickDirection().y;
                }
                else if (source.StickTarget == InputSource::eStickTarget::LeftTrigger)
                {
                    value = input.GetLTrigger();
                }
                else if (source.StickTarget == InputSource::eStickTarget::RightTrigger)
                {
                    value = input.GetRTrigger();
                }
                break;

            case InputSource::eSourceType::MouseMove:
                if (componentAction == eGameAction::Camera_X)
                {
                    value = input.GetClientCursorDelta().x;
                }
                else if (componentAction == eGameAction::Camera_Y)
                {
                    value = input.GetClientCursorDelta().y;
                }
                break;

            default:
                continue;
        }

        total_value += value * source.Scale;
    }

    // 軸の値の最大値を 1.0f に制限.
    return std::min(1.0f, std::max(-1.0f, total_value));
}

//  複合軸取得.
DirectX::XMFLOAT2 VirtualPad::GetAxisInput(eGameAxisAction axisType) const
{
    DirectX::XMFLOAT2 result = { 0.0f, 0.0f };

    // それぞれのAxisを取得.
    if (axisType == eGameAxisAction::Move)
    {
        result.x = GetSingleAxisValue(eGameAction::Move_Axis_X);
        result.y = GetSingleAxisValue(eGameAction::Move_Axis_Y);
    }
    else if (axisType == eGameAxisAction::CameraMove)
    {
        result.x = GetSingleAxisValue(eGameAction::Camera_X);
        result.y = GetSingleAxisValue(eGameAction::Camera_Y);
    }
    else
    {
        return { 0.0f, 0.0f };
    }

    // 正規化.
    float lengthSq = result.x * result.x + result.y * result.y;

    if (lengthSq > 1.0f)
    {
        float length = std::sqrt(lengthSq);
        result.x /= length;
        result.y /= length;
    }

    return result;
}

// デフォルトバインディング.
void VirtualPad::SetupDefaultBindings()
{
    using EKey = XInput::Key;
    using EStickState = XInput::StickState;
    using ESource = InputSource::eSourceType;
    using ETarget = InputSource::eStickTarget;
    using Action = eGameAction;

    // MoveForward: 'W' または 左スティック上 (スティック方向).
    m_KeyMap[Action::MoveForward] = {
        eActionType::Button,
        {
            { ESource::KeyBorad, 'W' },
            { ESource::ControllerStickDir, 0, EKey::None, EStickState::Up, ETarget::Left }
        }
    };

    // MoveBackward: 'S' または 左スティック下.
    m_KeyMap[Action::MoveBackward] = {
        eActionType::Button,
        {
            { ESource::KeyBorad, 'S' },
            { ESource::ControllerStickDir, 0, EKey::None, EStickState::Down, ETarget::Left }
        }
    };

    // MoveRight: 'D' または 右矢印キー または 左スティック右.
    m_KeyMap[Action::MoveRight] = {
        eActionType::Button,
        {
            { ESource::KeyBorad, 'D' },
            { ESource::KeyBorad, VK_RIGHT },
            { ESource::ControllerStickDir, 0, EKey::None, EStickState::Right, ETarget::Left }
        }
    };

    // MoveLeft: 'A' または 左矢印キー または 左スティック左 (追加).
    m_KeyMap[Action::MoveLeft] = {
        eActionType::Button,
        {
            { ESource::KeyBorad, 'A' },
            { ESource::KeyBorad, VK_LEFT },
            { ESource::ControllerStickDir, 0, EKey::None, EStickState::Left, ETarget::Left }
        }
    };


    // Jump: スペース または Aボタン.
    m_KeyMap[Action::Cancel] = {
        eActionType::Button,
        {
            { ESource::KeyBorad, VK_SPACE },
            { ESource::ControllerButton, 0, EKey::A }
        }
    };

    // Attack: マウス左ボタン または 右トリガー (軸入力だがここでは論理入力として扱う).
    m_KeyMap[Action::Attack] = {
        eActionType::Button,
        {
            { ESource::MouseButton, VK_LBUTTON },
            { ESource::ControllerButton, 0, EKey::X }
        }
    };

    // Parry.
    m_KeyMap[Action::Parry] = {
        eActionType::Button,
        {
            { ESource::MouseButton, VK_RBUTTON },
             { ESource::ControllerTriggerAxis, 0, EKey::None, EStickState::None, ETarget::RightTrigger, 1.0f },
             { ESource::ControllerTriggerAxis, 0, EKey::None, EStickState::None, ETarget::LeftTrigger, 1.0f }
        }
    };

    // Dodge.
    m_KeyMap[Action::Dodge] = {
        eActionType::Button,
        {
            { ESource::KeyBorad, VK_LSHIFT },
            { ESource::ControllerButton, 0, EKey::LB },
            { ESource::ControllerButton, 0, EKey::RB },
            { ESource::ControllerButton, 0, EKey::A }
        }
    };

    // SpecialAttack.
    m_KeyMap[Action::SpecialAttack] = {
        eActionType::Button,
        {
            { ESource::KeyBorad, 'Q' },
            { ESource::ControllerButton, 0, EKey::Y }
        }
    };


    // Pause.
    m_KeyMap[Action::Pause] = {
        eActionType::Button,
        {
            { ESource::KeyBorad, VK_ESCAPE },
            { ESource::ControllerButton, 0, EKey::Start }
        }
    };

    // ----- [ Axis Actions: 軸入力の内部コンポーネント ] -----.

    // 【Move Axis X】: 左スティックX軸の生の値とキーボードA/Dを合成.
    m_KeyMap[Action::Move_Axis_X] = {
        eActionType::Axis,
        {
            // 左スティックX軸.
            { ESource::ControllerStickAxis, 0, EKey::None, EStickState::Right, ETarget::Left, 1.0f },
            // キーボード 'D' (正方向).
            { ESource::KeyBorad, 'D', EKey::None, EStickState::None, ETarget::None, 1.0f },
            // キーボード 'A' (負方向).
            { ESource::KeyBorad, 'A', EKey::None, EStickState::None, ETarget::None, -1.0f },
        }
    };

    // 【Move Axis Y】: 左スティックY軸の生の値とキーボードW/Sを合成.
    m_KeyMap[Action::Move_Axis_Y] = {
        eActionType::Axis,
        {
            // 左スティックY軸 (前方/後方)
            { ESource::ControllerStickAxis, 0, EKey::None, EStickState::Up, ETarget::Left, 1.0f },
            // キーボード 'W' (正方向).
            { ESource::KeyBorad, 'W', EKey::None, EStickState::None, ETarget::None, 1.0f },
            // キーボード 'S' (負方向).
            { ESource::KeyBorad, 'S', EKey::None, EStickState::None, ETarget::None, -1.0f },
        }
    };
    
    // 【Camera Axis X】: 右スティックX軸とマウス移動X軸を合成.
    m_KeyMap[Action::Camera_X] = {
        eActionType::Axis,
        {
            { ESource::ControllerStickAxis, 0, EKey::None, EStickState::Right, ETarget::Right, 1.0f },
            // マウス移動X軸 (感度調整のためのスケール).
            { ESource::MouseMove, 0, EKey::None, EStickState::None, ETarget::None, 0.5f }
        }
    };

    // 【Camera Axis Y】: 右スティックY軸とマウス移動Y軸を合成 (Y軸は反転).
    m_KeyMap[Action::Camera_Y] = {
        eActionType::Axis,
        {
            // 右スティックY軸 (カメラでは通常反転).
            { ESource::ControllerStickAxis, 0, EKey::None, EStickState::Up, ETarget::Right, -1.0f },
            // マウス移動Y軸 (反転させてスケールを設定).
            { ESource::MouseMove, 0, EKey::None, EStickState::None, ETarget::None, -0.5f }
        }
    };
}
