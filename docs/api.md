# API接口文档

## CLI
- `petag2 -i <input> <output> <metadata>` 插入元数据
- `petag2 -e <input>` 读取元数据

## 编解码
- `EncodeMetadata(std::string, std::string* err) -> std::vector<uint8_t>`
- `DecodeMetadata(std::vector<uint8_t>, std::string* out, std::string* err) -> bool`

## 文件I/O
- `ReadFileBytes(std::wstring, std::string* err) -> std::vector<uint8_t>`
- `WriteFileBytes(std::wstring, std::vector<uint8_t>, bool overwrite, std::string* err) -> bool`
- `IsPEFile(std::vector<uint8_t>) -> bool`

<!-- 验签模块已移除 -->

## SCT核心（Chromium，不改动）
- `CreatePEBinary(base::span<const uint8_t>) -> std::unique_ptr<BinaryInterface>`
- `BinaryInterface::SetTag(base::span<const uint8_t>) -> std::optional<std::vector<uint8_t>>`
- `BinaryInterface::tag() -> std::optional<std::vector<uint8_t>>`
