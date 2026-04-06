#include "FileManager.h"
#include <shlobj.h>

const std::filesystem::path FileManager::GetSelectFolderPath()
{
	IFileDialog* pfd = nullptr;	
	std::filesystem::path folderPath;

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(hr))
	{
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
		if (SUCCEEDED(hr))
		{
			DWORD dwOptions;
			pfd->GetOptions(&dwOptions);
			pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_NOCHANGEDIR);

			hr = pfd->Show(NULL);
			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = pfd->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					if (SUCCEEDED(hr))
					{
						folderPath = pszFilePath;
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			pfd->Release();
		}
		CoUninitialize();
	}
	return folderPath;
}


const std::filesystem::path FileManager::GetSelectFilePath(const wchar_t* filter)
{
	OPENFILENAMEW ofn;
	wchar_t szFile[MAX_PATH] = { 0 };

	// カレントディレクトリを保存.
	wchar_t currentDir[MAX_PATH];
	GetCurrentDirectoryW(MAX_PATH, currentDir);

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize		= sizeof(ofn);
	ofn.hwndOwner		= NULL;	
	ofn.lpstrTitle		= L"ファイルを選択";
	ofn.lpstrFilter		= filter;	
	ofn.nFilterIndex	= 1;
	ofn.lpstrFile		= szFile;
	ofn.nMaxFile		= MAX_PATH;	
	ofn.Flags			= OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
				
	if (GetOpenFileNameW(&ofn))
	{
		// ファイル選択後、カレントディレクトリを元に戻す.
		SetCurrentDirectoryW(currentDir);		
		return std::filesystem::path(szFile);
	}

	// ファイル選択をキャンセルした場合もカレントディレクトリを元に戻す.
	SetCurrentDirectoryW(currentDir);
	return std::filesystem::path();

}


const json FileManager::JsonLoad(const std::filesystem::path& filePath)
{
	json Out;

	// ファイルサイズが0なら空のjsonを返す.	
	auto size = std::filesystem::file_size(filePath);
	if (size == 0)
	{
		return Out;
	}

	// ファイルを開く.
	std::ifstream File(filePath);

	// ファイルが開けなかったらエラーを表示.
	if (!File.is_open()) 
	{ 
		_ASSERT_EXPR(false, filePath.string() + "が開けませんでした。");
		return Out; 
	}

	// json型に変換.
	File >> Out;

	// ファイルを閉じる.
	File.close();

	return Out;
}


const bool FileManager::JsonSave(const std::filesystem::path& filePath, const json& jsonData)
{
	// ファイルを開く.
	std::ofstream File(filePath);

	// ファイルが開けなかった場合、新規作成の確認をする.
	if (!File.is_open())
	{		
		std::wstring wFileName(filePath.string().begin(), filePath.string().end());

		std::wstring message = L"フォルダが開けませんでした。\nファイル: " + wFileName + L"\n新規作成しますか？";
		int response = MessageBoxW(nullptr, message.c_str(), L"確認", MB_YESNO);

		if (response == IDNO) { return false; }
	}

	// 書き込み.
	File << std::setw(2) << jsonData << std::endl;

	// ファイルを閉じる.
	File.close();

	return true;
}
