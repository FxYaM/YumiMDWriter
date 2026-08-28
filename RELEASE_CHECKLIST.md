# GitHub 发布检查清单

- [ ] 仓库所有者确认有权以 GPL-3.0-or-later 分发 `YumiMDWriter.ico`。
- [ ] 初始化 Git，检查提交中没有 `build/`、CraftRoot、Release、DLL、PDB、密钥或本机绝对路径。
- [ ] 执行 `reuse lint`，确认全部源码和资源都有版权、许可证信息，且不存在商业版图标。
- [ ] 记录对应的上游 ghostwriter commit 或版本标签。
- [ ] Windows x64 Release 配置与编译成功。
- [ ] CTest 全部通过。
- [ ] 在未加载 Craft 环境变量的进程中启动便携包。
- [ ] 验证中文输入法、源码行号、代码样式、顶部同步、本地图片、拖放和系统字体。
- [ ] 核对 `YumiMDWriter.exe` 的图标与 Windows 版本资源。
- [ ] 便携包包含 Qt WebEngine、平台/图像插件、KDE Frameworks 和 MSVC 运行库。
- [ ] Release 附带 `COPYING`、`LICENSES/`、第三方许可证和对应源码链接。
- [ ] GitHub Release 同时发布源码标签和 SHA-256 校验值。
