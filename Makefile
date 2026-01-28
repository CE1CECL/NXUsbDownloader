.PHONY: clean

usb-downloader:
	$(CC) -O2 -Wall -g -o usb-downloader usb-downloader.c -lusb

clean:
	rm -f usb-downloader
