# ULE4JIS

このリポジトリはオリジナルに加えて以下の要素が追加されています。
- CMake対応
- Windows起動時に自動起動するオプション
- 自動起動時にダイアログを表示しない(一瞬だけ表示される)
- エミュレーション状態を記憶する
- タスクトレイの右クリックメニューが閉じない不具合を修正

## ビルド方法

このプログラムをビルドするには、以下のツールが必要です：

- CMake (バージョン 3.10 以上)
- Visual Studio 2022
- C++ MFC for latest v143 build tools (x86 & x64) 等
- Boost 1.90.0 等

## ビルド方法

1. リポジトリをクローンまたはダウンロードします。

2. プロジェクトのルートディレクトリで以下のコマンドを入力：
   ```
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022"
   cmake --build . --config Release
   ```

3. ビルドが成功すると、`build/Ule4Jis/Release/` ディレクトリに実行ファイル `Ule4Jis.exe` が生成されます。