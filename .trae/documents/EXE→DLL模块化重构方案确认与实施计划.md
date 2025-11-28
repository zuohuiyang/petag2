# 导出函数
- `uint32_t WINAPI InsertPeTag(const wchar_t* filePath, const wchar_t* outputFilePath, const uint8_t* tagData, uint32_t tagLen);`
- `uint32_t WINAPI ReadPeTag(const wchar_t* filePath, uint8_t* outTagData, uint32_t outCapacity, uint32_t* outLen);`

# 约定与返回码
- C ABI：`extern "C" __declspec(dllexport)` + `WINAPI`
- 同步调用；DLL 不分配堆内存；就地更新支持 `outputFilePath==filePath`
- 返回码：`OK(0)`、`INVALID_ARG(100)`、`BUFFER_TOO_SMALL(110)`、`IO_ERROR(200)`、`FORMAT_ERROR(201)`、`CERT_ERROR(300)`、`NOT_FOUND(404)`

# 实现映射
- 复用 `chrome/updater/certificate_tag.*`（PE）；必要时复用 `src/codec/metadata_codec.*`；IO 使用 `src/base/io.*`

# 验证
- 单元覆盖：参数校验/缓冲不足/就地与新文件写入/无标签
- 集成：示例二进制插入与读取后 `signtool` 验签
- ABI：导出检查

# 交付物
- `pctag_api.h`、`pctag.dll`、薄壳 `petag2.exe`