cc=g++
target=keyword_finder
jsoncpp_path=/usr/local/lib/libjsoncpp.a

all:
	$(cc) -c ./*.cpp
	$(cc) -o $(target) ./*.o $(jsoncpp_path)
	rm -rf ./*.o

clean:
	rm -rf ./*.o	