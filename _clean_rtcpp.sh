#!/bin/bash -e
(
  set -ex ;
  ham-glob-files "*[0-9]_rtcpp.*" | xargs -t rm -f
)
