# Dig-Into-Carla Sensor

> 高行微言，所以修身。

传感器也是一种Actor，附着在车辆上，按照Tick周期产生数据。每个Tick调用回调函数产生点云或者图像数据。

传感器可以设置属性：Tick、FOV、分辨率、安装位置等。

```

blueprint = world.get_blueprint_library().find('sensor.camera.rgb')  #获取传感器
blueprint.set_attribute('image_size_x', '1920')                      #设置分辨率X
blueprint.set_attribute('image_size_y', '1080')                      #设置分辨率Y
blueprint.set_attribute('fov', '110')                                #设置FOV 110
blueprint.set_attribute('sensor_tick', '1.0')                        #设置sensor_tick
```

传感器继承自基类ASensor，使用FAsynDataStream管理数据流,订阅关系，Tick等。

unreal/CarlaUE4/Plugins/Carla/Source/Carla/Sensor/Sensor.h

```
/// Base class for sensors.
UCLASS(Abstract, hidecategories = (Collision, Attachment, Actor))
class CARLA_API ASensor : public AActor
```

可以通过在UE 中创建C++类的方式生成sensor.h和sensor.cpp，点击创建类即可。


### 传感器数据流

SceneCaptureCamera{.cpp .h}

`Sensor`（基类）在 `Tick/PrePhysTick` 里产出一帧---> 写入 `FDataStream`--->`FAsyncDataStream`异步推送给客户端（LibCarla/Python 的 `sensor.listen()`）

相机类基于 `SceneCaptureSensor`/`SceneCaptureCamera` 渲染到 `UTextureRenderTarget2D`，再经 `PixelReader/ImageUtil` 拿到 CPU 内存并序列化。

#### Common配置：

* DataStream.h
  定义**线程安全**的数据流抽象 `FDataStream`（每个传感器各一条），对外暴露 `MakeAsyncDataStream()` 方法，将传感器数据写到对应的端口。
* AsyncDataStream.cpp/.h
  把传感器线程/渲染线程里的产出**排队**到发送线程，避免阻塞游戏/渲染 Tick。
* Sensor.cpp/.h
  所有传感器的**基类**`ASensor`：
  生命周期函数：`PostActorCreated()`, `BeginPlay()`, `EndPlay()`, `Tick`, `PostPhysTick`。

  持有 `FDataStream`；管理订阅（有无客户端）与 `sensor_tick` 节流。
  公共属性（Transform、附着体、噪声开关等）注册到蓝图库 or Actor。
* SensorFactory.cpp/.h
  传感器**工厂/注册表**：把蓝图中的 `id`（如 `sensor.camera.rgb`、`sensor.lidar.ray_cast`）映射到对应 C++ Actor 类，并设置属性/默认值。服务器收到“spawn sensor”的 RPC 时走这里实例化。
* SensorManager.cpp/.h
  统一登记/管理已生成的 `ASensor`，提供按 id 查询、广播设置等。

#### 场景捕获相关：

* SceneCaptureSensor.h / .cpp
  相机类的**通用基类**：内部持有 `USceneCaptureComponent2D` 或等价组件，负责：
  创建/配置 `UTextureRenderTarget2D`（分辨率、FOV、后处理)
  与渲染线程同步（`CaptureScene` / `UpdateContent`）
  帧出栈后通过 `PixelReader` 读回，并借 `ImageUtil` 进行打包。
* SceneCaptureCamera.h / .cpp
  具体的**RGB 相机**实现：设置 `PostProcessSettings` 生成可见光图像。输出 → `Image`（8-bit 颜色）。
* DepthCamera.h / .cpp
  深度相机：使用深度材质/渲染管线输出**线性深度**或**视差**。
* SemanticSegmentationCamera.h / .cpp
  语义分割：每像素输出**类别** id
* InstanceSegmentationCamera.h / .cpp
  实例分割：每像素输出**实例 ID**（区分同类不同实体）
* NormalsCamera.h / .cpp
  法线相机：输出世界/视图空间法线（基于 GBuffer 或材质 pass）。
* OpticalFlowCamera.h / .cpp
  光流相机：两帧之间的像素位移向量，通常依赖引擎光流 pass/GPU 计算。
* DVSCamera.h / .cpp
  动态视觉传感器（事件相机）：对比前后帧亮度变化，生成**事件流**（极性、时间戳、坐标）。输出类型与传统帧不同。
* PixelReader.h / .cpp
  工具：将 `UTextureRenderTarget2D` 的 GPU 纹理**异步读回** CPU；
* ImageUtil.h / .cpp
  图像工具：像素格式转换、Gamma/线性空间变换、打包到目标 buffer（例如深度到 32F、RGB 到 8U、法线编码等）。
* ShaderBasedSensor.h / .cpp
  以**材质/着色器**为核心的可扩展传感器基类（自定义渲染）

#### 射线/物理传感器相关：

* RayCastLidar.h / .cpp
  标准 LiDAR：按 `LidarDescription` 的通道/频率/分辨率发射**物理射线**，采样命中点、距离、强度，并（按需）加入噪声/回波模型。输出点云（XYZ，相位/强度可选）。
* RayCastSemanticLidar.h / .cpp
  语义 LiDAR：在 RayCast 基础上，命中时附带**语义标签/实例 id**（来自命中体的标签组件），输出语义点云。
* LidarDescription.h
  LiDAR 的**参数结构体**：`channels`、`points_per_second`、`rotation_frequency`、`upper_fov/lower_fov`、`range`、`noise_stddev` 等；
* Radar.h / .cpp
  雷达：在一定扇区/俯仰内投射波束，依据**相对速度**估计**多普勒**，输出目标点（距离、方位、俯仰、径向速度）

#### 状态/事件类传感器相关：

* InertialMeasurementUnit.h / .cpp
  IMU：根据 Actor 运动计算**线加速度**、**角速度**，并添加重力项到合适坐标系（本体/世界）；可加噪声/偏置模型。
* GnssSensor.h / .cpp
  GNSS：把 UE 世界坐标经地图 `GeoReference` 转成**经纬高**（lat/lon/alt），周期发布。
* LaneInvasionSensor.h / .cpp
  车道入侵：检测轮迹/包围盒与**车道线**（实/虚线类型）交叉，发布入侵事件。
* ObstacleDetectionSensor.h / .cpp
  障碍检测（“超声/碰撞预警”近似）：前向投射/包围盒检测，输出最近障碍距离/速度等（常配合AEB）。
* RssSensor.h / .cpp
  RSS 传感器（Intel RSS）：封装与 RSS 库的交互，计算**安全距离/响应**，发布 RSS 状态（是否需减速、最小/最大可行加速度等）。
* WorldObserver.h / .cpp
  世界观察者：周期性扫描世界快照（时间戳、所有 Actor 变换/速度/父子关系等），供客户端做全局感知/重放。
