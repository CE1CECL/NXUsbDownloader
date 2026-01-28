#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <fcntl.h>

#include <usb.h>

//----------------------------------------------------------------------------
//	Global variables
char *nsih_file = NULL;
char *bin_file = NULL;
char *secondboot_file = NULL;
char *processor_type = NULL;

char *down_address = NULL;
char *start_address = NULL;

int header_mode = 1;

#define HEADER_ID	((((unsigned int)'N')<< 0) | (((unsigned int)'S')<< 8) | (((unsigned int)'I')<<16) | (((unsigned int)'H')<<24))

#define	VER_STR	"Nexell USB Downloader Version 1.4.2"

//----------------------------------------------------------------------------
//	Defines
#define	NSIH_SIZE	512
enum{
	NEXELL_VID  = 0x2375,
	SAMSUNG_VID = 0x04E8,
	NXP2120_PID = 0x2120,
	NXP3200_PID = 0x3200,
	NXP4330_PID = 0x4330,
	NXP5430_PID = 0x5430,
	S5PXX18_PID = 0x1234,
	S5PXX18_2_PID = 0x04E8,
};

//----------------------------------------------------------------------------
//	Declare functions
int send_data( int vid, int pid, unsigned char *pData, int size );
int ProcessNSIH( const char *pfilename, unsigned char *pOutData );
int NXP4330_ImageDownload( unsigned int vendorId, unsigned int productId );
int NXP2120_ImageDownload( void );

static void usage(void)
{
	printf("\n%s, %s, %s\nusage : \n", VER_STR, __DATE__, __TIME__);
	printf("   -h : show usage\n");
	printf("   -f [file name]      : send file name\n");
	printf("   -n [file name]      : nsih file name\n");
	// printf("   -m [mode]           : 0 = No header, 1: NSIH Header\n");
	printf("   -b [file name]      : second boot file name(NXP4330 or higher)\n");
	printf("   -t [processor type] : select target processor type\n");
	printf("      ( type : nxp2102/nxp3200/nxp4330/nxp5430/slsiap )\n" );
	printf("   -a [address]        : download address\n");
	printf("      ( 2120=0x80100000, nxp4330=0x40100000, other=0xffff0000)\n");
	printf("   -j [address]        : jump address\n");
	printf("      ( 2120=0x80100000, nxp4330=0x40100000, other=0xffff0000)\n");
	printf("\n");
	printf(" case 1. nxp4330(5430/slsiap) second boot download\n" ); 
	printf("  #>sudo ./usb-downloader -t nxp4330 -n nsih.txt -b pyrope_secondboot.bin\n" ); 
	printf(" case 2. nxp4330 image donwload(Download : Header(512) + Image)\n" ); 
	printf("  #>sudo ./usb-downloader -t nxp4330 -n nsih.txt -f u-boot.bin\n" ); 
	printf(" case 3. nxp2120 image donwload\n" ); 
	printf("  #>sudo ./usb-downloader -t nxp2120 -n nsih.file -f u-boot.bin\n" ); 
	printf("\n");
}

static void print_downlod_info( const char *msg )
{
	printf( "============================================================\n" );
	printf( " %s, %s, %s\n", VER_STR, __DATE__, __TIME__ );
	printf( " %s\n", msg );
	printf( " processor type  : %s\n", processor_type?processor_type:"NULL" );
	printf( " nsih file       : %s\n", nsih_file?nsih_file:"NULL" );
	printf( " bin file        : %s\n", bin_file?bin_file:"NULL" );
	printf( " secondboot file : %s\n", secondboot_file?secondboot_file:"NULL" );
	printf( " download addr   : %s\n", down_address?down_address:"default" );
	printf( " start addr      : %s\n", start_address?start_address:"default" );
}


int main(int argc, char **argv)
{
	int c;
	printf("USB Download Tool\n");
	printf("\n");

	while (1) {
		// c = getopt(argc, argv, "a:j:b:n:f:t:m:h");
		c = getopt(argc, argv, "a:j:b:n:f:t:h");

		if (c == -1)
			break;
		switch (c)
		{
		case 'a':
			down_address = strdup(optarg);
			break;

		case 'j':
			start_address = strdup(optarg);
			break;

		case 'b':
			secondboot_file = strdup(optarg);
			break;

		case 'n':
			nsih_file = strdup(optarg);
			break;

		case 'f':
			bin_file = strdup(optarg);
			break;

		case 't':
			processor_type = strdup(optarg);
			break;

		// case 'm':
		// 	header_mode = atoi(optarg)? 1:0;
		// 	break;

		case 'h':
			usage();
			return 0;

		default:
			printf("unkown option parameter(%c)\n", c);
			break;
		}
	}

	if( processor_type == NULL )
	{
		printf("Error !!!, set processor type!!!\n");
		return -1;
	}

	if( !strncmp( "nxp2120", processor_type ,7 )  )
	{
		if( 0 != NXP2120_ImageDownload() )
		{
			printf("NXP2120_ImageDownload Failed\n");
			return -1;
		}
	}
	else if ( (!strncmp( "nxp4330", processor_type ,7 )) )
	{
		if( 0 != NXP4330_ImageDownload(NEXELL_VID, NXP4330_PID) )
		{
			printf("NXP4330_ImageDownload(NXP4330) Failed\n");
			return -1;
		}
	}
	else if ( (!strncmp( "nxp5430", processor_type ,7 )) )
	{
		if( 0 != NXP4330_ImageDownload(NEXELL_VID, NXP5430_PID) )
		{
			printf("NXP4330_ImageDownload(NXP5430) Failed\n");
			return -1;
		}
	}
	else if ( (!strncmp( "slsiap", processor_type ,7 )) )
	{
		if( 0 != NXP4330_ImageDownload(SAMSUNG_VID, S5PXX18_PID) )
		{
			printf("NXP4330_ImageDownload(S5PXX18) Failed\n");
			return -1;
		}
	}
	else
	{
		printf(" Unknown or unsupported processor type!!!\n");
		return -1;
	}

	return 0;
}

//============================================================================
//
//						USB Functions
//
static int is_init_usb = 0;
static usb_dev_handle *get_usb_dev_handle( int vid, int pid )
{
	struct usb_bus *bus;
	struct usb_device *dev;
	if( !is_init_usb )
	{
		usb_init();
		is_init_usb = 1;
	}
	usb_find_busses();
	usb_find_devices();
	
	for( bus = usb_get_busses(); bus ; bus = bus->next )
	{
		for ( dev = bus->devices; dev ; dev = dev->next )
		{
			if( (dev->descriptor.idVendor == vid && dev->descriptor.idProduct == pid ) ||
				(dev->descriptor.idVendor == pid && dev->descriptor.idProduct == vid ) )	//	for S5P4418 Pre
			{
				return usb_open(dev);
			}
		}
	}
	return NULL;
}



int send_data( int vid, int pid, unsigned char *pData, int size )
{
	int ret;
	usb_dev_handle *dev_handle;
	dev_handle = get_usb_dev_handle( vid, pid );

	if( NULL == dev_handle )
	{
		printf("Cannot found matching USB device.(vid=0x%04x, pid=0x%04x)\n", vid, pid);
		return -1;
	}

	if (usb_claim_interface(dev_handle, 0) < 0) {
		printf("usb_claim_interface() failed!!!\n");
		usb_close(dev_handle);
		return -1;
	}

	printf("=> Downloading %d bytes\n", size);

	ret = usb_bulk_write(dev_handle, 2, (void *)pData, size, 5*1000*1000);

	if( ret == size )
	{
		printf("=> Donwload Sucess!!!\n");
	}
	else
	{
		printf("=> Download Failed!!(ret=%d)\n", ret);
		usb_close(dev_handle);
		return -1;
	}

	usb_release_interface(dev_handle, 0);
	usb_close( dev_handle );
	return 0;
}

//
//						USB Functions
//
//============================================================================


//============================================================================
//
//						NSIH Function
//
int ProcessNSIH( const char *pfilename, unsigned char *pOutData )
{
	FILE	*fp;
	char ch;
	int writesize, skipline, line, bytesize, i;
	unsigned int writeval;

	fp = fopen( pfilename, "rb" );
	if( !fp )
	{
		printf( "ProcessNSIH : ERROR - Failed to open %s file.\n", pfilename );
		return 0;
	}

	bytesize = 0;
	writeval = 0;
	writesize = 0;
	skipline = 0;
	line = 0;

	while( 0 == feof( fp ) )
	{
		ch = fgetc( fp );
		
		if( skipline == 0 )
		{
			if( ch >= '0' && ch <= '9' )
			{
				writeval = writeval * 16 + ch - '0';
				writesize += 4;
			}
			else if( ch >= 'a' && ch <= 'f' )
			{
				writeval = writeval * 16 + ch - 'a' + 10;
				writesize += 4;
			}
			else if( ch >= 'A' && ch <= 'F' )
			{
				writeval = writeval * 16 + ch - 'A' + 10;
				writesize += 4;
			}
			else
			{
				if( writesize == 8 || writesize == 16 || writesize == 32 )			
				{
					for( i=0 ; i<writesize/8 ; i++ )
					{
						pOutData[bytesize++] = (unsigned char)(writeval & 0xFF);
						writeval >>= 8;
					}
				}
				else
				{
					if( writesize != 0 )
						printf("ProcessNSIH : Error at %d line.\n", line+1 );
				}

				writesize = 0;
				skipline = 1;
			}
		}

		if( ch == '\n' )
		{
			line++;
			skipline = 0; 
			writeval = 0;
		}
	}

	printf( "ProcessNSIH : %d line processed.\n", line+1 );
	printf( "ProcessNSIH : %d bytes generated.\n", bytesize );
	
	fclose( fp );

	return bytesize;
}
//
//						NSIH Function
//
//============================================================================

static size_t get_file_size( FILE *fd )
{
	size_t fileSize;
	long curPos;

	curPos = ftell( fd );

	fseek(fd, 0, SEEK_END);
	fileSize = ftell( fd );
	fseek(fd, curPos, SEEK_SET );

	return fileSize;
}

//============================================================================
//
//						NXP4330 Download Functions
//
static int NXP4330_SecondBootMode(unsigned int vendorId, unsigned int productId)
{
	FILE *fd_nsih = NULL;
	FILE *fd_secondboot = NULL;
	unsigned char *send_buf=NULL;
	int second_boot_send_size = 16*1024;
	int nsih_result;
	unsigned int *NSIH;
	int read_size;
	unsigned int down_addr=0xFFFF0000, start_addr=0xFFFF0000;

	if( productId == NXP4330_PID )
	{
		down_addr=0x40100000;
		start_addr=0x40100000;
	}

	if( down_address  ) down_addr  = strtoul( down_address , NULL, 16 );
	if( start_address ) start_addr = strtoul( start_address, NULL, 16 );

	print_downlod_info("Second Boot Mode");

	fd_nsih       = fopen( nsih_file      , "rb" );
	fd_secondboot = fopen( secondboot_file, "rb" );

	if( !fd_nsih || !fd_secondboot )
	{
		printf("File open failed!! check filename!!\n");
		goto ErrorExit;
	}

	if( productId == NXP4330_PID )
		second_boot_send_size = 16*1024;
	else
		second_boot_send_size = get_file_size(fd_secondboot)+NSIH_SIZE;

	send_buf = malloc( second_boot_send_size );

	nsih_result = ProcessNSIH( nsih_file, send_buf );
	if( nsih_result != NSIH_SIZE )
	{
		printf("Warning : NSIH File is wrong!!!\n");
	}

	NSIH = (unsigned int*)send_buf;
	NSIH[17] = get_file_size(fd_secondboot);
	NSIH[18] = down_addr;
	NSIH[19] = start_addr;
	NSIH[127]= HEADER_ID;

	read_size = fread( send_buf+NSIH_SIZE, 1, second_boot_send_size-NSIH_SIZE, fd_secondboot );
	if( read_size < 0 )
	{
		printf("secondboot file read failed!!!\n");
		goto ErrorExit;
	}

	send_data( vendorId, productId, send_buf, second_boot_send_size );

	free( send_buf );
	if( fd_nsih       )	fclose (fd_nsih      );
	if( fd_secondboot )	fclose (fd_secondboot);
	return 0;
ErrorExit:
	free( send_buf );
	if( fd_nsih       )	fclose (fd_nsih      );
	if( fd_secondboot )	fclose (fd_secondboot);
	return -1;
}

static int NXP4330_ImageOnly(unsigned int vendorId, unsigned int productId)
{
	FILE *fd_image = NULL;
	unsigned char *send_buf=NULL;
	int buf_size, read_size;

	fd_image = fopen( bin_file, "rb" );

	if( NULL==fd_image )
	{
		printf("File open failed!! check filename!!(%s)\n", bin_file);
		goto ErrorExit;
	}

	buf_size = get_file_size( fd_image );
	send_buf = malloc( buf_size );

	read_size = fread( send_buf, 1, buf_size, fd_image );
	if( read_size < 0 )
	{
		printf("image file read failed!!!\n");
		goto ErrorExit;
	}

	send_data( vendorId, productId, send_buf, buf_size );
	free( send_buf );
	if( fd_image )	fclose (fd_image);
	return 0;

ErrorExit:
	free( send_buf );
	if( fd_image )	fclose (fd_image);
	return -1;
}

static int NXP4330_ImageWithHeader( unsigned int vendorId, unsigned int productId )
{
	FILE *fd_nsih = NULL;
	FILE *fd_image = NULL;
	unsigned char *send_buf=NULL;
	int buf_size = 0;
	int nsih_result;
	unsigned int *NSIH;
	int read_size;
	unsigned int down_addr=0xFFFF0000, start_addr=0xFFFF0000;
	if( productId == NXP4330_PID )
	{
		down_addr=0x40100000;
		start_addr=0x40100000;
	}

	if( down_address  ) down_addr  = strtoul( down_address , NULL, 16 );
	if( start_address ) start_addr = strtoul( start_address, NULL, 16 );

	print_downlod_info("Send Image width NSHI Header");

	fd_nsih       = fopen( nsih_file      , "rb" );
	fd_image      = fopen( bin_file     , "rb" );

	if( !fd_image )
	{
		printf("File open failed!! check filename!!\n");
		goto ErrorExit;
	}

	buf_size = get_file_size(fd_image) + NSIH_SIZE;

	send_buf = malloc( buf_size );

	if( fd_nsih )
	{
		nsih_result = ProcessNSIH( nsih_file, send_buf );
		if( nsih_result != NSIH_SIZE )
		{
			printf("Warning : NSIH File is wrong!!!\n");
			goto ErrorExit;
		}
	}
	else
	{
		memset( send_buf, 0, NSIH_SIZE );
	}

	NSIH = (unsigned int*)send_buf;
	NSIH[17] = get_file_size(fd_image);
	NSIH[18] = down_addr;
	NSIH[19] = start_addr;
	NSIH[127]= HEADER_ID;

	read_size = fread( send_buf+NSIH_SIZE, 1, buf_size-NSIH_SIZE, fd_image );
	if( read_size < 0 )
	{
		printf("secondboot file read failed!!!\n");
		goto ErrorExit;
	}

	send_data( vendorId, productId, send_buf, buf_size );

	free( send_buf );
	if( fd_nsih  )	fclose (fd_nsih );
	if( fd_image )	fclose (fd_image);
	return 0;
ErrorExit:
	free( send_buf );
	if( fd_nsih  )	fclose (fd_nsih );
	if( fd_image )	fclose (fd_image);
	return -1;
}


int NXP4330_ImageDownload( unsigned int vendorId, unsigned int productId )
{
	if( secondboot_file==NULL )
	{
		if( header_mode )
		{
			//	Header(512) + Image
			return NXP4330_ImageWithHeader(vendorId, productId);
		}
		else
		{
			//	Image Only Mode
			return NXP4330_ImageOnly(vendorId, productId);
		}
	}
	else
	{
		//	Second Boot Mode ( Send 16KB )
		return NXP4330_SecondBootMode(vendorId, productId);
	}
	return 0;
}


//
//						NXP4330 Download Functions
//
//============================================================================



//============================================================================
//
//						NXP2120 Download Functions
//
int NXP2120_ImageDownload( void )
{
	FILE *fd_nsih = NULL;
	FILE *fd_image = NULL;
	unsigned char *send_buf=NULL;
	int buf_size;
	int nsih_result, read_size;
	unsigned int *NSIH;
	unsigned int down_addr=0x80100000, start_addr=0x80100000;

	if( down_address  ) down_addr  = strtoul( down_address , NULL, 16 );
	if( start_address ) start_addr = strtoul( start_address, NULL, 16 );

	print_downlod_info("Send Image Widthout Header");

	fd_nsih  = fopen( nsih_file, "rb" );
	fd_image = fopen( bin_file , "rb" );

	if( !fd_nsih || !fd_image )
	{
		printf("File open failed!! check filename!!\n");
		goto ErrorExit;
	}

	buf_size = get_file_size(fd_image) + 512;
	send_buf = malloc( buf_size );
	nsih_result = ProcessNSIH( nsih_file, send_buf );
	if( nsih_result != NSIH_SIZE )
	{
		printf("Warning : NSIH Wrong!!!\n");
	}

	NSIH = (unsigned int*)send_buf;
	NSIH[1] = get_file_size(fd_image);
	NSIH[2] = down_addr;
	NSIH[3] = start_addr;

	read_size = fread( send_buf+NSIH_SIZE, 1, buf_size-NSIH_SIZE, fd_image );
	if( read_size< 0 )
	{
		printf("image file read failed!!!\n");
		goto ErrorExit;
	}

	send_data( NEXELL_VID, NXP2120_PID, send_buf, buf_size );

	free( send_buf );
	if( fd_nsih  )	fclose (fd_nsih  );
	if( fd_image )	fclose (fd_image );
	return 0;

ErrorExit:
	free( send_buf );
	if( fd_nsih  )	fclose (fd_nsih  );
	if( fd_image )	fclose (fd_image );
	return -1;
}
//
//						NXP2120 Download Functions
//
//============================================================================
