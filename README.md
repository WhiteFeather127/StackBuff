# StackBuffs - 栈操作 Buff 系统

基于 WIC (WinterIsComing) 框架的栈操作 Buff 集合，用于在游戏运行时管理单位栈并进行栈顶操作。

## StackPush <主动>

位于 `rulesmd.ini`：

```ini
[SomeBuffType]
Effect.Type=StackPush
```

将挂载了此 Buff 的单位按阵营加入指定数字栈（LIFO），激活时入栈，消失时自动出栈。

### 效果种类相关属性

```ini
[SomeBuffType]
Effect.ExtraCodeA=0    ; 整数, 栈 ID, 不同 ID 表示不同的栈, 默认 0
```

### 说明

- 栈存储的是 **buff 实例**，通过 buff 可获取所属单位
- After / Remove 双保险，保证无论 Buff 以何种方式消失都出栈
- Save/Load：保存单位指针，读档后首次访问时惰性重建 buff 指针

## StackTop <主动>

位于 `rulesmd.ini`：

```ini
[SomeBuffType]
Effect.Type=StackTop
```

查找指定栈的栈顶单位，通过 SDK 接口为其附加若干 Buff。支持循环触发和弹出模式。

### 效果种类相关属性

```ini
[SomeBuffType]
Effect.ExtraCodeA=0        ; 整数, 栈 ID, 默认 0
Effect.AcceptBuffs=        ; Buff 列表, 给栈顶单位附加的 Buff
Effect.Modes=0             ; 整数, 弹出模式, 默认 0
                           ; 0 = 仅附加, 不弹出
                           ; 1 = 附加后弹出栈顶（结束栈顶单位的 StackPush）
Effect.Counts=-1            ; 整数, 触发次数, 默认 -1
Effect.Delay=0             ; 整数, 触发间隔（帧数）, 0 = 每帧触发, 默认 0
```

### 说明

- 通过栈顶的 **buff 实例** 获取所属单位和执行出栈，无需按类型字符串查找
- 出栈时直接调用 `buff->TryAfter()`，路径更短

## 构建

运行 `build.bat` 即可编译 Release/Win32 版本，输出 `StackBuffs.dll`。

## 注入

将 `Release/StackBuffs.dll` 复制到游戏目录的 `Patches/` 文件夹中，SyringeIH 会在游戏启动时自动加载。

