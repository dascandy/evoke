#!/bin/bash

cpack
VERSION=`ls | grep evoke_.*.deb | sed -e 's/.*evoke_//' -e 's/\.deb//'` 

curl -X PUT https://apt.dascandy.nl/upload?name=evoke-linux\&version=$VERSION\&key=$APIKEY --data-binary @evoke_$VERSION.deb
