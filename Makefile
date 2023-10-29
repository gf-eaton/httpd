httpd: httpd.cpp
	apt install -y libpqxx-dev
	g++ $^ -O3 -I/usr/include/ -std=c++20 -lpqxx -o $@

test: httpd
	/usr/bin/time -f"Executed:%es\nRAM:%MKb\nSystem:%Ss" ./mca --runfor 00:01 --id 1 --assetid 1 --config ./config-mca.json
	/usr/bin/time -f"Executed:%es\nRAM:%MKb\nSystem:%Ss" ./mca --runfor 0000000

httpd-arm64: httpd.cpp
	apt install -y g++-12-aarch64-linux-gnu gcc-12-aarch64-linux-gnu
	apt install -y libpqxx-dev:arm64
	aarch64-linux-gnu-g++-12 $^ -O3 -I/usr/include/modbus/ -I/usr/include/ -L/usr/lib/aarch64-linux-gnu/ -std=c++20 -lpqxx -o $@

mca-pack: httpd-arm64
	rm -f httpd-arm64.zip
	cp /usr/lib/aarch64-linux-gnu/libpqxx-6.4.so .
	zip httpd-arm64.zip libpqxx-6.4.so httpd-arm64
	rm -f libpqxx-6.4.so

requirements.txt:
	git clone https://github.com/ithewei/libhv.git
	cd libhv ; ./configure
	cd libhv ; make
	cd libhv ; make install
	touch requirements.txt
