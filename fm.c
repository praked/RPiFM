#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <unistd.h>

volatile unsigned char *allof7e;

#define CM_GP0CTL (0x7e101070)
#define GPFSEL0 (0x7E200000)
#define CM_GP0DIV (0x7e101074)
#define GPIO_BASE (0x7E200000)

#define ACCESS(offset, type) (*(volatile type*)(offset+(int)allof7e-0x7e000000))
#define SETBIT(base, bit) ACCESS(base,int) |= 1<<bit
#define CLRBIT(base, bit) ACCESS(base,int) &= ~(1<<bit)

struct GPCTL {
        char SRC         : 4;
        char ENAB        : 1;
        char KILL        : 1;
        char             : 1;
        char BUSY        : 1;
        char FLIP        : 1;
        char MASH        : 2;
        unsigned int     : 13;
        char PASSWD      : 8;
};

void setup_fm() {
        int mem_fd = open("/dev/mem", O_RDWR|O_SYNC);
        if (mem_fd < 0) {
                printf("can't open /dev/mem\n");
                exit(-1);
        }
        allof7e = (unsigned char *)mmap(
                        NULL,
                        0x01000000,
                        PROT_READ|PROT_WRITE,
                        MAP_SHARED,
                        mem_fd,
                        0x3f000000
                        );

        if (allof7e == (unsigned char *)-1) {
                exit(-1);
        }

        close(mem_fd);

        SETBIT(GPFSEL0 , 14);
        CLRBIT(GPFSEL0 , 13);
        CLRBIT(GPFSEL0 , 12);

        ACCESS(CM_GP0CTL, struct GPCTL) = (struct GPCTL) {6, 1, 0, 0, 0, 1, 0x5a };
}

void close_fm() {
        static int close = 0;
        if (!close) {
                close = 1;
                printf("Closing Fm\n");
                ACCESS(CM_GP0CTL, struct GPCTL) = (struct GPCTL) {6, 0, 0, 0, 0, 0, 0x5a };
                exit(0);
        }
}

void modulate(int div){
         ACCESS(CM_GP0DIV,int) = (0x5a << 24) + div;
}

void delay(int n) {
        int clock = 0;
        for (int i = 0; i < n; ++i) {
                ++clock;
        }
}

void playWav(char *filename, int div, float bandwidth,int del) {
        int fp = open(filename, 'r');
        lseek(fp, 22, SEEK_SET); //Skip header
        short *data = (short *)malloc(1024);
        printf("Now broadcasting: %s\n", filename);

        while (read(fp, data, 1024)) {

                for (int j = 0; j<1024/2; j++) {
                        float divif = (int)floor((float)(data[j])/65536.0f*bandwidth);

                        modulate(divif+div);
                        delay(del);
                }
        }
}

int main(int argc, char **argv) {
        signal(SIGTERM, &close_fm);
        signal(SIGINT, &close_fm);
        atexit(&close_fm);
        
        setup_fm();

        float freq = atof(argv[2]);
        float bandwidth;
        int mod = (500/freq)*4096;
        modulate(div);
        int del;

        if (argc==3) {
                del = 2750;
                bandwidth = 100.0;
                printf("Setting up modulation: %f Mhz / %d @ %f\n",freq,div,bandwidth);
                playWav(argv[1], div, bandwidth,del);
        }
        else if (argc==5) {
          del = atoi(argv[4]);
          bandwidth = atof(argv[3]);
          printf("Setting up modulation: %f Mhz / %d @ %f\n",freq,div,bandwidth,delay);
          playWav(argv[1], div, bandwidth,del);
          }
        else {
                fprintf(stderr,
                                "Usage: %s wavfile.wav freq [power]\n\n"
                                "Where wavfile is 16 bit 22.050kHz Mono\n"
                                "Power will default to 25 if not specified. It should only be lowered!",argv[0]);
        }
        return 0;
}
                                                                                                                                                                        131,1         Bot


                                                                                                                                                                        103,0-1       62%
