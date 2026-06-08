# StackBuffs - 栈操作 Buff 系统

基于 WIC (WinterIsComing) 框架的栈操作 Buff 集合，用于在游戏运行时管理单位栈并进行栈顶操作。

## StackPush <主动>

位于 `rulesmd.ini`：

```ini
[SomeBuffType]
Effect.Type=StackPush
```

将挂载了此 Buff 的单位按阵营加入指定数字栈（LIFO），挂载时入栈，移除时自动出栈，指针失效时清理栈中的失效指针。

### 效果强度值转换：0 项参数

效果强度值对此效果种类无效。

### 效果种类相关属性

位于 `rulesmd.ini`：

```ini
[SomeBuffType]
; 效果种类相关属性
Effect.ExtraCodeA=0                        ; 整数 , 栈 ID , 不同的栈 ID 表示不同的栈 , 默认值是 0
```

### 广播与监听相关属性

不支持广播或监听。

### 数值显示相关属性

不支持数值显示。

## StackTopBuff <主动>

位于 `rulesmd.ini`：

```ini
[SomeBuffType]
Effect.Type=StackTopBuff
```

查找指定栈的栈顶单位，通过 SDK 接口为其附加若干 Buff，激发时执行。支持弹出模式，附加后可选择将栈顶单位弹出栈。

### 效果强度值转换：0 项参数

效果强度值对此效果种类无效。

### 效果种类相关属性

位于 `rulesmd.ini`：

```ini
[SomeBuffType]
; 效果种类相关属性
Effect.ExtraCodeA=0                        ; 整数 , 栈 ID , 默认值是 0
Effect.AcceptBuffs=                        ; Buff 列表 , 给栈顶单位附加的 Buff 列表
Effect.Modes=0                             ; 整数 , 【弹出模式】 , 无效值默认为 0 , 默认值是 0
                                           ; 0 = 仅附加 , 不弹出栈顶
                                           ; 1 = 附加后弹出栈顶（移除栈顶单位的 StackPush Buff）
Effect.Counts=1                            ; 整数 , 触发次数 , 默认值是 1
Effect.Delay=0                             ; 整数 , 触发间隔（帧数）, 0 表示每帧触发 , 默认值是 0
```

### 广播与监听相关属性

不支持广播或监听。

### 数值显示相关属性

不支持数值显示。

## 构建

运行 `build.bat` 即可编译 Release/Win32 版本，输出 `StackBuffs.dll`。

## 注入

编译后，将 `Patches/StackBuffs.dll.inj` 和 `Release/StackBuffs.dll` 复制到游戏目录下的 `Patches/` 文件夹中，
SyringeIH 会在游戏启动时自动加载本 DLL。
