#!/bin/bash

HOST=$1
PROJECT_ID=$2

while true
do
  python3 daemon.py --host $HOST --project_id $PROJECT_ID
  sleep 1
done
