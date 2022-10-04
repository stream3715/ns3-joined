#!/bin/bash
if [ $# -ne 4 ]; then
    echo "指定された引数は$#個です。" 1>&2
    echo "実行するには3個の引数が必要です。" 1>&2
    exit 1
fi

export ID_PRD=$2
export ID_CON_MJ=$3
export ID_CON_MN=$4

./waf --run=ndn-kademlia > $2-$3-$4.log

if [ $? -ne 0 ]; then
    echo "シミュレーションに失敗"
    rm *.tsv *.log
    exit -1
fi

mkdir -p result-$1/$2/$3-$4
rm -rf result-$1/$2/$3-$4/*

mv app-delays-trace.tsv result-$1/$2/$3-$4/k-$1-app-delays-trace-$2-$3-$4.tsv
mv drop-trace.tsv result-$1/$2/$3-$4/k-$1-drop-trace-$2-$3-$4.tsv
mv rate-trace.tsv result-$1/$2/$3-$4/k-$1-rate-trace-$2-$3-$4.tsv
mv $2-$3-$4.log result-$1/$2/$3-$4/k-$1-$2-$3-$4.log

cd result-$1/$2/$3-$4
/home/strea/t2c.sh k-$1-app-delays-trace-$2-$3-$4.tsv
/home/strea/t2c.sh k-$1-drop-trace-$2-$3-$4.tsv
/home/strea/t2c.sh k-$1-rate-trace-$2-$3-$4.tsv

exit 0
