# XDrive（45° 全向底盘）

`include/x-drive.h` / `src/x-drive.cpp`

## 结构

四个驱动轮按 45° 斜置、轮轴指向车辆中心，可组合出任意方向平移与原地旋转（holonomic）。

输入约定（均为 percent，正方向）：

- `forward`：+ 前进，− 后退
- `lateral`：+ 右移，− 左移
- `rotational`：+ 顺时针，− 逆时针

## 运动学混合

```
L1 = f + l + r
L2 = f - l + r
R1 = f - l - r
R2 = f + l - r
```

- 该式与四轮几何一致：前进=(+1,+1,+1,+1)、右移=(+1,−1,−1,+1)、顺时针=(+1,+1,−1,−1)。
- `drive()` 先按 `forward_gain_/lateral_gain_/rotational_gain_` 缩放各轴（默认 1.0，可用 `setGains` 调），再做混合。

## 输出缩放

- 若四轮中峰值超过 100，则**等比缩小**（保持方向），再乘全局上限 `max_output_ = P_XDRIVE_MAX_OUTPUT`（默认 0.5）。
- 即实际峰值 ≈ 70%（可调）。

## idle-brake（怠速锁死）

- 每轮输出绝对值低于 `min_output_ = P_XDRIVE_MIN_OUTPUT` 时，对该轮 `stop(hold)`（由 `applyMotor` 处理），高于 `min_output_ + hysteresis_`（`P_XDRIVE_HYSTERESIS`）才恢复，避免阈值附近抖动。
- `setIdleBrake(bool)` 可整体开关（默认开）。
- `wheelOutput(index)`：返回每轮最近一次**实际下发值**（被锁死时为 0）。
- 说明：autonomous 期间由 `autonomous()` 首尾 `setIdleBrake(false/true)` 关闭，避免逐轮锁死扭曲 holonomic 平移。

## 刹车状态

- `isBraking()`：当四轮实测速度（`velocity(pct)`）都 ≈0（`< kBrakeVelEps`）时为真；`stop(type)` 按 `type` 设定（`coast` 为假，`brake/hold` 为真）。
- 该标志被 `Odometry` 用于“刹车时暂停 IMU 积分”（跨线程 `volatile` 读）。

## 手动控制

```cpp
void drive(double forward, double lateral, double rotational);
void stop(vex::brakeType type = vex::brakeType::coast);
```

## 自动位移/旋转（阻塞）

## 自动位移/旋转（阻塞 / 非阻塞双签名）

目标坐标位于**里程计坐标系**（开机系，原点=开机位置，cm）。

**阻塞版（无回调）**
```cpp
void AutoMoveTo(double target_x, double target_y, double speed = P_AUTO_SPEED);
void AutoMoveFor(double diff_x, double diff_y, double speed = P_AUTO_SPEED);
void AutoRotateTo(double target_deg, double speed = P_AUTO_SPEED);
void AutoRotateFor(double diff_deg, double speed = P_AUTO_SPEED);
```
**非阻塞版（回调为倒数第二参、必填，可为 `nullptr`；`speed` 末参）**
```cpp
void AutoMoveTo(double target_x, double target_y, void (*onDone)(void), double speed = P_AUTO_SPEED);
void AutoMoveFor(double diff_x, double diff_y, void (*onDone)(void), double speed = P_AUTO_SPEED);
void AutoRotateTo(double target_deg, void (*onDone)(void), double speed = P_AUTO_SPEED);
void AutoRotateFor(double diff_deg, void (*onDone)(void), double speed = P_AUTO_SPEED);
```
- **非阻塞**：入队后立即返回；由**本类的 worker 线程**执行；完成时调用 `onDone`（若非空）。队列满则**丢弃**。相对动作 `*For` 在**执行时**以当前里程计位姿为基准。
- **阻塞**：同样经队列（入队 + 票据等待），由 worker 执行；因此天然与队列串行、可排队。
- ⚠️ **禁止在回调中调用阻塞版 Auto**（回调运行于 worker 线程，会造成死锁）。本项目无重入场景。

**特快位移（无 PID，仅阻塞）**
```cpp
void AutoFastMoveFor(double diff_x, double diff_y);
```
- 沿**世界系**相对位移 `(diff_x, diff_y)` 以**允许的最大速度**移动（上限 = `P_XDRIVE_MAX_OUTPUT × 100%`，与传值 >100 无关）；**无 PID、无减速、不保持朝向**（`rot=0`）。
- 判定：`|剩余| < P_AUTO_POS_TOL` 即停（或 `P_AUTO_TIMEOUT_MS` 超时）；结束 `stop(hold)`。
- 经命令队列由 worker 执行（与其它 Auto 串行）。

`runMove(tx,ty,speed)`：

1. 记录进入时朝向 `entry_heading`。
2. 误差 `ex=tx-wheelX, ey=ty-wheelY`；按当前 `heading` 旋到车体系：`bf=ex·c+ey·s`、`bl=-ex·s+ey·c`。
3. `pid_move_forward`、`pid_move_lateral`（`P_AUTO_MOVE_PID_*`）分别对 `bf/bl` 输出；`pid_rot`（`P_AUTO_ROT_PID_*`）对 `degNorm180(entry_heading-heading)` 输出以**保持进入朝向**。
4. 三路输出限幅到 `speed` 后 `drive()`。
5. 退出：`|ex|,|ey| < P_AUTO_POS_TOL` 持续 `P_AUTO_SETTLE_MS`，或 `P_AUTO_TIMEOUT_MS` 超时；结束 `stop(hold)`。

加速末端：到位容差内**平移置零**（仅保留朝向修正，避免 settle 期间爬行）；否则若平移指令幅值低于 `P_AUTO_MIN_OUTPUT`，则**按幅值等比抬到下限**（保持方向），避免末端趋零而爬行。

`runRotate(t_deg,speed)`：`pid_rot` 对 `degNorm180(t_deg-heading)` 输出，`drive(0,0,rot)`，`|err|<P_AUTO_ANGLE_TOL` 稳定或超时退出，结束 `stop(hold)`。

- 相对版：`AutoMoveFor→runMove(wheelX+dx, wheelY+dy)`；`AutoRotateFor→runRotate(heading+ddeg)`。
- 位置来源：**轮式里程计**（`wheelX/wheelY`）。
- `speed` 为 PID 输出上限（percent），默认 `P_AUTO_SPEED`；`drive()` 内还会乘 `max_output_`。
- `pid_move_forward/pid_move_lateral/pid_rot` 为 `x-drive.cpp` 匿名命名空间的**文件级静态对象**，跨多步复用，每次进入 `reset()`；并 `setUseSeconds(true)`（按秒计时，使 `kD` 可作为阻尼）。

## 相关宏（`config.h`）

`P_XDRIVE_MAX_OUTPUT`、`P_XDRIVE_MIN_OUTPUT`、`P_XDRIVE_HYSTERESIS`、`P_AUTO_SPEED`、`P_AUTO_POS_TOL`、`P_AUTO_ANGLE_TOL`、`P_AUTO_TIMEOUT_MS`、`P_AUTO_SETTLE_MS`、`P_AUTO_MIN_OUTPUT`、`P_AUTO_MOVE_PID_*`、`P_AUTO_ROT_PID_*`、`P_AUTO_QUEUE`。
