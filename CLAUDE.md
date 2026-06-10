# StackBuffs — CLAUDE.md

## 项目概述

StackBuffs 是一个用于 **Command & Conquer: Yuri's Revenge** 的 DLL 插件，基于 **EC-SDK（Extension Container）** 框架和 **WIC（Winter Is Coming）** 库开发。它实现了一套「栈操作」Buff 系统，包含 `StackPush`（入栈标记）和 `StackTop`（栈顶操作）两种 Buff，用于在游戏运行时管理单位栈并进行栈顶操作。

## 构建命令

```bat
# 完整构建 Release
build.bat

# 或直接调用 msbuild
msbuild StackBuffs.vcxproj /p:Configuration=Release /p:Platform=Win32 /t:Build /v:minimal

# Debug 构建
msbuild StackBuffs.vcxproj /p:Configuration=Debug /p:Platform=Win32 /t:Build /v:minimal
```

## 项目结构

```
StackBuffs/
├── build.bat                    # 构建脚本（vswhere + VsDevCmd.bat + msbuild）
├── StackBuffs.sln               # Visual Studio 2022 解决方案
├── StackBuffs.vcxproj           # VC++ 项目文件
├── CLAUDE.md                    # 本文件
├── README.md                    # 效果说明文档
├── Source/                      # 【项目源文件】—— 唯一可修改的目录
│   ├── Main.cpp                 # DllMain — ECInitLibrary 入口
│   ├── StackManager.h           # StackManager 头文件
│   ├── StackManager.cpp         # StackManager 实现
│   ├── StackPushBuff.h          # StackPushBuffClass 头文件
│   ├── StackPushBuff.cpp        # StackPushBuffClass 实现
│   ├── StackTopBuff.h           # StackTopBuffClass 头文件
│   ├── StackTopBuff.cpp         # StackTopBuffClass 实现
│   └── YRMath_fixed.h           # 数学函数辅助
├── Patches/
│   └── StackBuffs.dll.inj      # Syringe 钩子注入配置
├── Release/                     # 构建输出目录
├── YRpp/                        # 【上游依赖】Yuri's Revenge SDK 头文件 — 勿动
└── EC-SDK/                      # 【上游依赖】Extension Container SDK — 勿动
```

## 技术栈

| 项目         | 详情                              |
| ------------ | --------------------------------- |
| 语言         | C++17 (Debug) / C++20 (Release)   |
| 平台         | Windows x86 (Win32), DLL          |
| 工具集       | Visual Studio 2022 (v143)         |
| 构建系统     | MSBuild                           |
| 运行时库     | `/MTd` (Debug) / `/MT` (Release)  |
| 异常处理     | `/EHsc` (Sync)                    |
| 对齐         | 8 字节 (`/Zp8`)                   |
| 指令集       | SSE2                              |
| SDK 依赖     | EC-SDK (submodule), YRpp         |

## ⚠️ 重要原则

- **YRpp/** 和 **EC-SDK/** 是上游依赖，**严禁修改**其中的任何文件
- 项目源文件仅限于 `Source/` 目录下的 `.h` 和 `.cpp`
- 如果需要新增功能，只在 `Source/` 下添加新文件，并在 `.vcxproj` 中引用

## 关键约定

### 编码
- **项目源文件**（`Source/`）：UTF-8 **without BOM**
- **SDK 头文件**（`EC-SDK/`）：UTF-8 **with BOM**
- `.vcxproj` 中已添加 `/source-charset:utf-8 /execution-charset:utf-8`，**不要移除**
- 如果新增源文件，确保保存为 UTF-8 without BOM

### 预处理器定义
```
SYR_VER=2
HAS_EXCEPTIONS=0
NOMINMAX
_CRT_SECURE_NO_WARNINGS
_WIN32_WINNT=0x0601
NTDDI_VERSION=0x06010000
```

### 代码风格
- 使用 `#pragma once` 而非 Include Guards
- 类名使用 `PascalCase` 并以 `Class` 结尾（如 `StackPushBuffClass`）
- 成员变量使用匈牙利前缀：`SITimerLeft`（SI = Save/Load 兼容字段），`SIExtraCode_A`，`SIEffectMode_0`
- 方法名使用 `PascalCase`（如 `OnEnterState_Active`, `EffectAI`, `Push`）
- 错误处理优先使用 `try-catch` 捕获 `SIException`
- 注释使用中文或中英双语

### 命名空间
- `Init` 命名空间用于初始化相关函数（如 `Init::Initialize()`）

### Syringe / 注入
- `.inj` 文件格式：`地址 = 符号名, 覆写字节数, 优先级`
- 当前使用 DllMain 直接调用 ECInitLibrary，无需 .inj 钩子

## 工作流

### 添加新 Buff
1. 在 `Source/` 下创建 `YourBuffClass.h` 和 `YourBuffClass.cpp`
2. 继承 `SIBuffClass`
3. 实现 `OnEnterState_Active()` 和 `EffectAI()`
4. 在 `Main.cpp` 的 `fnOrdered` 中用 `SIClassManager::RegisterBuff<YourBuffClass>("YourType")` 注册
