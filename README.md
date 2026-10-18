# VEX-MEKFC5
<div>
<img src="https://img.shields.io/badge/build-preview-brightgreen?style=for-the-badge"/>&emsp;
<img src="https://img.shields.io/badge/match-closed-blue?style=for-the-badge"/>
</div>
$_{By\space癫疯召唤师(MEKFC5)，感谢DeepSeek老师和GPT老师的支持}$

&emsp;&emsp;适用于 2026 SJTU VEX 校内赛的竞赛机器人程序（C++）。机器人采用 **45° 斜置全向底盘（X-drive）**，搭载一个牵引式**升降机**一个**夹爪**，配有惯性传感器与里程计，支持手动驾驶（第一人称/第三人称）与自动位移/旋转、机构自动动作。

## 硬件与端口

| 设备 | 对象名 | 端口 | 齿轮箱 | 反转 |
|---|---|---|---|---|
| 驱动轮 L1 | `Motor_Drive_L1` | PORT2 | 18:1 | 否 |
| 驱动轮 L2 | `Motor_Drive_L2` | PORT3 | 18:1 | 否 |
| 驱动轮 R1 | `Motor_Drive_R1` | PORT4 | 18:1 | 是 |
| 驱动轮 R2 | `Motor_Drive_R2` | PORT5 | 18:1 | 是 |
| 惯性传感器 | `Inertial` | PORT20 | — | — |
| 升降电机 1 | `Motor_Lift_1` | PORT9 | 18:1 | 否 |
| 升降电机 2 | `Motor_Lift_2` | PORT8 | 18:1 | 是 |
| 夹爪电机 | `Motor_Clamp` | PORT10 | 18:1 | 否 |

- 驱动轮直径 8.25 cm，电机:轮传动比 1.0（见 `P_ODOM_WHEEL_DIAMETER_CM` / `P_ODOM_WHEEL_RATIO`）。
- 升降两电机同步，`Motor_Lift` 为二者的 `motor_group`。

## 构建与烧录

- 使用 VEXcode VS Code 扩展（工程名 `MEKFC5`），或 `make`（`vex/mkenv.mk`、`vex/mkrules.mk`）。
- `makefile` 自动收集 `src/*.cpp` 与 `src/*/*.cpp`、头文件 `include/**`。
- 产物：`build/MEKFC5.bin`（烧录到 V5 Brain）。

## 目录结构

```
MEKFC5/
├─ include/            头文件（含 HiDebug/、HiIO/ 子目录）
├─ src/                源文件（含 HiDebug/、HiIO/ 子目录）
├─ docs/               项目技术文档 + VEX API 参考
├─ vex/                构建脚本（mkenv.mk / mkrules.mk）
└─ makefile
```

## 文档索引

- `docs/architecture.md` — 架构、分层、线程、约定
- `docs/build-and-conventions.md` — 构建与编码约定
- `docs/xdrive.md` — 全向底盘与自动位移/旋转
- `docs/odometry.md` — 航向、轮式里程计、IMU 航位推算
- `docs/lift.md` — 升降机
- `docs/clamp.md` — 夹爪（含位置锁定）
- `docs/pid.md` — PID 控制器
- `docs/driver-control.md` — 手动控制、驾驶模式、轴锁定、遥测屏幕
- `docs/autonomous.md` — 自动阶段接口
- `docs/logging-io.md` — HiDebug 日志与 HiIO 屏幕输出
- `docs/config-tuning.md` — 集中调参宏（`config.h`）
- `docs/VexAPI-*.md` — VEX 官方 API 参考（外部资料，版权所有VEX官方，详见 [https://api.vex.com/v5/home/](https://api.vex.com/v5/home/)）

## 关键约定

- 坐标系：以**开机时刻的车体位置和车头朝向**为基准（以开机位置，正前为 ${x+}$ ，正右为 ${y+}$ ），单位 **cm**；朝向`heading` 正方向为小车俯视角顺时针。
- 全部可调参数集中在 `include/config.h`（`P_*` 宏），由 `vex.h` 引入，全局可见。~~虽然这不是好的做法，每次改动参数都需要全量编译。~~
- 全项目全局对象定义在 `src/instance.cpp`，`extern` 声明在 `include/instance.h`。


## 版本说明
* **最新版`8510865`**: 自动化路线完整，控制机制更完善，人工操作性能良好，但未经过完整的人工代码审查，自动化阶段存在异常推出的风险。
* **稳定版`c35df65`**: 经过人工代码审查、严格的鲁棒性测试和赛场实战，可稳定使用，人工操作性能良好，但自动化路线较为简单，并未实现全部赛场任务。
