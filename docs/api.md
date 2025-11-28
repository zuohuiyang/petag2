# API接口文档

## CLI
- `petag2 -i <input> <output> <metadata>` 插入元数据
- `petag2 -e <input>` 读取元数据

## 编解码
- `EncodeMetadata(std::string, std::string* err) -> std::vector<uint8_t>`
- `DecodeMetadata(std::vector<uint8_t>, std::string* out, std::string* err) -> bool`

## DLL API（仅字符串）
- `InsertPeTag(const wchar_t* filePath, const wchar_t* outputFilePath, const char* metadata, uint32_t metaLen) -> uint32_t`
- `ReadPeTag(const wchar_t* filePath, char* outMetadata, uint32_t outCapacity, uint32_t* outLen) -> uint32_t`
- 元数据格式：可打印 ASCII 字符串，长度 1-255 字节（内部使用单字节长度字段）。

## 文件I/O
- `ReadFileBytes(std::wstring, std::string* err) -> std::vector<uint8_t>`
- `WriteFileBytes(std::wstring, std::vector<uint8_t>, bool overwrite, std::string* err) -> bool`
- `IsPEFile(std::vector<uint8_t>) -> bool`

<!-- 验签模块已移除 -->

## SCT核心（Chromium，不改动）
- `CreatePEBinary(base::span<const uint8_t>) -> std::unique_ptr<BinaryInterface>`
- `BinaryInterface::SetTag(base::span<const uint8_t>) -> std::optional<std::vector<uint8_t>>`
- `BinaryInterface::tag() -> std::optional<std::vector<uint8_t>>`
