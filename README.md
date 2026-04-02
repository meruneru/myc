# myc

mycは、学習目的で作成されている自作Cコンパイラです。
x86-64 Linuxをターゲットとしています。

## 特徴

現在、以下の機能をサポートしています。

- 基本的な算術演算 (`+`, `-`, `*`, `/`)
- 比較演算 (`==`, `!=`, `<`, `<=`, `>`, `>=`)
- 変数への代入 (`=`)
- 複数行のステートメント
- 制御構文 (`if`, `else`, `while`, `for`)
- 関数呼び出し
- 関数定義
- ローカル変数

## 開発環境の準備

### 必要なパッケージ

コンパイルとテストには以下のパッケージが必要です。

- `gcc` / `binutils` (リンカおよびアセンブラとして使用)
- `make`
- `gauche` (テスト実行に使用)

Ubuntu/Debian系の場合、以下のコマンドでインストールできます。

```bash
sudo apt-get update
sudo apt-get install -y gcc make gauche binutils
```

### Dockerを使用した開発

提供されている `Dockerfile` を使用して開発環境を構築することもできます。

```bash
docker build -t myc-dev -f Docker/Dockerfile .
./onLin.sh
```

## 使い方

### ビルド

```bash
make
```

実行ファイル `build/myc` が生成されます。

### テストの実行

テストはGauche (Scheme) で記述されており、並列で実行されます。

```bash
make test
```

### コンパイラの直接実行

ソースコード（文字列）を引数として渡すと、標準出力にx86-64アセンブリを出力します。

```bash
./build/myc "main() { return 42; }"
```

## ディレクトリ構成

- `main.c`: エントリポイント
- `parse.c`: 字句解析・構文解析
- `codegen.c`: コード生成
- `myc.h`: ヘッダーファイル
- `test.scm`: Gaucheによるテストスクリプト
- `test_helper.c`: テスト用の補助コード
