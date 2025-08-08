# Dig-Into-Carla Config

> Better late than never. 

当前目录下存放着各种ini文件，如果是源码安装方式的话，make launch将 UE4 启动起来，可以看到默认地图Town10HD_Opt。

更改默认地图的话需要修改 DefaultEngine.ini：

```python
[/Script/EngineSettings.GameMapsSettings]
EditorStartupMap=/Game/Carla/Maps/Town10HD_Opt.Town10HD_Opt
GameDefaultMap=/Game/Carla/Maps/Town10HD_Opt.Town10HD_Opt
ServerDefaultMap=/Game/Carla/Maps/Town10HD_Opt.Town10HD_Opt
GlobalDefaultGameMode=/Game/Carla/Blueprints/Game/CarlaGameMode.CarlaGameMode_C
GameInstanceClass=/Script/Carla.CarlaGameInstance
TransitionMap=/Game/Carla/Maps/Town10HD_Opt.Town10HD_Opt
GlobalDefaultServerGameMode=/Game/Carla/Blueprints/Game/CarlaGameMode.CarlaGameMode_C
```