#!/bin/bash -e
export HAM_NO_VER_CHECK=1
if [[ -z "$HAM_HOME" ]]; then echo "E/HAM_HOME not set !"; exit 1; fi
. "$HAM_HOME/bin/ham-bash-setenv.sh"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

SERVER_DIR=$WORK/niLang/_http_server

(set -x ;

 # just delete the previous server folder
 rm -Rf $SERVER_DIR/ ;

 # index.html
 mkdir -p $SERVER_DIR/www ;
 ln -s $WORK/niLang/sources/http_server/www/index.html $SERVER_DIR/www/index.html ;

 # niLang
 mkdir -p $SERVER_DIR/www/niLang ;
 mkdir -p $SERVER_DIR/www/niLang/bin ;
 ln -s $WORK/niLang/bin/web-js $SERVER_DIR/www/niLang/bin/web-js ;
 ln -s $WORK/niLang/data $SERVER_DIR/www/niLang/data ;
 ln -s $WORK/niLang/sources/http_server/www/index.html $SERVER_DIR/www/niLang/index.html ;

 # Run server
 cd "$SERVER_DIR/www" ;
 simple-http-server -x -e -d "$SERVER_DIR/www" -p 20801 regular ;
)
