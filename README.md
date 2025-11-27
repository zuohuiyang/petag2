# petag2 项目说明

## 目录结构
- `src/`：
  - `base/`：文件 I/O 与日志
  - `cli/`：命令行入口（help/insert/read）
  - `codec/`：元数据编解码
- `chrome/updater/`：证书标签操作（Chromium 代码）
- `tests/unittest/`：`metadata_codec` 单元测试
- `scripts/`：集成测试脚本
- `build/msvc/`：VS 项目文件（`pctag.vcxproj`、`unittest.vcxproj`）
- `third_party/boringssl/`：第三方库与头文件
- `out/`：构建产物输出目录

## 构建
- 使用 VS2022 打开根目录 `pctag.sln`
- 平台：`x64` 与 `Win32`
- 配置：`Release` 推荐；`Debug` 需匹配第三方库的运行时（已按 Release 方式配置）

## 使用
- 插入：`petag2.exe --insert <input> <output> "{test_chan:123}"`
- 读取：`petag2.exe --read <input>`
- 帮助：`petag2.exe --help`

## 说明
- 已移除：`--verify`、`--diagnose-pe`、`--dump-certdir`
- x86 构建需提供 `third_party/boringssl/lib32`（`crypto.lib` 等）

