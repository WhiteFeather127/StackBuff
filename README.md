# Buff效果-杂项

## StackPush <主动>

将挂载了此 Buff 的单位按阵营加入指定数字栈（LIFO），挂载时入栈，移除时自动出栈，指针失效时清理栈中的失效指针。

### 效果种类相关属性

位于 `rulesmd.ini`：

```ini
[SomeBuffType]
; 效果种类相关属性
Effect.Type=StackPush                      ; 效果种类
Effect.ExtraCodeA=0                        ; 整数 , 栈 ID , 不同的栈 ID 表示不同的栈 , 默认值是 0
```

### 效果强度值转换

不涉及。

### 广播与监听相关属性

不支持广播或监听。

### 数值显示相关属性

不支持数值显示。

---

## StackTopBuff <主动>

查找指定栈的栈顶单位，通过 SDK 接口为其附加若干 Buff，激发时执行。支持弹出模式，附加后可选择将栈顶单位弹出栈。

### 效果种类相关属性

位于 `rulesmd.ini`：

```ini
[SomeBuffType]
; 效果种类相关属性
Effect.Type=StackTopBuff                   ; 效果种类
Effect.ExtraCodeA=0                        ; 整数 , 栈 ID , 默认值是 0
Effect.AcceptBuffs=                        ; Buff 列表 , 给栈顶单位附加的 Buff 列表
Effect.Modes=0                             ; 整数 , 【弹出模式】 , 无效值默认为 0 , 默认值是 0
                                           ; 0 = 仅附加 , 不弹出栈顶
                                           ; 1 = 附加后弹出栈顶（移除栈顶单位的 StackPush Buff）
```

### 效果强度值转换

不涉及。`Effect.Modes` 用于控制弹出模式而非效果强度转换。

### 广播与监听相关属性

不支持广播或监听。

### 数值显示相关属性

不支持数值显示。

---

## 完整示例

```ini
[BuffTypes]
0=Q1Marker                                 ; StackPush - 标记单位入栈(1号栈)
1=Q1Dispatcher                             ; StackTopBuff - 给1号栈顶单位附加 Buff
2=PowerUp60                                ; 游戏自带效果 - 60帧内攻击力提升150%

[Q1Marker]
Effect.Type=StackPush
Effect.ExtraCodeA=1

[Q1Dispatcher]
Effect.Type=StackTopBuff
Effect.ExtraCodeA=1
Effect.AcceptBuffs=PowerUp60
Effect.Modes=0

[PowerUp60]
Effect.Type=PropMultAttack
Effect.Modes=150
Power.Bases=0.5
Duration=60

; 通过弹头挂载 Q1Marker
[Q1MarkerWH]
CellSpread=0
BuffAdd.Types=Q1Marker
```
