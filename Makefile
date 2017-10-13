all: compile

compile:
	sudo platformio run -d button
upload:
	sudo platformio run -d button -t upload --upload-port /dev/ttyUSB1

