#!/bin/bash -e
if [ "$1" = "doit" ]; then
    (set -x ;
     git clean -d -f -x ;
     rm -Rf "$TEMPDIR/obj/nilang-"*)
else
    (set -x ;
     git clean -d -f -x -n ;
     ls "$TEMPDIR/obj/nilang-"*)
    echo "# Type './_clean_deep.sh doit' to remove the files listed above."
fi
