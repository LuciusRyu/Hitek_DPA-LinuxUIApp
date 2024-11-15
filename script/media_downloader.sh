#!/bin/bash

TARGET_URI="http://$CALLED_IP/$1"
LOCAL_PATH="$2"

echo "CALLED_IP = $CALLED_IP"
echo "TARGET_URI = $TARGET_URI"
echo "LOCAL_PATH = $LOCAL_PATH"

mkdir -p $LOCAL_PATH
cd $LOCAL_PATH

wget $TARGET_URI -O $LOCAL_FILE $3

echo "Done"


