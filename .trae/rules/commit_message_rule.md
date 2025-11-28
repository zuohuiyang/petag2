# Trae Rule: 提交信息规范（Commit Message Style）

## 总则
- 语言：统一使用中文。
- 结构：首行摘要（Summary）+ 空行 + 详情（Details）。
- 安全：不得包含密钥、令牌、账户信息等敏感内容。

## 首行摘要（Summary）
- 内容简洁明确，控制在 72 字以内。
- 使用动宾短语，避免句号与多余修饰。
- 建议包含模块前缀，示例：`api:`、`docs/api:`、`style/build:`、`cli:`、`dll:`。
- 概括本次提交的核心主题，避免堆砌细节。

## 详情（Details）
- 空行后采用项目化条目，每行以 `- ` 开头，单行完整表达一个要点。
- 覆盖范围：变更点、影响面、接口/行为变化、兼容性、迁移说明。
- 必须包含验证信息：构建与测试状态（例如 VS2022 配置、`scripts/run_tests.bat`、`signtool` 验签结果）。
- 代码引用使用 `file_path:line_number` 格式，示例：`include/petag_api.h:31`。
- 避免冗长段落，详情总长度建议不超过 500 字。

## 合并与整合
- 当连续多次提交属于同一主题，按需使用软重置整合为一次提交：`git reset --soft HEAD~N` 后重新提交。
- 重新撰写摘要与详情，避免在详情中重复显而易见的改动。

## 禁止项
- 过度叙事性文字、表情/emoji、无关吐槽。
- 中英文异常混排；统一中文描述，英文仅用于专业术语或标识符。

## 示例

示例一：
```
api: DLL 接口字符串化并放宽长度

- InsertPeTag/ReadPeTag 改为字符串输入输出（include/petag_api.h:31,37）
- DLL/CLI 与实现同步更新（src/dll/pctag_api.cpp, src/cli/main.cpp）
- ASCII 校验放宽至 1-255 并更新错误信息（src/codec/metadata_codec.cpp）
- 单元测试覆盖边界（tests/unittest/metadata_codec_test.cpp:13）
- VS2022 Release x64 构建通过；run_tests.bat 通过；signtool 验签成功
```

示例二：
```
docs/api: README 增补 DLL 调用与发布产物

- 新增 DLL 示例与 JSON 建议（README.md）
- 发布包包含二进制与头文件（out/x64/Release/petag2_package.zip）
- 测试脚本改为 JSON 插入（scripts/run_tests.bat）
```

