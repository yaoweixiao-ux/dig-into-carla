# Dig-Into-Carla Server

> 高行微言，所以修身。


Server(Unreal):负责世界演进、渲染与各类传感器。

UE插件中的ASensor作为传感器基类：

管理FDataStream、订阅、Tick生命周期(prephysTick/OnFirstClientConnected/...)

自类实现不同传感器(Camera / LIDAR/ Radar / IMU / GNSS / 事件等)
