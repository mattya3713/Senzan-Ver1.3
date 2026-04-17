#pragma once
#include "System/10_Singleton/SingletonTemplate.h"
#include <chrono>

/**********************************************************************************
* @author    : 淵脇 未来.
* @date      : 2025/10/5.
* @brief     : ゲーム内の時間の制御.
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

	// タイマーを開始（秒単位で指定）.
	void StartTimer(float DurationSeconds);
	// タイマーを停止/再開.
	void SwitchTimer();

	//タイマーの進捗率を取得 (0.0 = 開始, 1.0 = 終了).
	float GetTimerProgress() const;

	// タイマーの終了を取得.
	bool IsTimerFinished();
	// タイマーが終了した瞬間に一度だけtrueを返す.
	bool IsTimerJustFinished();

	/*************************************************************
	* @brief	ワールドの時間の流れを変更する.
	* @param[in]	NewTimeScale	：新しい時間スケールの値(例: 0.5f で半分の速さ、2.0f で倍速).
	* ************************************************************/
	void SetWorldTimeScale(float NewTimeScale);

	/*************************************************************
	* @brief	ワールドの時間の流れを変更する.
	* @param[in]	NewTimeScale	：新しい時間スケールの値(例: 0.5f で半分の速さ、2.0f で倍速).
	* @param[in]	DurationSeconds	：NewTimeScale を適用する持続時間(秒). 0以下の場合は即座に戻る.
	* @param[in]	Override	    ：すでに変更があっても上書きする.
	* ************************************************************/
	void SetWorldTimeScale(float NewTimeScale, float DurationSeconds, bool Override = false);

	// アプリが非アクティブから復帰したときに呼ぶ。タイマーの基準をリセットして.
	// 大きなデルタが流れ込むのを防ぐ.
	void ResetOnResume();

private:
	bool IsReadyForNextFrame();
private:
	Time();

	// 生成やコピーを削除.
	Time(const Time& rhs) = delete;
	Time& operator = (const Time& rhs) = delete;
private:

	// 単調増加の時間計測にsteady_clockを使用する.
	std::chrono::time_point<std::chrono::steady_clock> m_PreviousTime;	// 前フレームの時間.

	float m_TargetFrameTime;// 目標フレーム時間(秒).
	float m_DeltaTime;		// フレーム間の時間差.

	float m_WorldTimeScale;	// 時間の尺度(通常1f, 2fで二倍の尺度で動く).

	float m_OriginalTimeScale;		// 一時変更前の元の時間スケール (1.0fなど).
	float m_TimeScaleRestoreTime;	// 時間スケールを元に戻す絶対時刻.

	float m_TimerNow;   // タイマーの現在時間.
	float m_TimerMax;   // タイマーの最大時間.
	bool  m_IsTimerActive;    // タイマーが動いているか.
	bool  m_TimerFinished; // 終了フラグ.
	bool  m_JustTimerFinished; // 終了フラグ.
};
