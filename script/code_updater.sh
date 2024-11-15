#!/bin/bash

TARGET_URI="http://$CALLED_IP/$1"
LOCAL_PATH="download"
LOCAL_FILE="temp.zip"
TARGET_PATH=$2

if [[ -z "$2" ]] ; then
    TARGET_PATH="remote"
fi

echo "CALLED_IP = $CALLED_IP"
echo "TARGET_URI = $TARGET_URI"
echo "LOCAL_PATH = $LOCAL_PATH"
echo "TARGET_PATH = $TARGET_PATH"

cd ../local_web

mkdir -p $LOCAL_PATH

if [ -d "$LOCAL_PATH" ] ; then
    echo "Path check OK"
else
    echo "Local path is not exist!"
    exit 1
fi

cd $LOCAL_PATH

wget $TARGET_URI -O $LOCAL_FILE
unzip -o $LOCAL_FILE
rm $LOCAL_FILE

cd ..
cp -rv ./$LOCAL_PATH/* ./$TARGET_PATH/
rm -rf ./$LOCAL_PATH/*

echo "Done"


