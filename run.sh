#!/bin/bash

if [ $# -ne 1 ]; then
    echo "指定された引数は$#個です。" 1>&2
    echo "実行するには1個の引数が必要です。" 1>&2
    exit 1
fi

./waf --run=ndn-kademlia >$1.log

if [ $? -ne 0 ]; then
    echo "シミュレーションに失敗"
    rm *.tsv *.log
    exit -1
fi

mkdir -p result/$1
rm -rf result/$1/*
mv app-delays-trace.tsv drop-trace.tsv rate-trace.tsv $1.log result/$1/.
cd result/$1
/home/strea/t2c.sh app-delays-trace.tsv
/home/strea/t2c.sh drop-trace.tsv
/home/strea/t2c.sh rate-trace.tsv

exit 0
