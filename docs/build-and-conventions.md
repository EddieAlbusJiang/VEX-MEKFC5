# 构建与编码约定

## 构建

- 工程为 VEXcode V5 工程（`MEKFC5`），可用 VEXcode VS Code 扩展构建/烧录，或命令行 `make`。
- `makefile`：
  - `SRC_C = $(wildcard src/*.cpp) + src/*.c + src/*/*.cpp + src/*/*.c`
  - 头文件目录 `include`（`INC_F`）
  - 产物 `build/MEKFC5.bin` / `.elf`
- 新增源文件放到 `src/`（或 `src/<子目录>/`）即可，无需修改 `makefile`。

## 工具链注意事项

- VEX 的 C++ 标准库是**定制版**：部分 C 标准库函数（如 `snprintf`）位于**全局命名空间**而非 `std`，应直接调用 `snprintf(...)`。
- 使用 `vex::` 命名空间下的 API（电机、惯性、控制器、屏幕等）；涉及 API 用法请优先查阅 `docs/VexAPI-*.md`。

## 命名与风格

- 类：`PascalCase`（`XDrive`、`Odometry`、`Lift`、`Clamp`、`PIDController`）。
- 成员变量：`snake_case_`（尾下划线，如 `motor_`、`speed_`）。
- 常量：`constexpr` 且 `k` 前缀（如 `kDeadZone`、`kBrainsRows` 风格）。
- 调参宏：`config.h` 中 `P_` 前缀 + 模块名（如 `P_LIFT_SPEED`、`P_XDRIVE_MAX_OUTPUT`、`P_AUTO_POS_TOL`）。
- 全局对象：`instance.h` 用 `extern` 声明（`Motor_Drive_L1`、`Chassis`、`Odom` 等），`instance.cpp` 唯一定义。
- 自动/手动接口命名：`Auto<Subsystem><Verb>`（如 `AutoMoveTo`、`AutoClampOpen`、`AutoLiftUp`）。

## 头文件

- 一律 `#pragma once`。
- 避免头文件重量级依赖：仅需类型引用时可**前向声明**（例如 `x-drive.h` 只前向声明 `class Odometry;`）。
- 具体实现包含在 `.cpp` 中。

## 健壮性约定

- 角度/位移接口普遍带**软限位**、**容差**、**超时**与**稳定判定**。
- 对外输入做**钳制**（`setSpeed` 钳制 [0,100]、`clampSym`、屏幕字段钳制）。
- 跨线程共享用 `vex::mutex` 保护；实时标志用 `volatile`。
- 空依赖做判空（`HiIO::BrainScreen` 未注入 Brain 时为 no-op）。
