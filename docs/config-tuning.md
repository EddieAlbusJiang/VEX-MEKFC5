# 调参（config.h）

所有可调参数以宏集中在 `include/config.h`，经 `include/vex.h` 引入，全局可见；修改后需重新编译。

## Lift

| 宏 | 默认 | 含义 |
|---|---|---|
| `P_LIFT_MIN_ANGLE` | 0.0 | 底部限位（电机 deg） |
| `P_LIFT_MAX_ANGLE` | 2350.0 | 顶部限位（电机 deg） |
| `P_LIFT_SPEED` | 33.0 | 手动恒速（percent） |
| `P_LIFT_AUTO_SPEED` | 45.0 | 自动位置控制速度上限（percent） |
| `P_LIFT_AUTO_TOL` | 5.0 | 自动到位容差（电机轴 deg） |
| `P_LIFT_AUTO_TIMEOUT_MS` | 4500 | 单次升降超时（ms） |

## Clamp

| 宏 | 默认 | 含义 |
|---|---|---|
| `P_CLAMP_OPEN_ANGLE` | 90.0 | 张开限位（大角，deg） |
| `P_CLAMP_CLOSE_ANGLE` | 0.0 | 合紧基准限位（小角，deg；实际端点仍依原校准确定） |
| `P_CLAMP_SPEED` | 20.0 | 恒速（percent） |
| `P_CLAMP_TOLERANCE` | 1.0 | 手动位置锁定死区（deg） |
| `P_CLAMP_AUTO_TOL` | 2.0 | 自动关闭及中间角度目标到位容差（deg） |
| `P_CLAMP_AUTO_OPEN_TOL` | 6.0 | 自动完全张开目标容差（deg），按 Cp≈85 已释放的实车反馈设置 |
| `P_CLAMP_AUTO_TIMEOUT_MS` | 2000 | 单次夹爪动作超时（ms） |
| `P_CLAMP_GRIP_MAX_POSITION` | 6.0 | 带物夹取接触范围上界，相对闭合基准零点（deg） |
| `P_CLAMP_GRIP_MIN_TRAVEL` | 10.0 | 启用接触判定前实际收拢行程（deg） |
| `P_CLAMP_GRIP_STABLE_DEG` | 0.5 | 接触确认期间允许位置波动（deg） |
| `P_CLAMP_GRIP_SETTLE_MS` | 200 | 近闭合接触确认持续时间（ms），仅用于 AutoClampGrip |
| `P_AUTO_GRIP_PAUSE_MS` | 250 | 第二次夹取成功后保持等待，再开始升降和转向（ms） |
| `P_CLAMP_PID_KP/KD/KI` | 0.5 / 0.03 / 0.0 | 位置锁定 PID 增益（按秒计时） |
| `P_CLAMP_CALIB_SPEED` | 9.0 | 关闭角校准时的慢速（percent） |
| `P_CLAMP_CALIB_MS` | 1200 | 关闭角校准向关闭方向旋转时长（ms） |
| `P_CLAMP_CLOSE_CALIB_OFFSET` | (-3.0) | 校准补偿：`close_angle = 测量角 + 该值`（deg） |

## Odometry

| 宏 | 默认 | 含义 |
|---|---|---|
| `P_ODOM_COEFFICIENT` | 1 | 惯性陀螺刻度校正系数 |
| `P_ODOM_WHEEL_DIAMETER_CM` | 8.25 | 驱动轮直径（cm） |
| `P_ODOM_WHEEL_RATIO` | 1.0 | 电机到轮的外部传动比 |

> `cm_per_deg = π·轮径 / (360·比)`。

## XDrive

| 宏 | 默认 | 含义 |
|---|---|---|
| `P_XDRIVE_MAX_OUTPUT` | 0.75 | 手动、转向和接近目标时的基础输出比例（0..1） |
| `P_XDRIVE_MIN_OUTPUT` | 1.0 | idle-brake 进入阈值（percent） |
| `P_XDRIVE_HYSTERESIS` | 1.0 | idle-brake 退出滞回（percent） |

## Auto（底盘自动位移/旋转）

| 宏 | 默认 | 含义 |
|---|---|---|
| `P_AUTO_SPEED` | 100.0 | 默认速度上限（percent，随后受全局输出比例限制） |
| `P_AUTO_TRANSIT_MAX_OUTPUT` | 0.85 | 自动长距离平移输出上限（0..1） |
| `P_AUTO_TRANSIT_BLEND_START_CM` | 6.0 | 此误差以内恢复基础输出比例 |
| `P_AUTO_TRANSIT_BLEND_END_CM` | 20.0 | 此误差以外使用完整途中输出上限，两阈值之间线性过渡 |
| `P_AUTO_RETURN_TURN_SPEED` | 65.0 | 第一次放置后退完成的 90° 转向速度上限，其余仍为 45 |
| `P_AUTO_POS_TOL` | 1.0 | 位移到位容差（cm） |
| `P_AUTO_ANGLE_TOL` | 1.5 | 旋转到位容差（deg） |
| `P_AUTO_TIMEOUT_MS` | 5000 | 超时（ms） |
| `P_AUTO_SETTLE_MS` | 20 | 最终停靠点/转向目标容差内需稳定时长（ms） |
| `P_AUTO_PASS_MIN_OUTPUT` | 20.0 | `AutoMoveVia` 经过点接近时最低平移指令（percent，仍受速度上限限制） |
| `P_AUTO_MIN_OUTPUT` | 8.0 | 最小平移指令（percent，防末端爬行） |
| `P_AUTO_ROT_MIN_OUTPUT` | 7.0 | 接近目标时最低转向指令（percent，仍受该次速度上限限制） |
| `P_AUTO_POS_HYSTERESIS` | 0.5 | 已进入到位范围后的额外回弹余量（cm） |
| `P_AUTO_ANGLE_HYSTERESIS` | 0.5 | 已进入到位范围后的额外回弹余量（deg） |
| `P_AUTO_RUN_LIMIT_MS` | 30000 | 全程预算；到时中止并报超时，不算完成 |
| `P_AUTO_MOVE_PID_KP/KD/KI` | 2.5 / 0.06 / 0.0 | 纵向/横向 PID（按秒计时） |
| `P_AUTO_ROT_PID_KP/KD/KI` | 0.8 / 0.05 / 0.0 | 朝向 PID（按秒计时） |

## Auto 机构（Clamp/Lift 自动动作）

| 宏 | 默认 | 含义 |
|---|---|---|
| `P_MECH_AUTO_SETTLE_MS` | 60 | 进入到位范围后保持确认时长（ms） |
| `P_MECH_PROGRESS_MS` | 700 | 停滞检测窗口（ms） |
| `P_MECH_PROGRESS_DEG` | 0.5 | 检测窗口内至少应变化的电机轴角度 |
| `P_MECH_HYSTERESIS` | 2.0 | 到位后重新起动的附加余量（deg） |

自动机构采用电机内置位置控制。旧 `P_LIFT_TOL`、`P_MECH_TIMEOUT_MS`、`P_MECH_SETTLE_MS`、`P_MECH_DECEL_TOL`、`P_MECH_MIN_OUTPUT` 仅保留兼容，V3 自动控制不再使用。

## 驾驶输入

| 宏 | 默认 | 含义 |
|---|---|---|
| `P_DRIVE_CURVE_EXP` | 1.9 | 摇杆输入曲线指数 `k`：`f(x)=sign(x)·|x/100|^k·100`（k=1 线性；k>1 小输入更柔和） |

## Auto 命令队列

| 宏 | 默认 | 含义 |
|---|---|---|
| `P_AUTO_QUEUE` | 4 | 每个子系统非阻塞 Auto 的**命令队列容量**（worker 线程环形缓冲）；满则等待空位 |

## 其他常量

- `TICK_MS`（在 `include/vex.h`，默认 10）：Clamp 位置锁定的 tick 补偿使用。
- `driver-control` 内部常量：`kDeadZone`（摇杆死区）、`kScreenRefreshMs`（屏幕刷新周期）。
- `XDrive`：`kBrakeVelEps`（判定“刹停”的速度阈值）；`Odometry`：`kGravity`、`kAccelLpf`、ZUPT 阈值、标定采样数等。

## 调参提示

- 位移自动 PID 按秒计算；自动升降和夹爪改用电机内置位置控制，`P_CLAMP_PID_*` 只影响手动位置锁定。
- 停止精度：`P_AUTO_POS_TOL` / `P_AUTO_ANGLE_TOL` / `P_CLAMP_AUTO_TOL` / `P_LIFT_AUTO_TOL` 越大，允许的到位误差越大。
- 底盘速度：`P_XDRIVE_MAX_OUTPUT` 控制基础输出比例，自动平移远离目标时平滑过渡到 `P_AUTO_TRANSIT_MAX_OUTPUT`（不低于基础比例）。
- `speed` 参数与 `P_*_SPEED` 宏控制各自动流程的速度上限。
