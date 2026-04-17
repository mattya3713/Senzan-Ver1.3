#include "DirectSound.h"

DirectSound::DirectSound()
    : m_lpSoundBuffer(nullptr)
{
    m_originalFrequency = 0;
}

DirectSound::~DirectSound()
{
    Release();
}

bool DirectSound::Init(const std::wstring& path, LPDIRECTSOUND8 lpSoundInterface)
{
    if (!lpSoundInterface) return false;

    WaveData wavData;

    // WAVファイルの読み込み.
    if (!LoadWavFile(path, &wavData))
    {
        return false;
    }

    // バッファ情報の設定.
    DSBUFFERDESC dsbd;
    ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS;
    dsbd.dwBufferBytes = wavData.Size;
    dsbd.guid3DAlgorithm = DS3DALG_DEFAULT;
    dsbd.lpwfxFormat = &wavData.WavFormat;

    // セカンダリバッファ作成.
    if (FAILED(lpSoundInterface->CreateSoundBuffer(&dsbd, &m_lpSoundBuffer, NULL)))
    {
        delete[] wavData.SoundBuffer;
        return false;
    }

    // 波形データをセカンダリバッファに書き込む.
    void* bufferPtr = nullptr;
    DWORD bufferSize = 0;
    if (FAILED(m_lpSoundBuffer->Lock(0, wavData.Size, &bufferPtr, &bufferSize, NULL, NULL, 0)))
    {
        delete[] wavData.SoundBuffer;
        return false;
    }

    memcpy(bufferPtr, wavData.SoundBuffer, bufferSize);

    m_lpSoundBuffer->Unlock(bufferPtr, bufferSize, NULL, 0);

    // 元のフォーマット周波数を保持.
    m_originalFrequency = wavData.WavFormat.nSamplesPerSec;

    // WAVデータを内部に保持しておく（複製バッファ作成用）.
    m_bufferSize = wavData.Size;
    m_bufferData.resize(m_bufferSize);
    memcpy(m_bufferData.data(), wavData.SoundBuffer, m_bufferSize);
    m_wavFormat = wavData.WavFormat;

    // 一時バッファの解放.
    delete[] wavData.SoundBuffer;

    // 保存しておく.
    m_lpDSInterface = lpSoundInterface;
    return true;
}

bool DirectSound::SetFrequency(DWORD frequency)
{
    if (!m_lpSoundBuffer) return false;
    // 周波数の範囲チェックはDirectSound側で行われる.
    if (SUCCEEDED(m_lpSoundBuffer->SetFrequency(frequency)))
    {
        m_currentFrequency = frequency;
        return true;
    }
    return false;
}

DWORD DirectSound::GetOriginalFrequency() const
{
    return m_originalFrequency;
}

void DirectSound::Release()
{
    if (m_lpSoundBuffer)
    {
        m_lpSoundBuffer->Stop();
        m_lpSoundBuffer->Release();
        m_lpSoundBuffer = nullptr;
    }
}

LPDIRECTSOUNDBUFFER DirectSound::Play(bool isLoop)
{
    if (!m_lpSoundBuffer) return nullptr;

    // ループ再生は必ず先頭から再生（BGMなど）.
    if (isLoop)
    {
        ResetPosition();
        DWORD flags = DSBPLAY_LOOPING;
        m_lpSoundBuffer->Play(0, 0, flags);
        return nullptr;
    }

    // 非ループ（効果音）は重複再生を許可するためにバッファを複製して返す。.
    // 既存バッファが再生中であれば別バッファを作って再生する。.
    DWORD status = 0;
    if (SUCCEEDED(m_lpSoundBuffer->GetStatus(&status)) && (status & DSBSTATUS_PLAYING))
    {
        // 再生中 -> バッファを複製して再生.
        if (!m_lpDSInterface) return nullptr;

        DSBUFFERDESC dsbd;
        ZeroMemory(&dsbd, sizeof(dsbd));
        dsbd.dwSize = sizeof(dsbd);
        dsbd.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS;
        dsbd.dwBufferBytes = m_bufferSize;
        dsbd.lpwfxFormat = &m_wavFormat;

        LPDIRECTSOUNDBUFFER newBuf = nullptr;
        if (SUCCEEDED(m_lpDSInterface->CreateSoundBuffer(&dsbd, &newBuf, NULL)))
        {
            // 新規バッファに WAV データを書き込む.
            void* pBuf = nullptr;
            DWORD lockSize = 0;
            if (SUCCEEDED(newBuf->Lock(0, m_bufferSize, &pBuf, &lockSize, NULL, NULL, 0)))
            {
                memcpy(pBuf, m_bufferData.data(), static_cast<size_t>(lockSize));
                newBuf->Unlock(pBuf, lockSize, NULL, 0);
            }
            // 複製バッファにも直近に設定された周波数（ピッチ）を適用.
            if (m_currentFrequency != 0)
            {
                newBuf->SetFrequency(m_currentFrequency);
            }
            newBuf->Play(0, 0, 0);
            return newBuf;
        }
        return nullptr;
    }

    // 再生していなければ先頭から再生.
    ResetPosition();
    DWORD flags = 0;
    m_lpSoundBuffer->Play(0, 0, flags);
    return nullptr;
}

void DirectSound::Stop()
{
    if (m_lpSoundBuffer)
    {
        m_lpSoundBuffer->Stop();
    }
}

void DirectSound::ResetPosition()
{
    if (m_lpSoundBuffer)
    {
        m_lpSoundBuffer->SetCurrentPosition(0);
    }
}

void DirectSound::SetVolume(int volume)
{
    if (!m_lpSoundBuffer) return;

    // DirectSoundの音量は -10000(無音) ～ 0(最大).
    // 引数の 0 ～ 10000 を適切な範囲に変換.
    LONG dsVolume = static_cast<LONG>(volume) - 10000;
    if (dsVolume < DSBVOLUME_MIN) dsVolume = DSBVOLUME_MIN;
    if (dsVolume > DSBVOLUME_MAX) dsVolume = DSBVOLUME_MAX;

    m_lpSoundBuffer->SetVolume(dsVolume);
}

int DirectSound::GetVolume() const
{
    if (!m_lpSoundBuffer) return 10000;

    LONG dsVolume = 0;
    if (SUCCEEDED(m_lpSoundBuffer->GetVolume(&dsVolume))) {
        // dsVolume is in range DSBVOLUME_MIN .. DSBVOLUME_MAX (-10000..0).
        int volume = static_cast<int>(dsVolume + 10000);
        if (volume < 0) volume = 0;
        if (volume > 10000) volume = 10000;
        return volume;
    }

    return 10000;
}

bool DirectSound::LoadWavFile(const std::wstring& fileName, WaveData* outWaveData)
{
    HMMIO mmioHandle = mmioOpenW((LPWSTR)fileName.c_str(), NULL, MMIO_READ);
    if (!mmioHandle) return false;

    MMCKINFO riffckInfo;
    riffckInfo.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    if (MMSYSERR_NOERROR != mmioDescend(mmioHandle, &riffckInfo, NULL, MMIO_FINDRIFF))
    {
        mmioClose(mmioHandle, 0);
        return false;
    }

    MMCKINFO ckInfo;
    ckInfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if (MMSYSERR_NOERROR != mmioDescend(mmioHandle, &ckInfo, &riffckInfo, MMIO_FINDCHUNK))
    {
        mmioClose(mmioHandle, 0);
        return false;
    }

    if (mmioRead(mmioHandle, (HPSTR)&outWaveData->WavFormat, sizeof(WAVEFORMATEX)) != sizeof(WAVEFORMATEX))
    {
        mmioClose(mmioHandle, 0);
        return false;
    }

    if (outWaveData->WavFormat.wFormatTag != WAVE_FORMAT_PCM)
    {
        mmioClose(mmioHandle, 0);
        return false;
    }

    mmioAscend(mmioHandle, &ckInfo, 0);

    ckInfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (MMSYSERR_NOERROR != mmioDescend(mmioHandle, &ckInfo, &riffckInfo, MMIO_FINDCHUNK))
    {
        mmioClose(mmioHandle, 0);
        return false;
    }

    outWaveData->Size = ckInfo.cksize;
    outWaveData->SoundBuffer = new char[ckInfo.cksize];
    if (mmioRead(mmioHandle, (HPSTR)outWaveData->SoundBuffer, ckInfo.cksize) != (LONG)ckInfo.cksize)
    {
        delete[] outWaveData->SoundBuffer;
        mmioClose(mmioHandle, 0);
        return false;
    }

    mmioClose(mmioHandle, 0);
    return true;
}
