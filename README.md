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

查找指定栈的栈顶单位，通过 SDK 接口为其附加若干 Buff。支持范围影响、循环触发和弹出模式。

### 效果种类相关属性

```ini
[SomeBuffType]
Effect.ExtraCodeA=0        ; 整数, 栈 ID, 默认 0
; 通用效果属性
Effect.Anims=              ; 两个动画, 【生效时在自己身上播放的动画】【生效时在受影响单位身上播放的动画】, 不写就不显示动画
Effect.AcceptBuffs=        ; Buff 列表, 挂载这些 Buff, 没设置就不挂载
Effect.Counts=-1           ; 整数, 生效次数, 负数 = 无限次, 0 = 无法生效并直接结束, 默认 -1
Effect.Delay=0             ; 整数, 每隔这么多帧生效一次, 小于 0 按 0 算, 默认 0, 单位: 帧
Effect.Range=0             ; 浮点数, 影响的范围(半径), 0 = 只影响栈顶单位, 大于 0 = 影响范围内所有单位, 默认 0, 单位: 格子
Effect.Modes=0             ; 整数, 弹出模式, 默认 0
                           ; 0 = 仅附加, 不弹出
                           ; 1 = 附加后弹出栈顶（结束栈顶单位的 StackPush）
```

### 说明

- 通过栈顶的 **buff 实例** 获取所属单位和执行出栈，无需按类型字符串查找
- 出栈时直接调用 `buff->TryAfter()`，路径更短
- `Effect.Range > 0` 时使用 WIC 的 `ForEach_Techno` 遍历全图单位，按距离筛选后挂载

## 构建

运行 `build.bat` 即可编译 Release/Win32 版本，输出 `StackBuffs.dll`。

## 注入

将 `Release/StackBuffs.dll` 复制到游戏目录的 `Patches/` 文件夹中，SyringeIH 会在游戏启动时自动加载。

