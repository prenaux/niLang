#!/bin/sh
DIR1=`pwd`
DIR2="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$DIR1"
if [ "$BUILD" == "" ]; then
   BUILD=$1
fi

if [[ $OS == Windows* ]]; then
  export NI_OS=NT
  export HAM_BIN_LOA=nt-x86
  if [ -z "$HOME" ]; then
    HOME=$(unxpath "$USERPROFILE")
    export HOME
  fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
  export NI_OS=OSX
  UNAME_STR=$(uname)
  MACHINE_TYPE=$(uname -m)
  if [ "$MACHINE_TYPE" == "x86_64" ]; then
    IS_TRANSLATED=$(
      sysctl -n sysctl.proc_translated 2>/dev/null
      true
    )
    if [ "$IS_TRANSLATED" == "1" ]; then
      # echo "W/!!! RUNNING UNDER ROSETTA, forcing HAM_BIN_LOA=osx-arm64 !!!"
      export HAM_BIN_LOA=osx-arm64
    else
      export HAM_BIN_LOA=osx-x64
    fi
  elif [ "$MACHINE_TYPE" == "arm64" ]; then
    export HAM_BIN_LOA=osx-arm64
  else
    echo "W/Unknown OS '$UNAME_STR' '$MACHINE_TYPE'"
  fi
  # export HAM_LOCAL_HOMEBREW="$HAM_HOME/toolsets/_brew/$HAM_BIN_LOA"
else
  UNAME_STR=$(/bin/uname)
  MACHINE_TYPE=$(/bin/uname -m)
  if [ "$UNAME_STR" == "Linux" ] && [ "$MACHINE_TYPE" == "x86_64" ]; then
    export NI_OS=LINUX
    export HAM_BIN_LOA=lin-x64
    # export HAM_LOCAL_HOMEBREW="$HAM_HOME/toolsets/_brew/$HAM_BIN_LOA"
  else
    echo "W/Unknown OS '$UNAME_STR' '$MACHINE_TYPE'"
    exit 1
  fi
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
    NT*)
        case $BUILD_BIN_LOA in
            *)
                export BIN_LOA=nt-x64
                export BIN_EXT=.exe
            ;;
            *-x86)
                export BIN_LOA=nt-x86
                export BIN_EXT=.exe
            ;;
        esac
        ;;
    LINUX*)
        export BIN_LOA=lin-x64
        export BIN_EXT=.x64
        ;;
    *)
        echo "E/ni-bash-setenv: Unknown OS: '$NI_OS'"
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
