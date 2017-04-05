all: dmcache sacache

dmcache : dmcache.cpp
	g++ -o dmcache -Wall dmcache.cpp

sacache : sacache.cpp
	g++ -o sacache -Wall sacache.cpp


