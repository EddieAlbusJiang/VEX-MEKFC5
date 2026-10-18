# Clamp（夹爪）

`include/clamp.h` / `src/clamp.cpp`

单电机驱动齿轮组开合，且支持**位置锁定**（停在中间位置并抗漂移）。

## 结构与参数

- 限位：`open_angle_`（大角）与 `close_angle_`（小角）；默认 `P_CLAMP_OPEN_ANGLE` / `P_CLAMP_CLOSE_ANGLE`。
- 手动恒速 `speed_`（`P_CLAMP_SPEED`）；自动使用内置位置控制。
- 停止一律 `hold`（支持中间锁死）。
- 手动锁定容差 `tolerance_`（`P_CLAMP_TOLERANCE`）；自动到位使用 `P_CLAMP_AUTO_TOL`。
- `isOpen()` 判断 `position() >= open_angle_ - tolerance_`；`isClosed()` 判断 `position() <= close_angle_ + tolerance_`。中间位置不会同时判为打开和关闭。

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
AutoResult AutoClampTo(double target_deg, double speed = P_CLAMP_SPEED);
AutoResult AutoClampFor(double delta_deg,  double speed = P_CLAMP_SPEED);
AutoResult AutoClampOpen(double speed = P_CLAMP_SPEED);   // → open_angle_
AutoResult AutoClampClose(double speed = P_CLAMP_SPEED);  // → close_angle_
```

## 自动接口（阻塞 / 非阻塞双签名）

```cpp
// 阻塞
AutoResult AutoClampTo(double target_deg, double speed = P_CLAMP_SPEED);
AutoResult AutoClampFor(double delta_deg, double speed = P_CLAMP_SPEED);
AutoResult AutoClampOpen(double speed = P_CLAMP_SPEED);
AutoResult AutoClampClose(double speed = P_CLAMP_SPEED);
// 非阻塞（回调倒数第二、可为 nullptr；speed 末参）
void AutoClampTo(double target_deg, void (*onDone)(void), double speed = P_CLAMP_SPEED);
void AutoClampFor(double delta_deg, void (*onDone)(void), double speed = P_CLAMP_SPEED);
void AutoClampOpen(void (*onDone)(void), double speed = P_CLAMP_SPEED);
void AutoClampClose(void (*onDone)(void), double speed = P_CLAMP_SPEED);
```

- 非阻塞版入队即返回，由本类 worker 线程执行，完成时调用 `onDone`；队列满则等待空间。
- ⚠️ 禁止在回调中调用阻塞版 Auto（死锁）。

内部 `runToAngle(target, speed)` 使用 V5 电机自带位置控制：

1. 目标钳制到 `[close, open]`，只提交一次非阻塞 `spinToPosition`。
2. 完全张开目标使用 `P_CLAMP_AUTO_OPEN_TOL`（6°），依据实车 `Cp≈85` 已完成释放的反馈；目标仍为 90°。关闭及中间角度目标保留 `P_CLAMP_AUTO_TOL`（2°）。进入相应到位范围后停止保持，在附加 `P_MECH_HYSTERESIS`（2°）范围内稳定 `P_MECH_AUTO_SETTLE_MS`（60ms）后完成。
3. 若回弹超出保持范围，重新提交位置目标；从未进入到位范围不能判为成功。不会用固定最低速度反复正反转。
4. 超时、持续无位移或自动阶段取消都停止电机，分别返回 `Timeout/Stalled/Cancelled`。
5. 保存实际位置作为之后手动保持的目标。日志显示结果、耗时、请求值 `req`、有效目标 `T` 和实际 `Cp`。

所有自动开、关、绝对角度、相对角度接口（含异步版本）共用此判断。阻塞接口返回 `AutoResult`；`waitUntilDone()` 等待已排队的动作，不重复移动。手动位置保持仍使用原 PID，自动到位判断与手动锁定容差分别设置。

## 带物夹取

第二次夹取使用专用阻塞接口 `AutoClampGrip(target, speed=P_CLAMP_SPEED)`，在原软限位内向关闭目标运动。正常到达角度仍直接成功；若物品阻止继续闭合，则必须同时满足：已实际收拢至少 10°、读数在有效目标减 2°到手动零点上方 6°之间、波动不超过 0.5°持续 200ms。确认期间保持原关闭指令，随后停止保持，并记 `[Clamp] CONTACT`。

这个判定依据本次 Cp≈3、有物品时停止但空夹正常的实车反馈，仅推断接触，需实测夹持是否牢固。开爪、普通 `AutoClampTo/Close` 和开机校准不启用此规则；较大开口处堵住、没有闭合行程、超时及取消仍返回失败。

## 关闭角校准

- `setCloseAngle(double angle)`：**纯设置**关闭限位（**钳制为不大于预定义 `P_CLAMP_CLOSE_ANGLE`**，防校准错误），**不驱动夹爪**（仅同步锁定目标、清 PID）。
- `calibrateCloseAngle()`（开机在 `initialize()` 调用）：以 `P_CLAMP_CALIB_SPEED`（9%）向关闭方向转 `P_CLAMP_CALIB_MS`(1200ms)，**期间绕过关闭软限位**；然后 `setCloseAngle(position() + P_CLAMP_CLOSE_CALIB_OFFSET)`（补偿 -3），随后**在此处**开启回 `open_angle_` 并同步锁定/清 PID。
- 头尾各输出一条 Info 日志：`[Clamp] close-angle calibration start` / `... done: measured=.. offset=.. close=..`。

## 相关宏（`config.h`）

`P_CLAMP_OPEN_ANGLE`、`P_CLAMP_CLOSE_ANGLE`、`P_CLAMP_SPEED`、`P_CLAMP_TOLERANCE`、`P_CLAMP_PID_KP/KD/KI`、`P_CLAMP_AUTO_TOL`、`P_CLAMP_AUTO_TIMEOUT_MS`、`P_MECH_AUTO_SETTLE_MS`、`P_MECH_HYSTERESIS`、`P_MECH_PROGRESS_MS/DEG`。

## 备注

- 开机前手动把夹爪置于张开；`initialize()` 中调用 `assumeOpen()`，把当前角度读数**视为最大角**（`open_angle_`），并同步锁定目标、清 PID/lock。
- 停止模式固定 hold（满足“中间位置锁死”需求）。
- **Root 模式（`ButtonLeft`）**：`Root_Mode` 为真时，**手动** `moveBy`/`stop` 忽略软限位（可越过限位）；ROOT 下右键才执行机构 `resetPosition()`。自动 `runToAngle` 始终遵守软限位。
