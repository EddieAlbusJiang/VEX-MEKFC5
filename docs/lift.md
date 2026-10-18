# Lift（升降机）

`include/lift.h` / `src/lift.cpp`

绞盘牵引负载升降，**两个电机同步**（`vex::motor_group`）。

## 结构与参数

- 软限位 `[min_angle_, max_angle_]`（电机轴角度，deg），构造时自动排序；默认来自 `P_LIFT_MIN_ANGLE` / `P_LIFT_MAX_ANGLE`。
- 手动恒速运动，速度 `speed_`（percent，`P_LIFT_SPEED`）；自动使用内置位置控制。
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

## 自动接口（阻塞 / 非阻塞双签名）

```cpp
// 阻塞
AutoResult AutoLiftTo(double target_deg, double speed = P_LIFT_AUTO_SPEED);
AutoResult AutoLiftFor(double delta_deg, double speed = P_LIFT_AUTO_SPEED);
AutoResult AutoLiftUp(double speed = P_LIFT_AUTO_SPEED);
AutoResult AutoLiftDown(double speed = P_LIFT_AUTO_SPEED);
// 非阻塞（回调倒数第二、可为 nullptr；speed 末参）
void AutoLiftTo(double target_deg, void (*onDone)(void), double speed = P_LIFT_AUTO_SPEED);
void AutoLiftFor(double delta_deg, void (*onDone)(void), double speed = P_LIFT_AUTO_SPEED);
void AutoLiftUp(void (*onDone)(void), double speed = P_LIFT_AUTO_SPEED);
void AutoLiftDown(void (*onDone)(void), double speed = P_LIFT_AUTO_SPEED);
```

- 非阻塞版入队即返回，由本类 worker 线程执行，完成时调用 `onDone`；队列满则等待空间。
- ⚠️ 禁止在回调中调用阻塞版 Auto（死锁）。

内部 `runToAngle` 统一调用 `runMechanismPosition`：使用电机组内置位置控制，自动速度默认 `P_LIFT_AUTO_SPEED=45`，不影响手动 33% 速度。进入电机轴 5° 到位范围后保持，允许额外 2° 回弹，稳定 60ms 后完成。持续 700ms 未移动至少 0.5° 返回 `Stalled`，总动作超时为 4500ms；自动阶段取消会立即退出并保持。超时或卡住不会作为到位成功。

阻塞接口返回 `AutoResult`，`waitUntilDone()` 等待已有指令而不重复提交目标。异步升降与底盘可并行。日志显示 `[Lift]` 结果、耗时以及目标 `T`、实际 `Lt`。

## 相关宏（`config.h`）

`P_LIFT_MIN_ANGLE`、`P_LIFT_MAX_ANGLE`、`P_LIFT_SPEED`、`P_LIFT_AUTO_SPEED`、`P_LIFT_AUTO_TOL`、`P_LIFT_AUTO_TIMEOUT_MS`、`P_MECH_AUTO_SETTLE_MS`、`P_MECH_HYSTERESIS`、`P_MECH_PROGRESS_MS/DEG`。

## 备注

- 开机前需手动把负载放到底部，`initialize()` 中 `resetPosition()` 建立零点。
- `AutoLiftFor`：`target = position() + delta`。
- **Root 模式（`ButtonLeft`）**：`Root_Mode` 为真时，**手动** `moveBy`（`up/down`）忽略软限位；ROOT 下右键才执行机构 `resetPosition()`。自动 `runToAngle` 始终遵守软限位。
