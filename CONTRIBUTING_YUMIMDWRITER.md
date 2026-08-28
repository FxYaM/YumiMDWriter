# 贡献指南

提交修改前请：

1. 保持改动集中，不重写 ghostwriter 核心编辑器或 Markdown 渲染管线。
2. Windows 专属行为使用 `Q_OS_WIN` 或 CMake `WIN32` 隔离。
3. 不使用固定屏幕坐标或手动 DPI 倍乘。
4. 新增源码保留 SPDX 版权与许可证标识。
5. 运行 Release/Debug 构建中与改动相关的测试，至少执行完整 CTest。
6. 涉及预览时同时测试长段落、图片、列表、表格和代码块。
7. 涉及输入法时记录 Windows 版本、输入法、缩放比例和显示器配置。

提交 issue 时请提供复现 Markdown、系统版本、Qt/KF 版本、缩放比例、截图或短录屏。不要提交包含私人文件路径、访问令牌或未获授权素材的日志。

上游通用贡献规则仍可参考 [CONTRIBUTING.md](CONTRIBUTING.md)。适合所有平台的修复应优先考虑回馈 KDE ghostwriter 上游。
