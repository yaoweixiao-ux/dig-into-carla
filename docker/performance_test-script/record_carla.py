import carla
import time
import csv

# 配置连接
client = carla.Client('localhost', 2000)
client.set_timeout(10.0)
world = client.get_world()

# 开启同步模式
settings = world.get_settings()
settings.synchronous_mode = True
settings.fixed_delta_seconds = 0.01  # 10ms 固定步长
world.apply_settings(settings)

# 设置输出
duration_list = []
output_file = "sim_loop_time.csv"

# 仿真主循环
try:
    for i in range(60000):  # 10分钟，60000次迭代
        start = time.perf_counter()

        world.tick()  # 驱动仿真前进一步

        end = time.perf_counter()
        duration_us = (end - start) * 1e6  # 转为微秒
        duration_list.append(duration_us)

        if i % 1000 == 0:
            print(f"Step {i}: duration {duration_us:.2f} us")

finally:
    # 还原设置
    settings.synchronous_mode = False
    world.apply_settings(settings)

    # 保存耗时数据
    with open(output_file, 'w') as f:
        writer = csv.writer(f)
        for d in duration_list:
            writer.writerow([d])

    print(f"Saved loop durations to {output_file}")

