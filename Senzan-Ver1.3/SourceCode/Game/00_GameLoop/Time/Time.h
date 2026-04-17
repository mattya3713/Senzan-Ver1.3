#pragma once
#include "Engine/Utility/SingletonTemplate/SingletonTemplate.h"
#include <chrono>

/**********************************************************************************
* @author    : 未定.
* @date      : 2025/10/5.
* @brief     : ゲーム内時間の制御.
* @pattern   : Singleton.
**********************************************************************************/

class Time final
	: public Singleton<Time>
{
public:
	friend class Singleton<Time>;
public:
	~Time();

	// フレーム間の経過時間を更新.
	void Update();

	// FPSを維持するための処理.
	void MaintainFPS();

	// デルタタイムを取得.
	const float GetDeltaTime() const;
	const float GetUnscaledDeltaTime() const;

	float GetNowTime();
	float GetWorldTimeScale() const;

	// タイマーを開始（指定秒数で終了）.
	void StartTimer(float DurationSeconds);
    // タイマーを一時停止/再開.
	void SwitchTimer();

    // タイマーの進行率を取得 (0.0 = 開始, 1.0 = 終了).
	float GetTimerProgress() const;

   // タイマーの終了状態を取得.
	bool IsTimerFinished();
    // タイマーが終了した瞬間に一度だけ true を返す.
	bool IsTimerJustFinished();

	/*************************************************************
	* @brief	ワールド時間の流れを変更する.
	* @param[in]	NewTimeScale	新しい時間スケールの値 (例: 0.5f で半分の速さ、1.0f で通常速度).
	* ************************************************************/
	void SetWorldTimeScale(float NewTimeScale);

	/*************************************************************
	* @brief	ワールド時間の流れを一定時間だけ変更する.
	* @param[in]	NewTimeScale	新しい時間スケールの値 (例: 0.5f で半分の速さ、1.0f で通常速度).
	* @param[in]	DurationSeconds	NewTimeScale を適用する継続時間. 0以下の場合は即時で元に戻る.
	* @param[in]	Override	    既に時間変更中でも上書きする.
	* ************************************************************/
	void SetWorldTimeScale(float NewTimeScale, float DurationSeconds, bool Override = false);

	// アプリ復帰時にタイマの基準時刻をリセットして
	// 大きなデルタが入るのを防ぐ.
	void ResetOnResume();

private:
	bool IsReadyForNextFrame();
private:
	Time();

  // 生成やコピーを禁止.
	Time(const Time& rhs) = delete;
	Time& operator = (const Time& rhs) = delete;
private:

 // 高精度な時間計測には steady_clock を使用する.
	std::chrono::time_point<std::chrono::steady_clock> m_PreviousTime;	// 前フレームの時刻.

   float m_TargetFrameTime;		// 目標フレーム時間(秒).
	float m_DeltaTime;			// フレーム間の経過時間.

   float m_WorldTimeScale;		// 時間の倍率(通常1f, 2fで倍速、0fで停止).

 float m_OriginalTimeScale;		// 一時変更前の元の時間スケール.
	float m_TimeScaleRestoreTime;// 時間スケールを元に戻す時刻.

    float m_TimerNow;			// タイマーの現在時刻.
	float m_TimerMax;			// タイマーの最大時間.
	bool  m_IsTimerActive;		// タイマーが動作中か.
	bool  m_TimerFinished;		// タイマー終了フラグ.
	bool  m_JustTimerFinished;	// タイマー終了直後フラグ.
};

