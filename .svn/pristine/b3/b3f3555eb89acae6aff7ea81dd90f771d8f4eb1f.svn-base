#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <asm/types.h> 
#include <linux/videodev2.h>
#include <linux/fb.h>
 
 
static unsigned int capframe = 0;
static unsigned char filename[30];
FILE *bmpFile;
 
unsigned char bmp_head_t[] = {
        0x42,0x4d,0x42,0x58,0x02,0x00,0x00,0x00,0x00,0x00,
        0x42,0x00,0x00,0x00,0x28,0x00,0x00,0x00,0xf0,0x00,
        0x00,0x00,0x40,0x01,0x00,0x00,0x01,0x00,0x10,0x00,
        0x03,0x00,0x00,0x00,0x00,0x58,0x02,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0xf8,0x00,0x00,0xe0,0x07,
        0x00,0x00,0x1f,0x00,0x00,0x00
};
 
 
static int fb_bpp;
 
//14byte      
typedef struct     
{     
  char cfType[2];         /* 文件类型, 必须为 "BM" (0x4D42)*/     
  unsigned int cfSize;         /* 文件的大小(字节) */     
  unsigned int cfReserved;     /* 保留, 必须为 0 */     
  unsigned int cfoffBits;      /* 位图阵列相对于文件头的偏移量(字节)*/     
}__attribute__((packed)) BITMAPFILEHEADER;       /* 文件头结构 */     
  
//40byte      
typedef struct     
{     
  unsigned int ciSize;         /* size of BITMAPINFOHEADER */     
  unsigned int ciWidth;        /* 位图宽度(像素) */     
  unsigned int ciHeight;       /* 位图高度(像素) */     
  unsigned short ciPlanes;       /* 目标设备的位平面数, 必须置为1 */     
  unsigned short ciBitCount;     /* 每个像素的位数, 1,4,8或24 */     
  unsigned int ciCompress;     /* 位图阵列的压缩方法,0=不压缩 */     
  unsigned int ciSizeImage;    /* 图像大小(字节) */     
  unsigned int ciXPelsPerMeter;/* 目标设备水平每米像素个数 */     
  unsigned int ciYPelsPerMeter;/* 目标设备垂直每米像素个数 */     
  unsigned int ciClrUsed;      /* 位图实际使用的颜色表的颜色数 */     
  unsigned int ciClrImportant; /* 重要颜色索引的个数 */     
}__attribute__((packed)) BITMAPINFOHEADER;       /* 位图信息头结构 */    
 
BITMAPFILEHEADER FileHead;     
BITMAPINFOHEADER InfoHead; 
 
unsigned char bmp_head[54];
 
static int fb_fp = -1;
static char *fb_addr = NULL;
 
int width=0;
int height=0;
 
static inline int fb_init(void)
{
	int dev_fp = -1;
	int fb_size;
	struct fb_var_screeninfo vinfo;
 
	dev_fp = open("/dev/fb0", O_RDWR);
	if (dev_fp < 0) {
		perror("/dev/fb0");
		return -1;
	}
	if (ioctl(dev_fp, FBIOGET_VSCREENINFO, &vinfo)) {
	        printf("Error reading variable information.\n");
		exit(1);
	}
	width=vinfo.xres;
	height=vinfo.yres;
	fb_bpp=vinfo.bits_per_pixel;
	//if(fb_bpp==24) fb_bpp=32;
	fb_size=width*height*fb_bpp/8;
	if ((fb_addr = (char*)mmap(0, fb_size, 
			PROT_READ | PROT_WRITE, MAP_SHARED, dev_fp, 0)) < 0) {
		perror("mmap()");
		return -1;
	}
	printf("%dx%d bpp:%d mmaped 0x%08x\n",width,height,fb_bpp,fb_addr);
 
	return dev_fp;
}
 
void writeImageToFile(unsigned int size)
{
	sprintf(filename,"/root/0%d.bmp",capframe);
   	bmpFile=fopen(filename, "w+");
	if(fb_bpp == 16)
		fwrite(bmp_head_t,1,66,bmpFile);
	else
	{
		fwrite(&FileHead,1,14,bmpFile);
		fwrite(&InfoHead,1,40,bmpFile);
	}
   	//fwrite(fb_addr,1,size,bmpFile);
	int h = 0;
	for (h = height-1; h > 0; h--)
	{
		fwrite(fb_addr + h * width*fb_bpp/8, 1, width*fb_bpp/8, bmpFile);
	}
   	fclose(bmpFile);
 
	printf("%s\n", filename);
}
 
int main(int argc, char *argv[])
{
	unsigned int i = 10;
	unsigned long size = 0;
	fb_fp = fb_init();	
	size=width*height*fb_bpp/8;
 
	if (argc < 2)
	{
		printf("run like 'prntscr num', num is bmpname'\n");
		return 0;
	}
 
	capframe = atoi(argv[1]);
 
	if(fb_bpp==16){
	    *((unsigned int*)(bmp_head_t+18)) = width;
	    *((unsigned int*)(bmp_head_t+22)) = height;
	    *((unsigned short*)(bmp_head+28)) = 16;
	}else{
		FileHead.cfType[0] = 'B';
		FileHead.cfType[1] = 'M';
		unsigned int tmp = width*height*fb_bpp/8;+54;
		FileHead.cfSize = tmp;
		FileHead.cfoffBits = 54;
		InfoHead.ciSize = 40;
		InfoHead.ciWidth = width;
		InfoHead.ciHeight = height;
		InfoHead.ciBitCount = 32;
		InfoHead.ciCompress = 0;
		InfoHead.ciSizeImage = size;
	}
 
	//while(i--)
	{
		writeImageToFile(size);
 
	}
	 
 
	return 0;
}
