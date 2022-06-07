#!/bin/bash
if [ $# -ne 2 ]; then
    echo "指定された引数は$#個です。" 1>&2
    echo "実行するには2個の引数が必要です。" 1>&2
    exit 1
fi

./waf --run=ndn-kademlia >$2.log

if [ $? -ne 0 ]; then
    echo "シミュレーションに失敗"
    rm *.tsv *.log
    exit -1
fi

mkdir -p result/$1/$2
rm -rf result/$1/$2/*

mv app-delays-trace.tsv result/$1/$2/app-delays-trace-$2.tsv
mv drop-trace.tsv result/$1/$2/drop-trace-$2.tsv
mv rate-trace.tsv result/$1/$2/rate-trace-$2.tsv
mv $2.log result/$1/$2/$2.log

cd result/$1/$2
/home/strea/t2c.sh app-delays-trace-$2.tsv
/home/strea/t2c.sh drop-trace-$2.tsv
/home/strea/t2c.sh rate-trace-$2.tsv

exit 0
