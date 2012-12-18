#! /bin/bash

# this test checks if the strigicmd utility properly detect creation and
# deletion of a file

function fail() {
    echo Test failed
    exit 1
}

#VG="valgrind --db-attach=yes "

rm -r x y
mkdir x
touch x/y
touch x/z
echo == src/strigicmd/strigicmd create -t clucene -d y x ==
if ! $VG src/strigicmd/strigicmd create -t clucene -d y x; then
    fail
fi
echo == src/strigicmd/strigicmd listFiles -t clucene -d y ==
if ! $VG src/strigicmd/strigicmd listFiles -t clucene -d y; then
    fail
fi
sleep 1
touch x/y
touch x/z
echo == src/strigicmd/strigicmd update -t clucene -d y x ==
exit
if ! $VG src/strigicmd/strigicmd update -t clucene -d y x; then
    fail
fi
echo == src/strigicmd/strigicmd listFiles -t clucene -d y ==
if ! $VG src/strigicmd/strigicmd listFiles -t clucene -d y; then
    fail
fi
OUT=`$VG src/strigicmd/strigicmd listFiles -t clucene -d y`
if [[ $OUT == 'x/z' ]]; then
    echo Test succesfull
    exit 0
fi
fail
