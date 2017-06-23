#!/bin/bash

killall -9 load_test_responder
killall -9 load_test_requester

rm -rf testrun
rm -rf *.out