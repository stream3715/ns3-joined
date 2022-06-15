#!/bin/bash
if [ $# -ne 2 ]; then
    echo "指定された引数は$#個です。" 1>&2
    echo "実行するには2個の引数が必要です。" 1>&2
    exit 1
fi

export ID_PRD=37
export ID_CON_MJ1=10
export ID_CON_MJ2=21
export ID_CON_MJ3=27
export ID_CON_MN1=8
export ID_CON_MN2=16
export ID_CON_MN3=33

./waf --run=ndn-kademlia >$2.log

if [ $? -ne 0 ]; then
    echo "シミュレーションに失敗"
    rm *.tsv *.log
    exit -1
fi

mkdir -p result/$1/$2
rm -rf result/$1/$2/*

mv app-delays-trace.tsv result/$1/$2/k-app-delays-trace-$2.tsv
mv drop-trace.tsv result/$1/$2/k-drop-trace-$2.tsv
mv rate-trace.tsv result/$1/$2/k-rate-trace-$2.tsv
mv $2.log result/$1/$2/k-$2.log

cd result/$1/$2
/home/strea/t2c.sh k-app-delays-trace-$2.tsv
/home/strea/t2c.sh k-drop-trace-$2.tsv
/home/strea/t2c.sh k-rate-trace-$2.tsv

exit 0
