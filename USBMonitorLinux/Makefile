all:
	g++ -static -std=c++11 -o usb_monitor main.cpp src/logger.cpp src/control.cpp src/base.cpp src/monitor.cpp -lusb-1.0 -lpthread -ludev
#	g++ -std=c++11 -o usb_monitor main.cpp src/logger.cpp src/control.cpp src/base.cpp src/monitor.cpp -lusb-1.0 -lpthread