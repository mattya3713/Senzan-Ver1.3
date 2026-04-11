#pragma once
#include "FilePath.h"

namespace MyFilePath {

    inline std::pair<std::string, std::string> SplitFileName(const std::string& Path, const char Splitter)
    {
        // 区切り文字の位置を探す.
        auto Index = Path.find(Splitter);
        std::pair<std::string, std::string> SpliteFilePath;

        // 区切り文字が見つからない場合は、元のパスと空文字を返す.
        if (Index == std::string::npos) {

            // 区切り文字より前は元のパス.
            SpliteFilePath.first = Path;
            // 区切り文字より後は空.
            SpliteFilePath.second = "";

            return SpliteFilePath;
        }

        // 区切り文字より前を取得.
        SpliteFilePath.first = Path.substr(0, Index);

        // 区切り文字より後を取得.
        SpliteFilePath.second = Path.substr(Index + 1, Path.length() - Index - 1);

        // 分割した結果を返す.
        return SpliteFilePath;
    }

    inline std::string GetExtension(const std::string& path) {
        // 後ろから "." を探す.
        auto Index = path.rfind('.');

        // 拡張子が見つからない場合.
        if (Index == std::string::npos) {
            // 空文字を返す.
            return "";
        }

        // 拡張子を返す.
        return path.substr(Index + 1);
    }

    inline std::wstring GetExtension(const std::wstring& path) {
        // 後ろから "." を探す.
        auto Index = path.rfind(L'.');

        // 拡張子が見つからない場合.
        if (Index == std::string::npos) {
            // 空文字を返す.
            return L"";
        }

        // 拡張子を返す.
        return path.substr(Index + 1);
    }

    // モデルから得られた相対テクスチャパスを、モデル基準のパスに変換して取得する.
    inline std::string GetTexPath(const std::string& ModelPath, const char* TexPath) {

        // モデルパスの最後のスラッシュまたはバックスラッシュを探す.
        int pathIndex1 = static_cast<int>(ModelPath.rfind('/'));
        int pathIndex2 = static_cast<int>(ModelPath.rfind('\\'));
        int pathIndex = std::max(pathIndex1, pathIndex2);

        // モデルパスにスラッシュがない場合.
        if (pathIndex == std::string::npos) {

            // テクスチャパスを追加.
            return ModelPath + TexPath;
        }

        // モデルパスからフォルダパスを取得してテクスチャパスを追加.
        auto folderPath = ModelPath.substr(0, pathIndex + 1);
        return folderPath + TexPath;
    }

    // ファイルパスなどの / を \\ に置換する.
    void ReplaceSlashWithBackslash(std::string* Path)
    {
        // nullptr チェック.
        if (Path == nullptr) { return; }

        for (char& c : *Path) {
            if (c == '/') {
                c = '\\';
            }
        }
    }
}
