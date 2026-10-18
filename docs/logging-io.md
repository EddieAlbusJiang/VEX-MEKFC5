# 日志与屏幕输出（HiDebug / HiIO）

## HiDebug（日志）

文件：`include/HiDebug.h`（汇总）、`include/HiDebug/log.h`、`include/HiDebug/str.h`、`include/HiDebug/conf.h`、`src/HiDebug/log.cpp`。

### 使用

```cpp
#include "HiDebug.h"

Debug::Log("message", Debug::LogLevel::Info);
Debug::Log("[Mech] Lift=" + Debug::doubleToString(Mech_Lift.position(), 1),
           Debug::LogLevel::Warning);
```

### 等级与格式

```cpp
enum class Debug::LogLevel : int { Debug, Info, Warning, Critical, Fatal };
```

- 输出格式：`[<Tag>]<内容>\r\n`，Tag 为 `D/I/W/C/F`。
- 低于 `HD_LOG_MIN_LEVEL` 的日志被丢弃（先过滤再拼串）。

### 输出目标与配置（`conf.h`）

以**宏是否定义**决定启用（在 `include/HiDebug/conf.h` 中统一配置）：

```cpp
#define HD_LOG_ENABLED
#define HD_LOG_OUTPUT_BRAIN       // 输出到 Brain（经 HiIO 滚动缓冲）
#define HD_LOG_OUTPUT_CONSOLE     // printf
// #define HD_LOG_OUTPUT_CONTROLLER  // 可选
#define HD_LOG_MIN_LEVEL LogLevel::Debug
```

- 线程安全：内部 `vex::mutex`。
- Brain 输出委托给 `HiIO::BrainScreen::printLine`（见下）。

### str.h

```cpp
namespace Debug {
inline std::string doubleToString(double value, unsigned int precision = 2);
}
```
- 处理 `NaN`/`±Infinity`；用全局 `snprintf`（VEX 定制库中 `snprintf` 不在 `std`）。

## HiIO（屏幕输出）

文件：`include/HiIO.h`（汇总）、`include/HiIO/brain_screen.h`、`src/HiIO/brain_screen.cpp`。

### 使用

```cpp
#include "HiIO.h"

HiIO::BrainScreen::setBrain(Brain);   // 使用前必须注入（会清屏并重置缓冲）
HiIO::BrainScreen::printLine("Hello"); // 追加一行，超过 12 行时滚动
HiIO::BrainScreen::clear();            // 清屏并清空缓冲
```

### 行为

- 默认 Brain 字体 `mono20` 为 **12 行 × 48 列**；`BrainScreen` 维护最近 12 行的**滚动缓冲**：未满时增量打印，满后丢弃最旧一行并整屏重绘。
- 换行仅由 `Brain.Screen.newLine()` 完成（`print` 不处理换行符）。
- **依赖注入**：`BrainScreen` 不直接依赖 `instance`；未 `setBrain` 时 `printLine/clear` 为 no-op。
- 线程安全：内部 `vex::mutex`。

> 扩展：HiIO 作为统一 I/O 管理模块，后续可在此加入 Controller/Console 等设备。
