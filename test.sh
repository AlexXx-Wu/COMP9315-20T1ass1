#!/bin/sh
cp ~/Documents/COMP9315-20T1ass1/pname.c .
cp ~/Documents/COMP9315-20T1ass1/pname.source .
cp ~/Documents/COMP9315-20T1ass1/data0.sql .

make
dropdb test
creatdb test
psql test -f pname.sql
psql test -f data0.sql