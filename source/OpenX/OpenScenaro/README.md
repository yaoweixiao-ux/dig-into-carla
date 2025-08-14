## ScenarioRunner for CARLA

该工程包含交通场景定义，可以通过PythonAPI接口定义。

当前支持：OpenSCENARIO标准和OpenSCENARIO 2.0标准

这是单独的模块：

```shell
https://github.com/carla-simulator/scenario_runner.git
```

CARLA 0.9.14对应到Version 0.9.13的版本代码；

[Version 0.9.13](https://github.com/carla-simulator/scenario_runner/releases/tag/v0.9.13) and the 0.9.13 Branch: Compatible with [CARLA 0.9.13](https://github.com/carla-simulator/carla/releases/tag/0.9.13) and [CARLA 0.9.14](https://github.com/carla-simulator/carla/releases/tag/0.9.14)

[文档](https://scenario-runner.readthedocs.io/en/latest/)

运行带xosc的场景描述文件:

```shell
python scenario_runner.py --openscenario <path/to/xosc-file>
```

Carla本体不实现xosc的解析与执行，这部分的功能是在Client端实现的。

解析文件入口：`scenario_runner.py`：运行单个或多个场景的核心入口与循环。

#### 对于OpenSCENARIO 1.x的解析路径在：

`srunner/tools/openscenario_parser.py`中实现把 `.xosc` 的实体、触发器、动作等解析成 CARLA 可执行对象/行为。

`srunner/scenarioconfigs/openscenario_configuration.py`：从 OpenSCENARIO 配置构建场景所需的配置（地图、参与者、参数等）。

#### 对于OpenSCENARIO 2.0的解析路径在：

通过 `--openscenario2` 入口，ScenarioRunner 增加了 OSC2 支持；文档与说明在其 ReadTheDocs 与仓库 README 中。

OpenDRIVE（道路网）**在 CARLA 核心 C++ 侧实现（`LibCarla/source/carla/opendrive/*`）**


scenario_runner.py时序图

```shell
main()
 └─ parse_args()
 └─ ScenarioRunner(args)
     ├─ new carla.Client(host,port) + set_timeout
     ├─ get_carla_version() ≥ 0.9.14 ?
     ├─ (optional) import agent
     └─ manager = ScenarioManager(...)
 run()
 └─ choose mode:
    [scenario]     _run_scenarios()
    [openscenario] _run_openscenario()
    [osc2]         _run_osc2()
    [route]        _run_route()
         └─ _load_and_run_scenario(config)
            ├─ _load_and_wait_for_world(config.town, config.ego)
            │   ├─ (reloadWorld? load_world)
            │   ├─ world = get_world()
            │   ├─ sync? set synchronous_mode + fixed_delta
            │   ├─ CarlaDataProvider.set_client/world
            │   └─ world.tick() / wait_for_tick()
            ├─ _prepare_ego_vehicles(config.ego)
            ├─ TrafficManager 配置
            ├─ 构造具体 Scenario 实例
            ├─ manager.load_scenario()
            ├─ manager.run_scenario()
            ├─ _analyze_scenario()
            ├─ (record?) start/stop_recorder + _record_criteria()
            └─ _cleanup()

```
