#!/bin/bash
rm -rf /opt/sigil/bin
rm -rf /opt/sigil/build
cd /opt/sigil
git submodule deinit --all --force