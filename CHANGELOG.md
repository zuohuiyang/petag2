# 变更日志

## v2.0.0（工程结构重构）
- 保留核心模块：read、insert、help
- 移除模块：verify、diagnose、dump（删除代码与工程引用）
- CLI 重命名与精简：统一工具名为 `petag2`，仅支持 `--insert`、`--read`、`--help`
- 可执行文件重命名：目标产物统一为 `petag2.exe`
- 解决方案迁移：将 `.sln` 移至工程根目录，VS 项目移至 `build/msvc`
- 构建配置：新增 Win32（x86）平台配置；x64 Debug/Release 可构建，x86 需提供 BoringSSL 对应库
- 文档更新：同步 API、用户手册与目录说明

