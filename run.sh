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

mkdir -p result-2ml/$1/$2-$3
rm -rf result-2ml/$1/$2-$3/*

mv app-delays-trace.tsv result-2ml/$1/$2-$3/k-2ml-app-delays-trace-$1-$2-$3.tsv
mv drop-trace.tsv result-2ml/$1/$2-$3/k-2ml-drop-trace-$1-$2-$3.tsv
mv rate-trace.tsv result-2ml/$1/$2-$3/k-2ml-rate-trace-$1-$2-$3.tsv
mv $1-$2-$3.log result-2ml/$1/$2-$3/k-2ml-$1-$2-$3.log

cd result-2ml/$1/$2-$3
/home/strea/t2c.sh k-2ml-app-delays-trace-$1-$2-$3.tsv
/home/strea/t2c.sh k-2ml-drop-trace-$1-$2-$3.tsv
/home/strea/t2c.sh k-2ml-rate-trace-$1-$2-$3.tsv

exit 0
