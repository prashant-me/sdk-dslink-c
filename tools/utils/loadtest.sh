#!/bin/bash

count="$1"

broker_address=http://10.228.219.200:40000/conn

mkdir testrun1
for i in {3..3}
do
    
    mkdir testrun/load_test_pair_$i
    cp load_test_responder testrun/load_test_pair_$i
    cp load_test_requester testrun/load_test_pair_$i

    ./testrun/load_test_pair_$i/load_test_responder --broker=${broker_address} --name=responder$i &> responder_$i.out &
    ./testrun/load_test_pair_$i/load_test_requester --broker=${broker_address} --name=${i}requester &> requester_$i.out &
done