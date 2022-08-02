#!/bin/bash
if [ $# -ne 3 ]; then
    echo "指定された引数は$#個です。" 1>&2
    echo "実行するには3個の引数が必要です。" 1>&2
    exit 1
fi

export ID_PRD=$1
export ID_CON_MJ=$2
export ID_CON_MN=$3

./waf --run=ndn-kademlia >$1-$2-$3.log

if [ $? -ne 0 ]; then
    echo "シミュレーションに失敗"
    rm *.tsv *.log
    exit -1
fi

mkdir -p result-2/$1/$2-$3
rm -rf result-2/$1/$2-$3/*

mv app-delays-trace.tsv result-2/$1/$2-$3/k-app-delays-trace-$1-$2-$3.tsv
mv drop-trace.tsv result-2/$1/$2-$3/k-drop-trace-$1-$2-$3.tsv
mv rate-trace.tsv result-2/$1/$2-$3/k-rate-trace-$1-$2-$3.tsv
mv $1-$2-$3.log result-2/$1/$2-$3/k-$1-$2-$3.log

cd result-2/$1/$2-$3
/home/strea/t2c.sh k-app-delays-trace-$1-$2-$3.tsv
/home/strea/t2c.sh k-drop-trace-$1-$2-$3.tsv
/home/strea/t2c.sh k-rate-trace-$1-$2-$3.tsv

exit 0
