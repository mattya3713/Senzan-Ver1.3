#pragma once
#include "System/10_Singleton/SingletonTemplate.h"
#include "SceneRegistry.generated.h"

/**********************************************************
* @author		: mattya3713.
* @date			: 2026/04/08.
* @patarn		: Singleton.
* @brief		: シーンの管理を行う.
**********************************************************/

class Buffer;
class SceneBase;
class SceneUIBase;
class SEBase;

class SceneManager :
	public Singleton<SceneManager>
{
	friend class Singleton<SceneManager>;
private:
	SceneManager();
public:
	~SceneManager();

    // シーンの読み込み.
    static void LoadScene(eList Scene, bool useFade = true);
        
    bool IsCurrentSceneMattya() const;

	// データの読み込み.
	void LoadData();

	static void Update();
	static void Draw();


    // シーンを作成.
    void MakeScene(eList Scene);

private:

private:
	std::unique_ptr<SceneBase> 		m_pScene;
	Buffer* m_pBuffer;		// 次シーンへインスタンスを入れるバッファー.

    eList m_NextSceneID;     // 次に遷移する予定のシーン ID.
    bool  m_IsSceneChanging; // シーン切り替え中フラグ.
    bool m_StartFade;        // 初回のみフェードしない.

#if _DEBUG
	eList m_CurrentSceneID;
	eList m_DebugFirstScene;	// デバッグ時に最初に起動させるシーン.
#endif // _DEBUG.
};
