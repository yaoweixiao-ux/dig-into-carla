# Dig-Into-Carla Source

> Time stays long enough for anyone who will use it.

Carla的源码是按照Unreal Engine插件的形式来实现的.

UE提供了标准插件的版本，支持将工程以插件的形式放到Unreal Engine里面。

所以Carla编译版本的源码存放在Unreal->CarlaUE4->Plugins->Carla->Source

![1755166752192](images/README/1755166752192.png)

Carla自动驾驶模拟器是Client-Server架构。

Server端一部分是UnrealEngnie，一部分是Carla ,使用C++实现。[Server端](https://zhida.zhihu.com/search?content_id=194107983&content_type=Article&match_order=2&q=Server%E7%AB%AF&zhida_source=entity)负责和仿真相关的功能：sensor 渲染，物理计算，更新world state和actor等。

Client端是Carla，由多个Client组成，支持多个Client同时运行 ,一部分是C++实现的，一部分是python实现的。Client端控制actor的逻辑，设置world条件。

![1755225368010](images/README/1755225368010.png)

Client-Server之间的通信方式采用的是RPC框架，TCP协议。

LibCarla源码在.../LibCarla/source/carla/rpc。

![1755225654172](images/README/1755225654172.png)

RPC（Remote Procedure Call）—远程过程调用，它是一种通过网络从远程计算机程序上请求服务，而不需要了解底层网络技术的协议。

![1755225841732](images/README/1755225841732.png)


### CARLA代码结构

```cpp
├── Build //编译时产生的文件夹，里面是编译安装的依赖工具
├── Co-Simulation //联合仿真
│      ├──PTV-Vissim //和 PTV-Vissim联合仿真
│      └── Sumo //和 Sumo联合仿真
├── Dist
├── Docs //markdown说明文档
├── Doxygen //如果安装了  Doxygen，使用 Doxygen生成的文档在这个文件夹
├── Examples //使用CARLA's C++ API的例子
├── Import //外部地图导入
├── IlibCarla //carla和ue4交互
├── PythonAPI //使用python实现的一些仿真例子
├── Unreal //Carla Plugin
└── Util //开发过程中使用的工具
```
