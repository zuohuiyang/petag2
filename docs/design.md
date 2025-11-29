# PE隐藏标签命令行工具设计

## 目标
- 在已签名PE中插入/读取隐藏元数据，保持Authenticode签名有效。
- 封装Chromium SCT方案：仅调用`CreatePEBinary`、`SetTag`、`tag`。

## 架构
- 模块：cli、base(io/logging)、codec(metadata)、verify(CryptoAPI)、依赖(third_party/chrome_updater/chrome/updater)。
- 依赖：BoringSSL（不改动源码）、Windows CryptoAPI。

## 流程
- 插入：读取→PE校验→CreatePEBinary→Encode→SetTag→写新文件→WinVerifyTrust验证。
- 读取：读取→PE校验→CreatePEBinary→tag→Decode→打印。

## 数据格式
- version(1B=0x01) | len(1B) | payload(lenB) | crc32(4B LE)。

## 约束
- ASCII英文字符，长度2–30；错误路径明确输出英文信息。

## 安全
- 不覆盖原文件；严格I/O错误处理；保持签名有效不修改SignerInfo。
