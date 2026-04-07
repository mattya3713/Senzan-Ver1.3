#pragma once
#include "../../../../Data/Library/json/json.hpp"
#include <filesystem>
#include <fstream>

// json型を作成.
using json = nlohmann::json;

/**********************************************************************************
* @author    : 未定.
* @date      : 未定.
* @brief     : ファイル関連.
**********************************************************************************/
namespace FileManager
{
    /*****************************************************************
    * @brief エクスプローラーからフォルダを選択してパスを取得する.
    *****************************************************************/
    const std::filesystem::path GetSelectFolderPath();


    /*****************************************************************
    * @brief エクスプローラーからファイルを選択してパスを取得する.
    * @param filter：拡張子のフィルター設定.
    * 
    * @note：filterの書き方.
    *   All Files(*.*)  ：フィルター名.
    *   \0				：フィルター名と拡張子の区切り.
    *   *.png			：拡張子（複数の場合は*.png;*.jpg）.
    *   \0				：複数のフィルターを設定するときの区切り.
    *****************************************************************/
    const std::filesystem::path GetSelectFilePath(const wchar_t* filter = L"All Files(*.*)\0*.*\0\0");


	/*****************************************************************
    * @brief JSON形式のファイルを開いてJSONデータを返す.
    * @param filePath：開くファイルのパス.
    *****************************************************************/
	const json JsonLoad(const std::filesystem::path& filePath);	


    /*****************************************************************
    * @brief JSONデータをJSON形式でファイルに書き込む.
    * @param filePath：書き込むファイルのパス.
    * @param jsonData：書き込むJSONデータ.
    *****************************************************************/
	const bool JsonSave(const std::filesystem::path& filePath, const json& jsonData);

}
