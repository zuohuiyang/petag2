# petag2 项目说明

在不破坏签名的前提下，为已签名 Windows PE 写入/读取短元数据的工具（基于 Chromium Certificate Tag）。
Embed and read short metadata in Authenticode-signed Windows PE without breaking the signature (based on Chromium Certificate Tag).

## 工程作用
- 为已签名的 Windows 可执行文件（PE）嵌入简短的元数据信息，并且嵌入后仍能通过系统的 Authenticode 签名校验。
- 通过在签名容器（PKCS#7）中利用不参与哈希的可变扩展区域写入数据，避免破坏原有签名哈希，从而保证签名有效性。
- 本工程仅封装并调用了 Chromium Updater 的证书标签实现，核心逻辑位于 `third_party/chrome_updater/chrome/updater/certificate_tag.*`。

## 使用
- Usage:
  - `petag2 -i|--insert <input> <output> <metadata>`
  - `petag2 -e|--read <input>`
- Options:
  - `--help` 显示帮助信息
- 参数说明：
  - `<input>` 为源签名文件路径（PE 文件，如 `.exe`）
  - `<output>` 为目标文件路径（写入嵌入信息后的新文件）
- 示例：
  - 插入：`petag2.exe --insert .\samples\signed.exe .\out\x64\Release\signed_tagged.exe "{\"test_chan\":123}"`
  - 读取：`petag2.exe --read .\out\x64\Release\signed_tagged.exe`
  - 帮助：`petag2.exe --help`

## DLL 调用方式
- 头文件：`include/petag_api.h`
- 导出函数：
  - `InsertPeTag(const wchar_t* filePath, const wchar_t* outputFilePath, const char* metadata, uint32_t metaLen) -> uint32_t`
  - `ReadPeTag(const wchar_t* filePath, char* outMetadata, uint32_t outCapacity, uint32_t* outLen) -> uint32_t`
- 示例（C/C++）：
```
#include <windows.h>
#include <string>
typedef uint32_t (WINAPI *InsertPeTagFn)(const wchar_t*, const wchar_t*, const char*, uint32_t);
typedef uint32_t (WINAPI *ReadPeTagFn)(const wchar_t*, char*, uint32_t, uint32_t*);
HMODULE h = LoadLibraryW(L"petag.dll");
auto InsertPeTag = (InsertPeTagFn)GetProcAddress(h, "InsertPeTag");
auto ReadPeTag = (ReadPeTagFn)GetProcAddress(h, "ReadPeTag");
uint32_t st = InsertPeTag(L"input.exe", L"output.exe", "{test:1}", 8);
char buf[256]; uint32_t out_len = 0;
st = ReadPeTag(L"output.exe", buf, 256, &out_len);
std::string meta(buf, buf + out_len);
```


## 目录结构
- `src/`：
  - `base/`：文件 I/O 与日志
  - `cli/`：命令行入口（help/insert/read）
  - `codec/`：元数据编解码
- `third_party/chrome_updater/chrome/updater/`：证书标签操作（Chromium 代码，原始源码保留）
- `tests/unittest/`：`metadata_codec` 单元测试
- `scripts/`：集成测试脚本
- `build/msvc/`：VS 项目文件（`petag.vcxproj`、`unittest.vcxproj`）
- `third_party/boringssl/`：第三方库与头文件
- `out/`：构建产物输出目录

## 构建
- 使用 VS2022 打开根目录 `petag.sln`
- 平台：`x64` 与 `Win32`
- 配置：`Release` 推荐；`Debug` 需匹配第三方库的运行时（已按 Release 方式配置）

## 发布产物
- 生成位置：`out/x64/Release/petag2_package.zip`
- 包含内容：
  - `bin/petag.dll`
  - `bin/petag2.exe`
  - `include/petag_api.h`
- 使用方式：解压后将 `bin` 拷贝到目标工程运行目录，开发集成引用 `include/petag_api.h`。
