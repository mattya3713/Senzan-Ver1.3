#pragma once
#include "StateBase.h"
#pragma once


/********************************************************
 * @brief : ステートマシン(有限状態機械)の管理クラス.
 * @author: mattya3713
 * @date  : 2025/09/05
********************************************************/

template<typename FSM_Owner>
class StateMachine
{
public:	

	StateMachine(FSM_Owner* pOwner)
	{
		m_pOwner = pOwner;
	}


	/**************************************************************************
	* @brief 状態を変更する.
	* @param	pNewState	：変更先の状態.	
	**************************************************************************/
    void ChangeState(std::shared_ptr<StateBase<FSM_Owner>> pNewState)
    {		
        // 現在のステートが存在し、遷移を許可していない場合は変更を拒否する.
        if (m_pCurrentState != nullptr && !m_pCurrentState->CanChangeState())
        {
            return;
        }

        if (m_pCurrentState != nullptr)
        {
            m_pCurrentState->Exit();
            m_pCurrentState = nullptr;
        }

        m_pCurrentState = pNewState;

        if (m_pCurrentState)
        {
            m_pCurrentState->Enter();
        }
    }

	// 更新処理.
	void Update()
	{
		if (m_pCurrentState != nullptr)
		{
			m_pCurrentState->Update();
		}
	}

	// 更新処理.
	void LateUpdate()
	{
		if (m_pCurrentState != nullptr)
		{
			m_pCurrentState->LateUpdate();
		}
	}

	// 描画処理.
	void Draw()
	{
		if (m_pCurrentState != nullptr)
		{
			m_pCurrentState->Draw();
		}
	}

	operator bool() const
	{
		return m_pCurrentState != nullptr;
	}

public:
	std::shared_ptr<StateBase<FSM_Owner>> m_pCurrentState;
	FSM_Owner* m_pOwner;
};
