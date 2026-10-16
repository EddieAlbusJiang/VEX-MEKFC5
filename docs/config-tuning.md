# 调参（config.h）

所有可调参数以宏集中在 `include/config.h`，经 `include/vex.h` 引入，全局可见；修改后需重新编译。

## Lift

| 宏 | 默认 | 含义 |
|---|---|---|
| `P_LIFT_MIN_ANGLE` | 0.0 | 底部限位（电机 deg） |
| `P_LIFT_MAX_ANGLE` | 1250.0 | 顶部限位（电机 deg） |
| `P_LIFT_SPEED` | 30.0 | 恒速（percent） |
| `P_LIFT_TOL` | 2.0 | 自动到位容差（deg） |

## Clamp

| 宏 | 默认 | 含义 |
|---|---|---|
| `P_CLAMP_OPEN_ANGLE` | 40.0 | 张开限位（大角，deg） |
| `P_CLAMP_CLOSE_ANGLE` | -10.0 | 合紧限位（小角，deg） |
| `P_CLAMP_SPEED` | 20.0 | 恒速（percent） |
| `P_CLAMP_TOLERANCE` | 1.0 | 位置锁定死区 / 自动到位容差（deg） |
| `P_CLAMP_PID_KP/KD/KI` | 0.9 / 0.05 / 0.0 | 位置锁定 PID 增益 |

## Odometry

| 宏 | 默认 | 含义 |
|---|---|---|
| `P_ODOM_COEFFICIENT` | (3600.0/3544.0) | 惯性陀螺刻度校正系数 |
| `P_ODOM_WHEEL_DIAMETER_CM` | 8.25 | 驱动轮直径（cm） |
| `P_ODOM_WHEEL_RATIO` | 1.0 | 电机到轮的外部传动比 |

> `cm_per_deg = π·轮径 / (360·比)`。

## XDrive

| 宏 | 默认 | 含义 |
|---|---|---|
| `P_XDRIVE_MAX_OUTPUT` | 0.5 | 全局输出上限（0..1，乘在归一化之后） |
| `P_XDRIVE_MIN_OUTPUT` | 1.0 | idle-brake 进入阈值（percent） |
| `P_XDRIVE_HYSTERESIS` | 1.0 | idle-brake 退出滞回（percent） |

## Auto（底盘自动位移/旋转）

| 宏 | 默认 | 含义 |
|---|---|---|
| `P_AUTO_SPEED` | 60.0 | 默认速度上限（percent） |
| `P_AUTO_POS_TOL` | 2.0 | 位移到位容差（cm） |
| `P_AUTO_ANGLE_TOL` | 2.0 | 旋转到位容差（deg） |
| `P_AUTO_TIMEOUT_MS` | 5000 | 超时（ms） |
| `P_AUTO_SETTLE_MS` | 100 | 容差内需稳定时长（ms） |
| `P_AUTO_MOVE_PID_KP/KD/KI` | 2.0 / 0.1 / 0.0 | 纵向/横向 PID |
| `P_AUTO_ROT_PID_KP/KD/KI` | 1.5 / 0.2 / 0.0 | 朝向 PID |

## Auto 机构（Clamp/Lift 自动动作）

| 宏 | 默认 | 含义 |
|---|---|---|
| `P_LIFT_TOL` | 2.0 | 升降到位容差（deg） |
| `P_MECH_TIMEOUT_MS` | 4000 | 机构自动超时（ms） |
| `P_MECH_SETTLE_MS` | 100 | 机构到稳定时长（ms） |

## 其他常量

- `TICK_MS`（在 `include/vex.h`，默认 10）：Clamp 位置锁定的 tick 补偿使用。
- `driver-control` 内部常量：`kDeadZone`（摇杆死区）、`kScreenRefreshMs`（屏幕刷新周期）。
- `XDrive`：`kBrakeVelEps`（判定“刹停”的速度阈值）；`Odometry`：`kGravity`、`kAccelLpf`、ZUPT 阈值、标定采样数等。

## 调参提示

- 位移/机构自动：先满足“能到位、不过冲、不抖动”，再求快。增大 `kP` 提速但易过冲；`kD` 增阻尼（其有效量级受 ms 计时影响）。
- 停止精度：`P_AUTO_POS_TOL` / `P_AUTO_ANGLE_TOL` / `P_CLAMP_TOLERANCE` / `P_LIFT_TOL` 越大越稳但越不精确。
- 底盘速度：`P_XDRIVE_MAX_OUTPUT` 控制整体上限。
- `speed` 参数与 `P_*_SPEED` 宏控制各自动流程的速度上限。
