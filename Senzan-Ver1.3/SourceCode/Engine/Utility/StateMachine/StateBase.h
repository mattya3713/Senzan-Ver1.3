#pragma once

/********************************************************
 * @brief : ステートマシン(有限状態機械)の基底クラス.
 * @author: mattya3713
 * @date  : 2025/09/05
********************************************************/

template<typename FSM_Owner>
class StateBase
{
public:
	StateBase(FSM_Owner* ownwr)
		: m_pOwner(ownwr)
	{
	}

	virtual void Enter() {}
	virtual void Update() {}
	virtual void LateUpdate() {}
	virtual void Draw() {}
	virtual void Exit() {}
    // ステート遷移を許可するか（デフォルトは許可）.
    virtual bool CanChangeState() const { return true; }
private:
	void SetOwner(FSM_Owner* pOwner) { m_pOwner = pOwner; }

protected:
	FSM_Owner* m_pOwner = nullptr;
};
