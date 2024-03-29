#!/bin/sh

git clean -xdff
git submodule foreach --recursive 'git clean -xdff'
