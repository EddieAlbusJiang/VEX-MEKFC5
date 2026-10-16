# Lift（升降机）

`include/lift.h` / `src/lift.cpp`

绞盘牵引负载升降，**两个电机同步**（`vex::motor_group`）。

## 结构与参数

- 软限位 `[min_angle_, max_angle_]`（电机轴角度，deg），构造时自动排序；默认来自 `P_LIFT_MIN_ANGLE` / `P_LIFT_MAX_ANGLE`。
- 恒速运动，速度 `speed_`（percent，`P_LIFT_SPEED`）。
- 停止一律 `hold`（负载可悬停）。
- 位置反馈取 `motor_group.position()`（第一个电机）。

## 手动接口

```cpp
double position() const;
bool   atBottom() const;   // <= min_angle_
bool   atTop() const;      // >= max_angle_
void   up();               // 向 max，恒速；到顶停
void   down();             // 向 min，恒速；到底停
void   stop();             // hold
void   resetPosition();    // 当前角度置 0（开机复位）
void   setSpeed(double speed); // 钳制 [0,100]
```

## 自动接口（阻塞）

```cpp
void AutoLiftTo(double target_deg, double speed = P_LIFT_SPEED);
void AutoLiftFor(double delta_deg,  double speed = P_LIFT_SPEED);
void AutoLiftUp(double speed = P_LIFT_SPEED);    // → max_angle_
void AutoLiftDown(double speed = P_LIFT_SPEED);  // → min_angle_
```

内部 `runToAngle(target, speed)`：

1. 目标钳制到 `[min, max]`。
2. 暂存并 `setSpeed(speed)`。
3. 每 ~10ms：
   - `|err| <= P_LIFT_TOL` → `hold` 并累计稳定（`P_MECH_SETTLE_MS`）后退出；
   - 保持中且在 `P_LIFT_TOL + P_MECH_HYSTERESIS` 内 → 继续 `hold`（防边界抖动）；
   - 顶限位 → 停；
   - 否则按误差符号驱动，速度经 `approachSpeed()` **临近减速**（`P_MECH_DECEL_TOL`）且不低于 `P_MECH_MIN_OUTPUT`。
4. 结束 `stop()`（hold），恢复原速度。

## 相关宏（`config.h`）

`P_LIFT_MIN_ANGLE`、`P_LIFT_MAX_ANGLE`、`P_LIFT_SPEED`、`P_LIFT_TOL`、`P_MECH_TIMEOUT_MS`、`P_MECH_SETTLE_MS`、`P_MECH_DECEL_TOL`、`P_MECH_MIN_OUTPUT`、`P_MECH_HYSTERESIS`。

## 备注

- 开机前需手动把负载放到底部，`initialize()` 中 `resetPosition()` 建立零点。
- `AutoLiftFor`：`target = position() + delta`。
- **Root 模式（`ButtonLeft`）**：`Root_Mode` 为真时，**手动** `moveBy`（`up/down`）忽略软限位；退出该模式时 `resetPosition()`。自动 `runToAngle` 不受影响。
