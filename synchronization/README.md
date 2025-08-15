### 同步模式

CARLA默认是异步状态；

eg: 设置一个车辆，设置自动驾驶模式，安置摄像头和激光雷达，落盘数据，摄像头有掉帧现象。

因为CARLA的**仿真server默认为异步模式，它会尽可能快地进行仿真，根本不管客户是否跟上了它的步伐**。

## 仿真世界里的时间步长

在simulation里的时间与真实世界是不同的，simulation里没有“一秒”的概念，只有“一个time-step"的概念。这个time-step相当于仿真世界进行了一次更新，它在真实世界里的时间可能只有几毫秒。

仿真世界里的这个time-step其实有两种，一种是[Variable time-step](https://zhida.zhihu.com/search?content_id=164023219&content_type=Article&match_order=1&q=Variable+time-step&zhida_source=entity), 另一种是[Fixed time-step](https://zhida.zhihu.com/search?content_id=164023219&content_type=Article&match_order=1&q=Fixed+time-step&zhida_source=entity).

Variable time-step. 顾名思义，仿真每次步长所需要的真实时间是不一定的，可能这一步用了3ms, 下一步用了5ms, 但是它会竭尽所能地快速运行。这是仿真默认的模式：

```shell
settings = world.get_settings()
settings.fixed_delta_seconds = None # Set a variable time-step
world.apply_settings(settings)
```

Fixed time-step. 在这种时间步长设置下，每次time-step所消耗的时间是固定的，比如永远是5ms. 设置代码如下：

```shell
settings = world.get_settings()
settings.fixed_delta_seconds = 0.05 #20 fps, 5ms
world.apply_settings(settings)
```

## 同步模式

carla simulation**默认模式为异步模式**+**variable time-step**, 而**同步模式则对应fixed time-step**.
在异步模式下, server会自个跑自个的，client需要跟随它的脚步，如果client过慢，可能导致server跑了三次，client才跑完一次, 这就是为什么摄像头掉帧的原因。

而在同步模式下，simulation会等待客户完成手头的工作后，再进行下一次更新。假设simulation每次更新只需要固定的5ms,但我们客户端储存照片需要10ms, 那么simulation就会等照片储存完才进行下一次更新，也就是说，一个真正cycle耗时10ms(simulation更新与照片储存是同时开始进行的）。设置代码**关键部分**如下：

```shell
def sensor_callback(sensor_data, sensor_queue, sensor_name):
    if 'lidar' in sensor_name:
        sensor_data.save_to_disk(os.path.join('../outputs/output_synchronized', '%06d.ply' % sensor_data.frame))
    if 'camera' in sensor_name:
        sensor_data.save_to_disk(os.path.join('../outputs/output_synchronized', '%06d.png' % sensor_data.frame))
    sensor_queue.put((sensor_data.frame, sensor_name))

settings = world.get_settings()
settings.synchronous_mode = True
world.apply_settings(settings)

camera = world.spawn_actor(blueprint, transform)
sensor_queue = queue.Queue()
camera.listen(lambda image: sensor_callback(image, sensor_queue, "camera"))

while True:
    world.tick()
    data = sensor_queue.get(block=True)
```
