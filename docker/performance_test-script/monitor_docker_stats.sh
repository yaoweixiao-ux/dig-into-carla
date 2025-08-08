#!/bin/bash
# monitor_docker_stats.sh
# 监控所有docker容器的资源使用情况，持续5分钟，1秒采样

INTERVAL=1
DURATION=300   # 5分钟
LOG_DIR="./logs"
mkdir -p "$LOG_DIR"

LOG_FILE="$LOG_DIR/docker_stats.log"

echo "开始监控所有docker容器资源，采样间隔：${INTERVAL}s，时长：${DURATION}s"
echo "日志保存到: $LOG_FILE"

# 写表头（docker stats默认输出列）
echo "timestamp,CONTAINER ID,NAME,CPU %,MEM USAGE / LIMIT,MEM %,NET I/O,BLOCK I/O,PIDS" > "$LOG_FILE"

SECONDS=0
while [ $SECONDS -lt $DURATION ]; do
    TIMESTAMP=$(date "+%Y-%m-%d %H:%M:%S")
    # 获取docker stats，非交互模式，1次采样
    docker stats --no-stream --format "{{.Container}},{{.Name}},{{.CPUPerc}},{{.MemUsage}},{{.MemPerc}},{{.NetIO}},{{.BlockIO}},{{.PIDs}}" | \
    while IFS= read -r line; do
        echo "${TIMESTAMP},${line}" >> "$LOG_FILE"
    done
    sleep $INTERVAL
done

echo "监控完成，日志保存在 $LOG_FILE"

