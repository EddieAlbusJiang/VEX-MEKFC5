# 架构

## 概述

MEKFC5 是一套模块化的 VEX V5 程序。按职责分层，硬件封装与上层策略解耦，便于调参与扩展。

## 分层

| 层 | 模块 | 职责 |
|---|---|---|
| 硬件封装 | `XDrive` | 全向底盘运动学（混合、限幅、idle-brake、自动位移/旋转） |
| | `Odometry` | 航向、轮式里程计、IMU 航位推算、安装标定 |
| | `Lift` / `Clamp` | 升降机 / 夹爪手动与自动动作 |
| | `PIDController` | 通用 PID |
| 服务 | `HiDebug`（`Debug::Log`、`str.h`） | 分级日志、多目标输出 |
| | `HiIO`（`BrainScreen`） | Brain 屏幕 12 行滚动缓冲 |
| 策略 | `driver-control` | 手动驾驶、模式切换、轴锁定、遥测屏幕 |
| | `autonomous` | 自动阶段流程（调用底盘/机构自动接口） |
| 装配 | `instance.*` | 全局硬件对象与单例 |
| 配置 | `config.h` | 集中调参宏 |

## 依赖注入与全局单例

- 硬件类通过**构造参数**接收其依赖（电机引用）；`XDrive` 额外持有 `Odometry&`；`Odometry` 持有 `vex::inertial&` 与四个驱动电机引用。这样便于替换与测试。
- `HiIO::BrainScreen` **不直接依赖** `instance`，通过 `setBrain(vex::brain&)` 注入，且在注入时清屏并重置缓冲。
- 全项目单例定义在 `src/instance.cpp`，`extern` 声明在 `include/instance.h`（`Chassis`、`Odom`、`Mech_Lift`、`Mech_Clamp` 等）。

## 多线程

| 线程 | 启动点 | 作用 |
|---|---|---|
| main | `main()` | 初始化、注册竞赛回调、常驻循环 |
| 里程计线程 | `initialize()` 中 `static vex::thread odometryThread` | 每 ~10ms 更新 IMU 积分与轮式里程计 |
| 单元测试线程 | `HiTest()` | 每 500ms 打印朝向/里程计/IMU/原始加速度 |
| 按钮/传感器回调线程 | `pressed()` 回调 | 切换驾驶模式/轴锁定、请求朝向复位等 |

- 跨线程共享状态用 `vex::mutex` 保护（如 `driver-control` 的 `drive_mode_mutex`）；对实时性要求高的标志使用 `volatile`（如 `XDrive::braking_`）。
- **自动/手动阶段互斥**：`Auto_Active`（`instance.*` 的 `volatile bool`）在 `autonomous()` 期间置位；`driverControl()` 见其为真时不再命令硬件，避免两者同时抢占底盘/机构造成抖顿。
- 涉及传感器写入的按钮回调**只置位请求**，实际调用（如 `resetRotation`）在控制循环中执行。

## 坐标系与单位

- 位移：以**开机时刻车头朝向**为基准系（原点=开机位置），单位 **cm**。
- `heading`：0–360°，**顺时针为正**。
- 底盘输入：`forward`(+前) / `lateral`(+右) / `rotational`(+顺时针)，单位 percent(-100..100)。
- 机构：电机轴角度（deg）。

## 数据流（手动）

```
Controller → driver-control.updateChassis → (NorthUp 旋转变换)? → (轴锁定)? → XDrive.drive → 4 电机
Controller → updateLift / updateClamp → Lift / Clamp 手动接口
Inertial → Odometry.updateImu/updateWheel → Odom（屏幕/自动流程读取）
```

## 数据流（自动）

```
autonomous() → Chassis.AutoMoveTo/AutoRotateTo → runMove/runRotate（PID，读取 Odom）→ XDrive.drive
autonomous() → Mech_Lift/Mech_Clamp.Auto* → runToAngle（恒速 + 容差）→ 机构电机
```

## 编译期配置

所有可调参数集中在 `include/config.h`（`P_*` 宏），经 `include/vex.h` 引入，全局可见；修改后需重新编译。详见 `config-tuning.md`。

## 目录约定

- 头文件 `include/**`，源文件 `src/**`；新模块在 `makefile` 的 `wildcard` 规则下自动纳入。
- 子模块（`HiDebug`、`HiIO`）采用 `include/<Mod>/` 与 `src/<Mod>/` 结构，并提供汇总头 `<Mod>.h`。
