# Clamp（夹爪）

`include/clamp.h` / `src/clamp.cpp`

单电机驱动齿轮组开合，且支持**位置锁定**（停在中间位置并抗漂移）。

## 结构与参数

- 限位：`open_angle_`（大角）与 `close_angle_`（小角）；默认 `P_CLAMP_OPEN_ANGLE` / `P_CLAMP_CLOSE_ANGLE`。
- 恒速 `speed_`（`P_CLAMP_SPEED`）。
- 停止一律 `hold`（支持中间锁死）。
- 到位/锁定容差 `tolerance_`（`P_CLAMP_TOLERANCE`）。

## 位置锁定

- `moveBy(direction)`：恒速旋转；按一个 `TICK_MS` 的位移**预测**并累加记录目标 `locked_position_`（`locked_position_ = position() + direction·Δ`）；到限位则停在限位。
- `stop()`：误差 `error = locked_position_ - position()`；
  - 超出 `tolerance_` 且未顶限位 → 用锁 PID（`P_CLAMP_PID_*`）驱动归位，输出上限为 `speed_`；
  - 否则 `hold`。
- 由于驱动控制每周期调用 `stop()`，形成带死区的持续位置保持。
- `TICK_MS` 定义在 `vex.h`（默认 10ms）。

> 说明：`moveBy` 内的 `Δ` 是对“下一 tick 位移”的估算（含经验系数），用于让记录目标贴近真实位置；如实际不匹配可整定。

## 手动接口

```cpp
double position() const;
bool   isOpen() const;
bool   isClosed() const;
void   open();      // 向 open_angle_（角度增大）
void   close();     // 向 close_angle_（角度减小）
void   stop();      // hold 或按锁定 PID 归位
void   resetPosition();          // 位置置 0，清锁定
void   setSpeed(double speed);
void   setLockTolerance(double tolerance);
void   setLockGains(double kP, double kD, double kI);
```

## 自动接口（阻塞）

```cpp
void AutoClampTo(double target_deg, double speed = P_CLAMP_SPEED);
void AutoClampFor(double delta_deg,  double speed = P_CLAMP_SPEED);
void AutoClampOpen(double speed = P_CLAMP_SPEED);   // → open_angle_
void AutoClampClose(double speed = P_CLAMP_SPEED);  // → close_angle_
```

## 自动接口（阻塞 / 非阻塞双签名）

```cpp
// 阻塞
void AutoClampTo(double target_deg, double speed = P_CLAMP_SPEED);
void AutoClampFor(double delta_deg, double speed = P_CLAMP_SPEED);
void AutoClampOpen(double speed = P_CLAMP_SPEED);
void AutoClampClose(double speed = P_CLAMP_SPEED);
// 非阻塞（回调倒数第二、可为 nullptr；speed 末参）
void AutoClampTo(double target_deg, void (*onDone)(void), double speed = P_CLAMP_SPEED);
void AutoClampFor(double delta_deg, void (*onDone)(void), double speed = P_CLAMP_SPEED);
void AutoClampOpen(void (*onDone)(void), double speed = P_CLAMP_SPEED);
void AutoClampClose(void (*onDone)(void), double speed = P_CLAMP_SPEED);
```

- 非阻塞版入队即返回，由本类 worker 线程执行，完成时调用 `onDone`；队列满则丢弃。
- ⚠️ 禁止在回调中调用阻塞版 Auto（死锁）。

内部 `runToAngle(target, speed)`（恒速 + 临近减速，不复用锁定 PID）：

1. 目标钳制到 `[close, open]`。
2. 暂存并 `setSpeed(speed)`。
3. 每 ~10ms：
   - `|err| <= tolerance_` → `hold` 并累计稳定（`P_MECH_SETTLE_MS`）后退出；
   - 处于“保持”且在 `tolerance_ + P_MECH_HYSTERESIS` 内 → 继续 `hold`（防边界抖动）；
   - 顶限位 → 停；
   - 否则按误差符号驱动，速度经 `approachSpeed()` **临近减速**（`P_MECH_DECEL_TOL`）且不低于 `P_MECH_MIN_OUTPUT`。
4. 结束 `hold`；把 `locked_position_ = position()`（保持后续手动 `stop()` 一致）；恢复原速度。

## 关闭角校准

- `setCloseAngle(double angle)`：**纯设置**关闭限位（**钳制为不大于预定义 `P_CLAMP_CLOSE_ANGLE`**，防校准错误），**不驱动夹爪**（仅同步锁定目标、清 PID）。
- `calibrateCloseAngle()`（开机在 `initialize()` 调用）：以 `P_CLAMP_CALIB_SPEED`（9%）向关闭方向转 `P_CLAMP_CALIB_MS`(1200ms)，**期间绕过关闭软限位**；然后 `setCloseAngle(position() + P_CLAMP_CLOSE_CALIB_OFFSET)`（补偿 -3），随后**在此处**开启回 `open_angle_` 并同步锁定/清 PID。
- 头尾各输出一条 Info 日志：`[Clamp] close-angle calibration start` / `... done: measured=.. offset=.. close=..`。

## 相关宏（`config.h`）

`P_CLAMP_OPEN_ANGLE`、`P_CLAMP_CLOSE_ANGLE`、`P_CLAMP_SPEED`、`P_CLAMP_TOLERANCE`、`P_CLAMP_PID_KP/KD/KI`、`P_MECH_TIMEOUT_MS`、`P_MECH_SETTLE_MS`、`P_MECH_DECEL_TOL`、`P_MECH_MIN_OUTPUT`、`P_MECH_HYSTERESIS`。

## 备注

- 开机前手动把夹爪置于张开；`initialize()` 中调用 `assumeOpen()`，把当前角度读数**视为最大角**（`open_angle_`），并同步锁定目标、清 PID/lock。
- 停止模式固定 hold（满足“中间位置锁死”需求）。
- **Root 模式（`ButtonLeft`）**：`Root_Mode` 为真时，**手动** `moveBy`/`stop` 忽略软限位（可越过限位）；退出该模式时 `resetPosition()`。自动 `runToAngle` 不受影响。
