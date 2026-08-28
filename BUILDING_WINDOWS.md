# Windows 构建说明

本文档用于从干净源码构建 YumiMDWriter。不要依赖已经安装的 ghostwriter，也不要把依赖安装到原版程序目录。

## 已验证环境

以下组合已完成 Release 编译和 5 项自动化测试：

| 组件 | 版本 |
|---|---|
| Windows | Windows 10/11 x64 |
| Visual Studio | 2022，MSVC 19.38 |
| Qt | 6.11.1 |
| KDE Frameworks | 6.28.0 |
| CMake | 4.1.4 |
| Ninja | 1.13.2 |

项目声明的最低版本为 CMake 3.16、Qt 6.5 和 KDE Frameworks 6.0。

## 依赖

推荐按照 [KDE Craft Windows 文档](https://community.kde.org/Craft) 创建独立的 x64 MSVC CraftRoot。需要的主要组件为：

- Qt Core、Gui、Widgets、Concurrent、Svg、WebChannel、WebEngineWidgets
- KDE ConfigWidgets、CoreAddons、Sonnet、WidgetsAddons、XmlGui
- Extra CMake Modules
- CMake、Ninja、MSVC 2022 x64

必须从 “x64 Native Tools Command Prompt for VS 2022” 或已经加载 Craft 环境的 PowerShell 构建，否则 MSVC 标准库和 Windows SDK 路径可能缺失。

## 配置、编译和测试

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass -Force
. C:\CraftRoot\craft\craftenv.ps1

.\scripts\build-windows.ps1 -CraftRoot C:\CraftRoot
```

等价的手动命令：

```powershell
cmake -S . -B build -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DBUILD_TESTING=ON `
  -DCMAKE_PREFIX_PATH=C:\CraftRoot `
  -DCMAKE_INSTALL_PREFIX=$PWD\stage

cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
cmake --install build --config Release
```

生成的程序位于：

```text
build\bin\YumiMDWriter.exe
```

## 便携部署

仅复制 EXE 不足以在干净电脑运行。发布包至少需要：

- Qt 与 KDE Frameworks 运行库
- `platforms/qwindows.dll`
- 图像格式插件
- Qt WebEngine 进程、资源和 locales
- Sonnet 插件及必要数据
- MSVC 运行库
- 翻译、许可证和项目说明

先使用 `windeployqt --release --webenginewidgets` 部署 Qt，再通过 KDE Craft 的打包/部署流程收集 KDE Frameworks 运行库。最终应在未加载 Craft 环境变量的进程中启动测试，并检查 PE 导入表没有未解析 DLL。

## 常见问题

### 找不到 `algorithm`、`type_traits` 等标准头文件

构建终端没有加载 MSVC x64 环境。重新进入 Visual Studio x64 开发者终端，或先加载 Craft 环境脚本。

### CMake 找不到 Qt6 或 KF6

确认 `CMAKE_PREFIX_PATH` 指向 CraftRoot，并检查 ABI 为 `windows-cl-msvc2022-x86_64`。

### WebEngine 页面空白

检查 `QtWebEngineProcess.exe`、`resources`、`translations/qtwebengine_locales` 和平台插件是否随包部署。
