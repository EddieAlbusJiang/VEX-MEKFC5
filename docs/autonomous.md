# Autonomous（自动阶段）

`include/autonomous.h` / `src/autonomous.cpp`，在 `main()` 注册：`Competition.autonomous(autonomous)`。

## 当前状态

`autonomous()` 目前为**占位**：

```cpp
void autonomous() {
    Auto_Active = true;            // 阶段互斥：让 driverControl 暂停命令硬件
    Chassis.setIdleBrake(false);   // 自动期间关闭逐轮 idle-brake

    // 示例（当前未执行）：
    // Chassis.AutoMoveTo(50.0, 0.0);
    // Chassis.AutoRotateTo(90.0);
    // Chassis.AutoMoveFor(-50.0, 0.0);
    // Chassis.AutoRotateFor(-90.0);

    vex::timer timer;
    printf(" [AUTON] Time: %dms\n", timer.time());

    Chassis.setIdleBrake(true);    // 恢复
    Auto_Active = false;
}
```

- **启动清零位姿**：`autonomous()` 起始处调用 `Odom.resetRotation()/resetWheel()`（IMU 位移已禁用，`resetImu` 已注释），把**朝向与轮式里程计位置**归零，使例程中的**绝对目标**总是相对“本次自动的起点与朝向”（无论从场控、`ButtonB` 还是开机直调触发）。
- **阶段互斥**：`Auto_Active`（`instance.*` 中的 `volatile bool`）在自动阶段置位，`driverControl()` 见其为真时**不再命令底盘/机构**（仅刷新屏幕），避免与 `autonomous()` 互相打断。
- **调试触发**：在手动阶段按 `ButtonB` 可**同步跑一遍** `autonomous()`（运行中再次按被忽略）；起止由 `autonomous()` 首尾的 `Debug::Log("[Auto] start/done")` 提示。
- **idle-brake 在自动阶段首尾统一关闭/恢复**（避免逐轮锁死扭曲 holonomic 运动），不在自动函数内部单独开关。

## 可用接口

**每个 Auto 均有“阻塞 / 非阻塞”两种重载**：阻塞版形如 `AutoXxx(args..., speed = 默认)`；非阻塞版形如 `AutoXxx(args..., void (*onDone)(void), speed = 默认)`（回调可为 `nullptr`）——入队即返回，由各自 worker 线程执行，完成时回调。三类（底盘/升降/夹爪）各有一个 worker，**可并行**。

**底盘（`XDrive`，坐标系=开机系、原点=开机位置、cm）**
```cpp
// 阻塞
Chassis.AutoMoveTo(tx, ty, speed = P_AUTO_SPEED);
Chassis.AutoMoveFor(dx, dy, speed = P_AUTO_SPEED);
Chassis.AutoRotateTo(tdeg, speed = P_AUTO_SPEED);
Chassis.AutoRotateFor(ddeg, speed = P_AUTO_SPEED);
// 特快位移：世界系相对位移、最大速度、无 PID、无减速、不保持朝向（仅阻塞）
Chassis.AutoFastMoveFor(dx, dy);
// 非阻塞
Chassis.AutoMoveTo(tx, ty, onDone /*可为 nullptr*/, speed = P_AUTO_SPEED);
// …其余同理
```

**升降（`Lift`）**
```cpp
Mech_Lift.AutoLiftTo(deg, speed = P_LIFT_SPEED);                 // 阻塞
Mech_Lift.AutoLiftTo(deg, onDone, speed = P_LIFT_SPEED);         // 非阻塞
Mech_Lift.AutoLiftUp(speed = P_LIFT_SPEED);
Mech_Lift.AutoLiftUp(onDone, speed = P_LIFT_SPEED);
// …
```

**夹爪（`Clamp`）**
```cpp
Mech_Clamp.AutoClampOpen(speed = P_CLAMP_SPEED);          // 阻塞
Mech_Clamp.AutoClampOpen(onDone, speed = P_CLAMP_SPEED);  // 非阻塞
// …
```

## 前置条件

- 位移依赖里程计，`initialize()` 已完成：惯性校准、原点复位（`resetWheel`），并启动了 `odometryThread`（IMU 位移已禁用）。
- 升降/夹爪零点由 `initialize()` 的 `resetPosition()` 建立（开机前需手动复位）。

## 注意事项

- **阻塞版**：含超时与到位稳定判定；按顺序调用即可串联多步（经队列由 worker 执行）。
- **非阻塞版**：入队即返回；多子系统可并行；完成后回调通知；队列满（`P_AUTO_QUEUE`）则**丢弃**新请求。
- ⚠️ **禁止在回调中调用阻塞版 Auto**（回调运行于 worker 线程，会与 worker 互等 → 死锁）。本项目无重入场景。
- 位移使用**轮式里程计**位置；打滑、低速受阻可能导致超时。
- 参数集中在 `config.h`（`P_AUTO_*`、`P_LIFT_TOL`、`P_CLAMP_*`、`P_MECH_*`、`P_AUTO_QUEUE`）。
