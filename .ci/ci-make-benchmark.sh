#!/bin/sh

set -e
. .ci/travis.sh
if [ "$1" = "release-ready" ] ; then
  exit 0
fi
if [ "$1" = "coverity" ] ; then
  exit 0
fi
travis_fold benchmark "ninja benchmark"
echo "Nothing to do here, the benchmarks don't seem to terminate"
#else
  #if [ "$DISTRO" != "" ] ; then
    #docker exec --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) make benchmark
  #else
    #export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"
    #make benchmark
  #fi
#fi
travis_endfold benchmark
