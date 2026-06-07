# MyWICBuffs — 自定义 Buff 效果类型

基于 **WIC (SIWinterIsComing) V0.1.0 SDK** 实现的两个自定义 Buff 效果类型。

---

## 注册

```ini
[BuffTypes]
0=MyStackPush
1=MyStackTopApplier
```

---

## StackPush（入栈标记）

将挂载了此 Buff 的单位按阵营加入指定数字栈（LIFO）。Buff 移除时自动出栈。

```ini
[MyStackPush]
Effect.Type=StackPush
Effect.ExtraCodeA=1       ; 栈 ID
```

---

## StackTopBuff（栈顶附加 Buff）

查找指定栈的栈顶单位，通过 SDK 接口为其附加若干 Buff。支持弹出模式。

```ini
[MyStackTopApplier]
Effect.Type=StackTopBuff
Effect.ExtraCodeA=1              ; 栈 ID
Effect.AcceptBuffs=PowerUpBuff   ; 给栈顶附加的 Buff
Effect.Modes=0                   ; 0=仅附加, 1=附加后弹出栈顶
```

---

## 完整示例

```ini
[BuffTypes]
0=Q1Marker
1=Q1Dispatcher
2=PowerUp60

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

[Q1MarkerWH]
CellSpread=0
BuffAdd.Types=Q1Marker
```
