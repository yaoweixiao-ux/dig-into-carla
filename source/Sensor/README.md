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
