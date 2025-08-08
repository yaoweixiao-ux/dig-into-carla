import matplotlib
import pandas as pd
import matplotlib.pyplot as plt

# 设置中文字体（使用系统已有的中文字体，如 SimHei）
matplotlib.rcParams['font.sans-serif'] = ['SimHei']  # 黑体
matplotlib.rcParams['axes.unicode_minus'] = False    # 解决负号显示为方块

# 加载数据
data = pd.read_csv("sim_loop_time.csv", header=None, names=["duration_us"])

# 统计指标
mean_val = data["duration_us"].mean()
std_val = data["duration_us"].std()
max_val = data["duration_us"].max()
min_val = data["duration_us"].min()

print("抖动分析报告：")
print(f"- 平均值 Mean:     {mean_val:.2f} us")
print(f"- 标准差 StdDev:  {std_val:.2f} us")
print(f"- 最大值 Max:      {max_val:.2f} us")
print(f"- 最小值 Min:      {min_val:.2f} us")
print(f"- 最大抖动幅度:    {max_val - mean_val:.2f} us")

# 绘图分析
plt.figure(figsize=(12, 6))
plt.plot(data["duration_us"], label="Loop Duration (us)")
plt.axhline(y=mean_val, color='r', linestyle='--', label=f"Mean = {mean_val:.1f}")
plt.axhline(y=max_val, color='orange', linestyle='--', label=f"Max = {max_val:.1f}")
plt.title("Carla Simulation Loop Duration Over Time")
plt.xlabel("Iteration")
plt.ylabel("Duration (us)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("loop_jitter_analysis.png")
plt.show()

