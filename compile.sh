#!/bin/bash

make all -j16
make modules_install -j16
make install -j16
