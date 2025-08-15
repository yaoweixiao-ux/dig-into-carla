## 汽车动力学

CARLA 0.9.14版本 采用UE 的 Chaos 引擎，由引擎求解轮胎力、悬架、动力学。

CARSIM 接入只是将Chaos解析切换为Carsim求解轮胎力、悬架、动力学。

代码位置：**`Plugins/Carla/Source/Carla/Vehicle/`**

### 动力学相关：

* WheeledVehicleMovementComponentNW.h/.cpp
  车辆运动组件，**把各物理参数下发给 UE（Chaos）**，由引擎求解轮胎力/悬架/动力学。
* VehiclePhysicsControl.h/.cpp WheelPhysicsControl.h
  **整车/单轮物理参数**（质量、阻力、扭矩曲线、悬架/轮胎等）的配置载体。
* CustomTerrainPhysicsComponent.h/.cpp
  按**地面材质**动态调整轮胎摩擦等，影响抓地/打滑。
* CarlaWheeledVehicle.\ CarlaWheeledVehicleNW.h/.cpp
  车体 Actor，**接收参数与控制并应用到运动组件**；读回状态（速度、轮速…）。

### 控制与接口

* `VehicleControl.h`
  经典控制输入（油门/刹车/转向/挡位）。
* `VehicleAckermannControl.h`、`AckermannController.*`、`AckermannControllerSettings.h`
  **Ackermann 控制器**，把“目标速度/转角(曲率)”转换成油门/刹车/方向输入，再交给运动组件求解。
* VehicleVelocityControl.*
  定速控制

CARLA负责将车辆的物理参数以及控制指令给到UE的chaos引擎，在引擎里进行动力学求解，最终作用到Actor的位置和旋转属性。
