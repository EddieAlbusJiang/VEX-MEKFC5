# PID 控制器

`include/pid-controller.h` / `src/pid-controller.cpp` 提供一个最小可复用的 PID。

## 数据结构

```cpp
struct PIDParam { double kP; double kD; double kI; };
```

## 接口

```cpp
class PIDController {
  public:
    explicit PIDController(PIDParam params);
    void   setGains(PIDParam params);
    void   setUseSeconds(bool use); // false(默认)=毫秒计时；true=秒计时
    void   reset();            // 清积分/微分/输出并重设时间基准
    void   update(double error);
    double output() const;
};
```

## 行为

- 每个控制周期调用一次 `update(error)`，随后读取 `output()`。
- `dt` 由内部 `vex::timer` 测量，默认**毫秒**；调用 `setUseSeconds(true)` 后 `dt` 以**秒**计（使 `kD` 量级更常规、可用作阻尼）。
  - `D = kD * (e - e_prev) / dt`
  - `I = kI * ∫ e dt`（按 dt 单位累加）
- `update`：`output = P + D + I`。
- `reset()`：清零 `prev_error_`、`error_integral_`、`output_`，并把 `prev_time_` 设为当前时刻（`dt<=0` 时 D 项取 0，避免除零）。

## 使用注意

- **一次性流程（如自动到位）在开始前应调用 `reset()`**，否则会带入上一段的积分/微分状态。
- 增益以实际量为准（例：位置环误差单位为 deg/cm，输出为 percent）。因 `dt` 为毫秒，`kD` 的有效量级偏小，调参时注意单位。
- 机加/底盘各不相同：本项目在 `config.h` 中分别提供 `P_CLAMP_PID_*` 与 `P_AUTO_MOVE_PID_*` / `P_AUTO_ROT_PID_*`。

## 使用位置

- `Clamp`：位置锁定环（`stop()` 内归位，增益 `P_CLAMP_PID_*`；**按秒计时**）。
- `XDrive` 自动流程：位移的纵向/横向环与朝向环（`static` 复用，按秒计时，`P_AUTO_*_PID_*`）。
