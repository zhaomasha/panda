#!/bin/bash
export panda_home='/home/bsp/programer/panda' 
export INITSZ='1'
export BLOCKSZ='4096'
export INDEX_BLOCKSZ='4096'
export INCREASZ='1'
export CACHESZ='1000'
export MASTER_IP="192.168.11.51"
export MASTER_PORT="4444"
export SLAVE_IP="192.168.11.51:192.168.11.52:192.168.11.54"
export SLAVE_PORT="4445"
export DIR_NAME="/home/zhaomasha/graphdata"
export BAL_DIR_NAME=$DIR_NAME"/panda_bal"
export SERVER_DIR_NAME=$DIR_NAME"/panda_server"

export HASH_NUM="1000"  
export VERTEX_INDEX_FILENAME="vertex.index"

