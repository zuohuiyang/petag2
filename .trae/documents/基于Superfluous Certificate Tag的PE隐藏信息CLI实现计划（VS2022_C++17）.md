## 总体确认
- 方案可行且满足要求：采用“Superfluous Certificate”（多余证书）在PKCS#7证书列表末尾注入携带自定义扩展OID的伪证书，数据位于扩展的`OCTET STRING`中；Windows信任链不会使用该证书，因而不破坏签名。
- 仓库已具备核心能力（PE/MSI解析、PKCS#7/ASN.1处理）：
  - 接口：`c:\project\petag2\certificate_tag.h:41` 创建PE对象；`c:\project\petag2\certificate_tag.h:29` 读取标签；`c:\project\petag2\certificate_tag.h:35` 写入标签。
  - 实现：`c:\project\petag2\certificate_tag.cc:46` 解析PE；`c:\project\petag2\certificate_tag.cc:471` 写入PE标签；`c:\project\petag2\certificate_tag.cc:206` 解析PKCS#7标签；`c:\project\petag2\certificate_tag.cc:304` 重建含“伪证书”的SignedData；`c:\project\petag2\certificate_tag_internal.h:258` 标签扩展OID。
- 微调建议：
  - 在插入前后使用 Windows CryptoAPI（`WinVerifyTrust`）做签名验证；失败则回滚。
  - CLI层限定元数据长度在2–30字符（UTF-8），并提供备份与恢复。

## 模块划分与职责
- PE文件解析模块：基于`PEBinary::Parse`定位`IMAGE_DIRECTORY_ENTRY_SECURITY`和`WIN_CERTIFICATE`，提取PKCS#7 SignedData（参考`c:\project\petag2\certificate_tag.cc:46`）。
- 数字签名处理模块：
  - 读取：`ParseTagImpl`从证书列表末尾证书的扩展中查找`kTagOID`并返回标签（`c:\project\petag2\certificate_tag.cc:206`）。
  - 写入：`SetTagImpl`复制SignedData并在证书列表末尾追加/替换伪证书，扩展OID为`kTagOID`（`c:\project\petag2\certificate_tag.cc:304`，OID见`c:\project\petag2\certificate_tag_internal.h:258`）。
  - PE落盘：重建`WIN_CERTIFICATE`头、8字节对齐并更新`IMAGE_DATA_DIRECTORY`长度（`c:\project\petag2\certificate_tag.cc:471`）。
- 元数据编解码模块：
  - 编码：用户字符串→UTF-8字节→写入扩展`OCTET STRING`；无需自定义封装。
  - 解码：`BinaryInterface::tag()`返回字节，CLI按UTF-8输出；非UTF-8时以hex输出。
- 命令行接口模块：
  - `petag.exe read -f <path>`：打印标签（字符串或hex）。
  - `petag.exe insert -f <path> -d <data>`：备份、校验原签名、写入标签、再次校验、成功后覆盖原文件。
  - 错误处理：所有失败路径提供明确退出码与消息。

## 技术实现细节
- ASN.1编码/解码：复用BoringSSL `CBS/CBB`（`CopyASN1`、`AddName`等，见`c:\project\petag2\certificate_tag.cc:200`、`c:\project\petag2\certificate_tag.cc:180`）；无需额外ASN.1库。
- OID：使用仓库既有`kTagOID`（`1.3.6.1.4.1.11129.2.1.9999`，见`c:\project\petag2\certificate_tag_internal.h:258`）。
- 对齐与长度：保持`WIN_CERTIFICATE`8字节对齐；正确更新`Certificate Table`长度（`c:\project\petag2\certificate_tag.cc:492`–`c:\project\petag2\certificate_tag.cc:496`）。
- VS2022/C++17：控制台项目；启用`/std:c++17`；链接`Crypt32.lib`、`Wintrust.lib`；`#include <wincrypt.h>`、`#include <softpub.h>`。
- Windows CryptoAPI验证：实现`VerifySignatureWithWintrust(const std::wstring& path)`：构造`WINTRUST_FILE_INFO`与`WINTRUST_DATA`，`GUID WINTRUST_ACTION_GENERIC_VERIFY_V2`，前后各调用一次。

## CLI设计与使用
- 读：`petag.exe read -f test.exe`
  - 输出：若为合法UTF-8，直接打印；否则打印`0x...`十六进制。
- 写：`petag.exe insert -f test.exe -d "渠道_123"`
  - 流程：备份→`WinVerifyTrust`（原文件）→`BinaryInterface::SetTag`生成新镜像→写盘→`WinVerifyTrust`（新文件）。
  - 长度校验：2–30字符；空串作为边界测试允许，但默认拒绝空插入（需带`--allow-empty`）。

## 保证通过微软签名检查
- 技术原理：伪证书不参与信任链与签名计算；仅更新`WIN_CERTIFICATE`包裹长度与对齐；签名主体未变。
- 验证手段：
  - 程序内：`WinVerifyTrust`返回`ERROR_SUCCESS`。
  - 可选外部：`signtool verify /pa`（文档说明中提供，CLI不依赖）。

## 测试方案细化
- 单元测试：
  - PE解析正确性：加载`test.exe`，校验`PEBinary::Parse`成功、证书类型与长度合理。
  - 签名验证：`VerifySignatureWithWintrust(test.exe)`在插入前后均为成功。
  - 元数据读写：长度2、30、空、非UTF-8（随机字节）往返一致；重复写入相同长度不增长（参考`c:\project\petag2\certificate_tag_unittest.cc:68`–`c:\project\petag2\certificate_tag_unittest.cc:72`）。
- 集成测试：
  - 端到端：`petag.exe insert/read`针对实际签名PE。
  - 备份与回滚：任一步失败自动恢复原文件。
- 现有测试参考：PE往返测试（`c:\project\petag2\certificate_tag_unittest.cc:30`–`c:\project\petag2\certificate_tag_unittest.cc:72`）；MSI相关测试存在但本CLI以PE为主。
- 测试数据：
  - 使用目录下`signed test.exe`（请提供或确认路径）；先备份。
  - 多长度数据（2–30）；空数据、最大长度；非UTF-8字节。

## 交付物
- 完整VS2022解决方案（`petag-cli.sln`）：
  - `petag-lib`（静态库）：对`certificate_tag`封装友好API。
  - `petag-cli`（控制台）：`read/insert`命令。
  - `petag-tests`（gtest）：单元/集成测试。
- 可执行文件：`petag.exe`（x64，C++17）。
- 文档：开发说明与API文档（使用、限制、验证方法）。
- 测试用例：覆盖所列场景并附运行说明。

## 风险与边界
- 某些厂商工具对PE后缀数据或证书表严格性不同；以`WinVerifyTrust`为准。
- 某些极端PE布局（自定义节/覆盖区）需保持原始字节顺序；当前实现对`WIN_CERTIFICATE`以外区域不改动。
- 大于30字节的标签可技术上支持，但本CLI按需求限制以简化验证。

## 下一步
- 如确认上述方案，开始创建VS2022解决方案与CLI入口，集成CryptoAPI验证，并补齐单元/集成测试与文档。