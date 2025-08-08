#!/bin/bash
NUM=10
BASE_RPC=1249
BASE_STREAM=1250
STEP=3

for i in $(seq 0 $((NUM-1))); do
    name="carla_$i"
    RPC_PORT=$((BASE_RPC + i * STEP))
    STREAM_PORT=$((BASE_STREAM + i * STEP))

    # 如果容器存在，先删除
    if sudo docker ps -a --format '{{.Names}}' | grep -wq "$name"; then
        echo "检测到旧容器 $name，正在删除..."
        sudo docker rm -f "$name"
    fi

    echo "启动 CARLA 实例 $i: RPC=$RPC_PORT, STREAM=$STREAM_PORT"
    sudo docker run -d --privileged --gpus all --net=host \
        -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
        --name "$name" \
        carlasim/carla:0.9.14 \
        /bin/bash ./CarlaUE4.sh -RenderOffScreen \
        -carla-rpc-port=${RPC_PORT} \
        -carla-streaming-port=${STREAM_PORT}
done

