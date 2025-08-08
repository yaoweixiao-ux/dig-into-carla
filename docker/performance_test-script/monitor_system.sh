#!/bin/bash
# monitor_system.sh
# 持续监控整个系统 CPU、内存、磁盘、网络、GPU 使用情况，并保存到日志（5 分钟）

INTERVAL=1                  # 采样间隔秒
DURATION=300                # 总监控时长（秒），5 分钟 = 300 秒
LOG_DIR="./logs"
mkdir -p "$LOG_DIR"

SYS_LOG="$LOG_DIR/system_stats.log"
GPU_LOG="$LOG_DIR/gpu_stats.log"

echo "开始监控整个系统，采样间隔: ${INTERVAL}s，总时长: ${DURATION}s"
echo "日志保存到: $LOG_DIR"

# 写表头
echo "timestamp,cpu_used%,mem_used%,mem_total,disk_used%,disk_used_GB,disk_total_GB,net_rx_MBps,net_tx_MBps" > "$SYS_LOG"
echo "timestamp,gpu_id,util%,mem_used_MB,mem_total_MB,mem_util%" > "$GPU_LOG"

# 获取初始网络统计
prev_rx=$(cat /sys/class/net/*/statistics/rx_bytes | awk '{sum+=$1} END {print sum}')
prev_tx=$(cat /sys/class/net/*/statistics/tx_bytes | awk '{sum+=$1} END {print sum}')

SECONDS=0
while [ $SECONDS -lt $DURATION ]; do
    TIMESTAMP=$(date "+%Y-%m-%d %H:%M:%S")

    # CPU 使用率（1 秒平均）
    cpu_used=$(top -bn1 | grep "Cpu(s)" | awk '{print 100 - $8}') 

    # 内存使用
    mem_total=$(free -m | awk '/内存：/ {print $3}')
    mem_used=$(free -m | awk '/内存：/ {print $2}')

    # 磁盘使用（根目录）
    read disk_used disk_total disk_percent <<< $(df -BG / | awk 'NR==2 {gsub("G","",$3); gsub("G","",$2); gsub("%","",$5); print $3, $2, $5}')

    # 网络速率
    curr_rx=$(cat /sys/class/net/*/statistics/rx_bytes | awk '{sum+=$1} END {print sum}')
    curr_tx=$(cat /sys/class/net/*/statistics/tx_bytes | awk '{sum+=$1} END {print sum}')
    rx_rate=$(awk -v c=$curr_rx -v p=$prev_rx -v i=$INTERVAL 'BEGIN {printf "%.2f", (c-p)/1024/1024/i}')
    tx_rate=$(awk -v c=$curr_tx -v p=$prev_tx -v i=$INTERVAL 'BEGIN {printf "%.2f", (c-p)/1024/1024/i}')
    prev_rx=$curr_rx
    prev_tx=$curr_tx

    # 写系统日志
    echo "$TIMESTAMP,$cpu_used,$mem_used,$mem_total,$disk_percent,$disk_used,$disk_total,$rx_rate,$tx_rate" >> "$SYS_LOG"

    # GPU 信息
    nvidia-smi --query-gpu=index,utilization.gpu,memory.used,memory.total,utilization.memory \
        --format=csv,noheader,nounits | \
    while read gpu_line; do
        echo "$TIMESTAMP,$gpu_line" >> "$GPU_LOG"
    done

    sleep $INTERVAL
done

echo "监控完成，已保存到 $LOG_DIR"

