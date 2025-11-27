# 用户操作手册

## 安装
- 使用VS2022打开工程根目录下的`pctag.sln`，配置x64 Release。
- 确保`third_party/boringssl/src/include`与`third_party/boringssl/lib/crypto.lib`存在。

## 使用
- 插入：`petag2.exe --insert <input> <output> "{test_chan:123}"`
- 读取：`petag2.exe --read <input>`
- 帮助：`petag2.exe --help`

## 注意事项
- 不覆盖原文件，`output`必须与`input`不同。
- 元数据需为ASCII英文字符，长度2–30。
- 若提示未签名或解析失败，请确认输入为已签名PE文件。

## 集成测试
- 放置`samples/signed.exe`，运行`scripts/run_tests.bat`。
