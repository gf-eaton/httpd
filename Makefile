httpd: httpd.cpp
	apt install -y libpqxx-dev
	cd libhv ; rm -fr build_x64 ; mkdir build_x64 ; cd build_x64 ; cmake .. && cmake --build . -j 4 --target libhv libhv_static
	g++ $^ -static -O3 -I/usr/include/ -I/root/httpd/libhv/build_x64/include/ -L/usr/lib/x86_64-linux-gnu/ -L/root/httpd/libhv/build_x64/lib/ -std=c++20 -lpqxx -lpq -lhv_static -o $@

test: httpd
	/usr/bin/time -f"Executed:%es\nRAM:%MKb\nSystem:%Ss" ./httpd --runfor 00:01 --config ./config-httpd.json
	/usr/bin/time -f"Executed:%es\nRAM:%MKb\nSystem:%Ss" ./mca --runfor 0000000

httpd-arm64: httpd.cpp
	apt install -y g++-12-aarch64-linux-gnu gcc-12-aarch64-linux-gnu
	apt install -y libpqxx-dev:arm64
	cd libhv ; rm -fr build_arm64 ; mkdir build_arm64 ; cd build_arm64 ; cmake .. -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc-12 -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++-12 && cmake --build . -j 4 --target libhv libhv_static
	aarch64-linux-gnu-g++-12 $^ -static -O3 -I/usr/include/ -I/root/httpd/libhv/build_arm64/include/ -L/usr/lib/aarch64-linux-gnu/ -L/root/httpd/libhv/build_arm64/lib/ -std=c++20 -lpqxx -lhv_static -o $@

httpd-pack: httpd-arm64
	rm -f httpd-arm64.zip
	zip httpd-arm64.zip httpd-arm64

#sample

requirements.txt:
	git clone https://github.com/ithewei/libhv.git
	cd libhv ; ./configure
	cd libhv ; make
	cd libhv ; make install
	touch requirements.txt

libhv-arm64:
	cd libhv ; rm -fr build_arm64 ; mkdir build_arm64 ; cd build_arm64 ; cmake .. -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc-12 -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++-12 && cmake --build . --target libhv libhv_static