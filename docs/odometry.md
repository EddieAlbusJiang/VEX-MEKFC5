# Odometry（航向与位移估计）

`include/odometry.h` / `src/odometry.cpp`

提供**航向**与**轮式里程计位移**（坐标系=开机系，原点=开机位置，单位 cm）。**IMU 航位推算已禁用**（相关代码以 `#if 0` 保留，便于恢复）。

## 构造

```cpp
Odometry(vex::inertial& inertial, double coefficient,
         vex::motor& l1, vex::motor& l2, vex::motor& r1, vex::motor& r2,
         double wheel_diameter_cm, double motor_to_wheel_ratio = 1.0);
```

- `coefficient`：惯性陀螺比例校正（约 `3600/3544`，修正其约 1.6% 欠读），见 `P_ODOM_COEFFICIENT`。
- 轮参数见 `P_ODOM_WHEEL_DIAMETER_CM` / `P_ODOM_WHEEL_RATIO`；换算 `cm_per_deg = π·轮径 / (360·比)`。

## 航向

```cpp
double rotation();        // 连续角度 = inertial.rotation() * coefficient
double heading();         // 归一到 [0,360)
void   resetRotation();   // 当前朝向置 0
bool   calibrate();       // 陀螺校准 + 静止稳定性检查
```

`calibrate()` 触发惯性校准，等待完成后采样两次 rotation，若 800ms 内漂移 < 0.05° 视为稳定。

## 轮式里程计

```cpp
void   updateWheel();
double wheelX() const;  // cm（开机系）
double wheelY() const;
void   resetWheel();
```

- 每周期读四轮 `position(deg)` 增量 `d1..d4`。
- X-drive 反解车体前进/横移：`f=(d1+d2+d3+d4)/4·cm_per_deg`、`l=(d1-d2-d3+d4)/4·cm_per_deg`。
- 按 `heading` 旋到开机系累加：`x += f·c − l·s`、`y += f·s + l·c`。
- 首帧只做基准（`wheel_prev_`），不产生位移。
- 实测需确认符号：前进→`wheelX` 增、右移→`wheelY` 增；若反，调整反解符号。

## IMU 航位推算（已禁用）

> 该功能已通过 `#if 0` 禁用（`updateImu/imuX/imuY/resetImu` 及加速度积分、ZUPT、刹车暂停等）。以下为原设计说明，供恢复参考。

```cpp
void   updateImu(bool drive_braked);
double imuX() const;  // cm
double imuY() const;
void   resetImu();
```

每周期：

1. 读三轴加速度（g）；用安装旋转矩阵 `imu_R_` 把 IMU 系加速度旋到车体系（取 x/y）。
2. 按 `heading` 旋到开机系，乘 `kGravity=980.665` 得 cm/s²。
3. 一阶低通（`kAccelLpf`）。
4. **刹车暂停**：若 `drive_braked`（驱动轮刹停），速度清零、位置冻结、刷新基准后返回。
5. 否则梯形积分：速度→位置。
6. **ZUPT**：`|a|≈1g` 且 `|gyroRate(z)|` 很小持续若干帧 → 速度清零。

> 限制：纯 IMU 二次积分漂移大，仅供短时/对比参考；实际位移以轮式里程计为主。

## 安装姿态标定（已禁用）

> `calibrateImuBias()` 已随 IMU 位移一并禁用（属 IMU 位移路径）。以下为原设计说明。

```cpp
void calibrateImuBias();
```

- 开机静止时采样重力方向 → `z_body`；构造正交基得到 `imu_R_`（默认单位阵）。
- **yaw 主动标定当前禁用**：原“开机自动前进一小段以估计车头方向”的代码已注释，现仅做**倾斜调平**（车体 x 取 IMU x 的水平投影）。`updateImu` 使用 `imu_R_` 做倾斜校正。
- 恢复 yaw 的方法：取消 `src/odometry.cpp` 中该段注释并启用相关宏。

## 线程与调用

- `odometryThread()`（在 `initialize()` 启动）每 ~10ms 调用 `Odom.updateWheel()`（`updateImu` 已注释禁用）。
- `initialize()` 中 `Odom.calibrate()` → `Odom.resetWheel()` 建立零点（`calibrateImuBias/resetImu` 已禁用）。

## 相关宏

`P_ODOM_COEFFICIENT`、`P_ODOM_WHEEL_DIAMETER_CM`、`P_ODOM_WHEEL_RATIO`。
