#!/bin/bash

TARGET_URI="http://$CALLED_IP/$1"
LOCAL_PATH="../local_web/download"
LOCAL_FILE="temp.zip"

echo "CALLED_IP = $CALLED_IP"
echo "TARGET_URI = $TARGET_URI"

cd $LOCAL_PATH

wget $TARGET_URI -O $LOCAL_FILE
unzip -o $LOCAL_FILE
rm $LOCAL_FILE

echo "Done"


