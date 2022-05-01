#!/bin/sh
DIR1=`pwd`
DIR2="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$DIR1"

if [ "$BUILD" == "" ]; then
   BUILD=$1
fi

if [ -z $NI_OS ]; then
    case `uname` in
        Darwin)
            export NI_OS=OSX
            ;;
        *)
            export NI_OS=`uname`
            ;;
    esac
fi

BIN_EXT=
case $NI_OS in
    OSX)
        case $BUILD_BIN_LOA in
            *-x86)
                export BIN_LOA=osx-x86
            ;;
            *-arm64)
                export BIN_LOA=osx-arm64
            ;;
            *)
                export BIN_LOA=osx-x64
            ;;
        esac
        ;;
    MINGW32_NT*)
        case $BUILD_BIN_LOA in
            *-x64)
                export BIN_LOA=nt-x64
                export BIN_EXT=.exe
            ;;
            *)
                export BIN_LOA=nt-x86
                export BIN_EXT=.exe
            ;;
        esac
        ;;
    NT*)
        case $BUILD_BIN_LOA in
            *-x64)
                export BIN_LOA=nt-x64
                export BIN_EXT=.exe
            ;;
            *)
                export BIN_LOA=nt-x86
                export BIN_EXT=.exe
            ;;
        esac
        ;;
    LINUX86)
        export BIN_LOA=lin-x86
        export BIN_EXT=.x86
        ;;
    LINUX64)
        export BIN_LOA=lin-x64
        export BIN_EXT=.x64
        ;;
    Linux)
        export BIN_LOA=lin-x64
        export BIN_EXT=.x64
        ;;
    *)
        echo "E/Unknown OS: '$NI_OS'"
        exit 1
        ;;
esac

if [ -e "$DIR1/$BIN_LOA/ni_$BUILD$BIN_EXT" ]; then
    export NIVM="$DIR1/$BIN_LOA/ni_$BUILD$BIN_EXT"
    export NIWVM="$DIR1/$BIN_LOA/niw_$BUILD$BIN_EXT"
elif [ -e "$DIR1/bin/$BIN_LOA/ni_$BUILD$BIN_EXT" ]; then
    export NIVM="$DIR1/bin/$BIN_LOA/ni_$BUILD$BIN_EXT"
    export NIWVM="$DIR1/bin/$BIN_LOA/niw_$BUILD$BIN_EXT"
elif [ -e "$DIR2/$BIN_LOA/ni_$BUILD$BIN_EXT" ]; then
    export NIVM="$DIR2/$BIN_LOA/ni_${BUILD}${BIN_EXT}"
    export NIWVM="$DIR2/$BIN_LOA/niw_${BUILD}${BIN_EXT}"
else
    export NIVM="cantfind_ni"
    export NIWVM="cantfind_niw"
fi

# echo "I/NIVM:" $NIVM
