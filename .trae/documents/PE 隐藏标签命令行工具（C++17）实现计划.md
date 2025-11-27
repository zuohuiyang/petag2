## 约束与原则
- 不改动 `chrome/updater` 源码；外层仅封装调用 `CreatePEBinary`、`SetTag`、`tag`。
- 不改动 BoringSSL 源码与头文件；仅作为外部依赖集成（头文件与静态库）。

## 项目结构
- `tools/pctag.sln`（VS2022解决方案）
- `tools/pctag.vcxproj`（C++17控制台程序）
- `src/cli/main.cpp`（命令行接口：insert/read/help）
- `src/base/io.{h,cpp}`（文件读写、路径校验、不覆盖原文件）
- `src/base/logging.{h,cpp}`（英文控制台日志）
- `src/codec/metadata_codec.{h,cpp}`（ASCII校验、2–30长度、CRC32封装/解析）
- `src/verify/wintrust_verify.{h,cpp}`（`WinVerifyTrust`验签）
- 依赖引用（不修改）：`chrome/updater/*`、`third_party/boringssl/src/include`、`third_party/abseil-cpp`、`base/*`
- `samples/`（`signed.exe`）与 `scripts/run_tests.bat`

## 命令行功能
- 插入：`-i/--insert [input] [output] [metadata]`
  - 校验ASCII与长度→读取PE→`CreatePEBinary`→`EncodeMetadata`→`SetTag`→写新文件→`WinVerifyTrust`验证
- 读取：`-e/--read [input]`
  - 读取PE→`CreatePEBinary`→`tag()`→`DecodeMetadata`→英文打印

## 构建与依赖
- VS2022、C++17、`/W4`、`/EHsc`；链接 `wintrust.lib; crypt32.lib; advapi32.lib; user32.lib; crypto.lib`
- 头文件包含：`chrome/`、`third_party/boringssl/src/include`、`third_party/abseil-cpp`、`base/`
- BoringSSL集成：按官方流程编译 `crypto.lib`（不改动源码），置于 `third_party/boringssl/lib`；头文件置于 `third_party/boringssl/src/include`

## 测试
- 单元：编解码、CRC32、错误路径、I/O边界
- 集成：插入→签名验证→读取→比对；含空/超长/无效PE/未签名PE用例

## 交付
- 完整VS方案与Release x64可执行
- 设计文档、API文档、用户手册、测试报告
- `samples/`与测试脚本

## 步骤
1. 新建解决方案与项目，配置C++17与库/包含目录
2. 实现 `base/io`、`codec/metadata_codec`、`verify/wintrust_verify`
3. 编写 `cli/main.cpp` 封装 `CreatePEBinary`/`SetTag`/`tag`
4. 补齐测试与脚本，使用 `samples/signed.exe` 验证