#pragma once

#include <dsound.h>
#include <mmsystem.h>
#include <string>

// DirectSoundおよびマルチメディアAPI用のライブラリリンク.
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "winmm.lib")

/******************************************************************
* CDirectSoundクラス.
* 個別の音声データ（セカンダリバッファ）を管理するクラス.
******************************************************************/
class DirectSound
{
public:
    // WAVファイル解析用構造体.
    struct WaveData
    {
        WAVEFORMATEX WavFormat;
        char* SoundBuffer;
        DWORD        Size;

        WaveData() : SoundBuffer(nullptr), Size(0) { ZeroMemory(&WavFormat, sizeof(WavFormat)); }
    };

public:
    DirectSound();
    ~DirectSound();

    // 初期化（ファイルパスとDirectSoundインターフェースを受け取る）.
    bool Init(const std::wstring& path, LPDIRECTSOUND8 lpSoundInterface);

    // 解放.
    void Release();

    // 再生.
    // 再生.
    // 戻り値: 重複再生用に作成したバッファを返す（NULL なら内部バッファを再利用）.
    LPDIRECTSOUNDBUFFER Play(bool isLoop);
    // 停止.
    void Stop();
    // 再生位置を最初に戻す.
    void ResetPosition();
    // 音量調整 (0 ～ 10000).
    void SetVolume(int volume);
    // 音量取得 (0 ～ 10000).
    int GetVolume() const;
    // 周波数設定 (ピッチ調整).
    bool SetFrequency(DWORD frequency);
    // 元のサンプルレートを取得.
    DWORD GetOriginalFrequency() const;

private:
    // WAVファイルの読み込み.
    bool LoadWavFile(const std::wstring& fileName, WaveData* outWaveData);

private:
    LPDIRECTSOUNDBUFFER m_lpSoundBuffer;    // セカンダリバッファ.
    DWORD m_originalFrequency; // 元のサンプルレート.
    // 所有している DirectSound インターフェース（バッファ複製に使用）.
    LPDIRECTSOUND8 m_lpDSInterface = nullptr;
    // WAV データのコピー（複製バッファ作成のために保持）.
    std::vector<char> m_bufferData;
    WAVEFORMATEX m_wavFormat = {};
    DWORD m_bufferSize = 0;
    // 最後に設定された再生周波数（ピッチ調整後）.
    DWORD m_currentFrequency = 0;
};
