all: clean usb-downloader usb-downloader.exe

usb-downloader:
	$(CC) -o usb-downloader usb-downloader.c -lusb -lusb-1.0 -static

usb-downloader.exe:
	i686-w64-mingw32-gcc -o usb-downloader.exe usb-downloader.c -lusb -lusb-1.0 -static -march=i386 -mtune=i386 -UWINVER -U_WIN32_WINNT -DWINVER=0x0400 -D_WIN32_WINNT=0x0400

clean:
	rm -f usb-downloader usb-downloader.exe
