#! /bin/bash
#
# (c) Jos van den Oever  <jos@vandenoever.info>
# modified by (c) Ben van Klinken <ustramooner@users.sourceforge.net
#
# This script checks out clucen and runs the given unit test. If the unit test
# fails, it goes back to the previous version, compiles and tests again.
# This goes on until the unit test is not present or runs successfully.

# check the arguments
if (( $# != 1 && $# != 2)); then
    echo Usage: $0 testname
    echo Note: This script must be run from the base
    exit
fi

# the path to the unit test executable
TESTNAME=$1

## Configuration parameters

# the maximal number of log entries to download
MAXLOGENTRIES=300

# the maximal number of steps you wish to take
MAXSTEPS=300

# make exectable with arguments
MAKE="make"

# should we do a drastic cleanup between runs or not?
FORCEFULLBUILD=0

#get the source dir
SOURCEDIR=`grep CMAKE_HOME_DIRECTORY CMakeCache.txt |perl -pi -e 's/.*=(.*)/\1/'`
if [ "$SOURCEDIR" == "" ]; then
    echo "Run cmake before running this script"
    exit 1
fi

####################

# function for testing a particular test in a particular revision
function runTest {
    REVISION=$1
    echo Testing revision $REVISION.
    
    
    exit 0

    # go back to the given revision
    cd $TESTDIR
    svn update $MODULE -r $REVISION $SOURCEDIR
    if (( $? != 0 )); then
        # if updating failed, we have to get a fresh version
        rm -rf $TESTDIR/$MODULE
        svn checkout -r $REVISION $SVNURL $SOURCEDIR
        if (( $? != 0 )); then exit; fi
    fi

    # configure the code
    # if we cannot configure the test, we continue to the next revision number
    if (( $FORCEFULLBUILD == 1 )); then
        rm -rf $TESTDIR/$MODULE/build
    fi
    mkdir $TESTDIR/$MODULE/build
    cd $TESTDIR/$MODULE/build
    cmake $SOURCEDIR
    if (( $? != 0 )); then return; fi

    # get the name of the unit test and build it
    # if we cannot build the test, we continue to the next revision number
    echo $MAKE $TESTNAME
    $MAKE $TESTNAME
    if (( $? != 0 )); then return; fi

    # find the test executable
    TESTPATH=`find -name $TESTNAME -type f -perm -u+x`

    # run the unit test and exit if it ran without error
    $TESTPATH
    if (( $? == 0 )); then
        echo The last revision where the test $TESTNAME worked was $REVISION.
        BROKEN=`grep -B 1 $REVISION $TESTDIR/revisions |head -1`
        echo The first revision that was broken was $BROKEN:
        svn log -r $BROKEN $TESTDIR/$MODULE $SOURCEDIR
        exit
    fi
}

# determine the URL of the svn repository
SVNURL=`svn info $SOURCEDIR | grep -m 1 '^URL: ' | cut -b 6-`
if (( $? != 0 )); then exit; fi

echo $SVNURL

# determine the module name
MODULE=`basename $SVNURL`

echo $MODULE

# get the last 100 relevant version numbers
svn log $MODULE --limit $MAXLOGENTRIES --non-interactive $SOURCEDIR\
	| grep -E '^r[0123456789]+' \
	| perl -pi -e 's/^r(\d+).*/\1/' | head -n $MAXSTEPS > revisions
if (( $? != 0 )); then exit; fi

for REVISION in `cat revisions`; do
    runTest $REVISION;
done

echo No revision was found in which the unit test worked.

