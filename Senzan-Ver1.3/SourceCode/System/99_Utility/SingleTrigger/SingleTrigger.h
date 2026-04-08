#pragma once

#include <functional>

/****************************
*	一回だけ処理を通すクラス.
*   担当:mattya3713.
****/


class SingleTrigger final
{
private:
    bool m_Triggered;

public:
    SingleTrigger()
        : m_Triggered(false)
    {
    }


    /*******************************************
    * @brief         : 条件が満たされたとき一度だけ関数を実行する.
    * @param OnceFunc : 実行したい関数.
    *******************************************/
    template<typename Once>
    void CheckAndTrigger(Once OnceFunc)
    {
        if (!m_Triggered)
        {
            // 条件を満たしたときに実行する処理.
            m_Triggered = true;
            OnceFunc();
        }
    }

    /*******************************************
    * @brief			: 条件が満たされたとき一度だけ関数を実行する.
    * @param OnceFunc	: 実行したい関数.
    * @param Reset  	: 条件リセット.
    *******************************************/
    template<typename Once, typename ResetFunc>
    void CheckAndTrigger(Once OnceFunc, ResetFunc Reset)
    {
        if (!m_Triggered)
        {
            // 条件を満たしたときに実行する処理.
            m_Triggered = true;
            OnceFunc();
        }
        else if (m_Triggered == true && Reset())
        {
            // 条件がリセットされたら、再度実行可能にする.
            m_Triggered = false;
        }
    }
};
