# Driver Control（手动控制）

`include/driver-control.h` / `src/driver-control.cpp`

在 `main()` 中注册：`Competition.drivercontrol(driverControl)`；同时 `main()` 也直接调用了 `driverControl()` 以在无场控时手动调试。

## 主循环

```
serviceHeadingReset()
updateChassis()
updateLift()
updateClamp()
updateControllerScreen()
（printf 机构/惯导遥测）
sleep 10ms
```

## 按钮映射

| 按键 | 功能 |
|---|---|
| `ButtonX` | 切换驾驶模式（HeadingUp ⇄ NorthUp），切换时自动复位朝向 |
| `ButtonY` | 复位朝向（当前朝向→0） |
| `ButtonA` | 轴锁定 开/关 |
| `ButtonB` | 调试：跑一遍 `autonomous()`（运行中忽略） |
| `ButtonUp` / `ButtonDown` | 升降 上升 / 下降 |
| `ButtonR2` / `ButtonR1` | 夹爪 合紧 / 张开 |

- 按键回调通过 `pressed()` 注册（一次）。回调线程与主循环共享状态用 `drive_mode_mutex` 保护。
- 传感器复位（`Odom.resetRotation()`）不在回调线程直接执行，而是**置位请求**（`heading_reset_requested`），由主循环 `serviceHeadingReset()` 执行。

## 驾驶模式

- **HeadingUp（模式 1，车头朝上）**：摇杆 `forward/lateral` 直接按车体系下发（默认）。
- **NorthUp（模式 2，北朝上）**：把摇杆视为“开机系（北=heading 0）”的意图，按当前 `heading` 旋到车体系后再下发：
  ```
  bf = f·cosθ + l·sinθ
  bl = -f·sinθ + l·cosθ
  ```
- `Axis1`（转向）不受模式影响。

## 输入

- `Axis3`：前后；`Axis4`：横移；`Axis1`：原地转向（均经 `deadZone`，阈值 `kDeadZone`）。

## 轴锁定（Button A）

- 启用后，在**完成 NorthUp 变换之后**，只保留绝对值较大的一路平移分量，另一路清零（平局保留前向）：
  ```
  if (|body_forward| >= |body_lateral|) body_lateral = 0; else body_forward = 0;
  ```
- 使小车以“横平竖直”的轨迹运动；`rotational` 不受影响。

## 控制器屏幕（Throttled ~150ms）

| 行 | 格式 | 内容 |
|---|---|---|
| Row1 | `Lf%02d Lb%02d Rf%02d Rb%02d` | 四轮最终下发速度（|pct|，≤99） |
| Row2 | `Lift%4d  Clamp%4d` | 升降/夹爪角度（deg） |
| Row3 | `Man %4d,%4d %c%c%03d` | 轮式里程计 x,y（cm，钳制 4 字符）+ 轴锁定 `A`/空格 + 驾驶模式 `X`/`Y` + 秒（`%03d`） |

- 每个字段定宽，直接覆盖旧字符，无需清屏。
- Row3 长度 19 列，符合 Controller 屏幕宽度。

## 备注

- **自动阶段互斥**：当 `Auto_Active`（自动阶段进行中）为真时，主循环**跳过** `serviceHeadingReset/updateChassis/updateLift/updateClamp`，仅刷新屏幕，避免与 `autonomous()` 抢占硬件。
- 驾驶模式与轴锁定是**两个独立设定**，仅共用同一把互斥锁。
- `updateLift/updateClamp` 在无按键时调用各机构 `stop()`（升降 hold；夹爪 hold/锁定）。
