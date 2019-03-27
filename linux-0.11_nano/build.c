#include <stdio.h>	/* fprintf */
#include <string.h>
#include <stdlib.h>	/* contains exit */
#include <sys/types.h>	/* unistd.h needs this */
#include <sys/stat.h>
#include <io.h>
#include <fcntl.h>

#define SYS_SIZE           0x6000
#define DEFAULT_MAJOR_ROOT 2
#define DEFAULT_MINOR_ROOT 0x1d

/* max nr of sectors of setup: don't change unless you also change bootsect etc */
#define SETUP_SECTS  4
#define STRINGIFY(x) #x
#define MAJOR(a)     (((unsigned)(a))>>8)
#define MINOR(a)     ((a)&0xff)

void die(char * str)
{
    fprintf(stderr, "%s\n", str);
    exit(1);
}

void usage(void)
{
    die("Usage: build bootsect setup system [rootdev] [> image]");
}

int main(int argc, char **argv)
{
    unsigned int i, c, id;
    char buf[1024];
    char major_root, minor_root;

    struct stat sb;
    int size = 512 + 4 * 512;
    FILE* imageFile = fopen("Boot.img", "wb");

    if ((argc != 4) && (argc != 5))
        usage();

    if (argc == 5)
    {
        if (strcmp(argv[4], "FLOPPY"))
        {
            if (stat(argv[4], &sb))
            {
                perror(argv[4]);
                die("Couldn't stat root device.");
            }

            major_root = MAJOR(sb.st_rdev);
            minor_root = MINOR(sb.st_rdev);
        }
        else
        {
            major_root = 0;
            minor_root = 0;
        }
    }
    else
    {
        major_root = DEFAULT_MAJOR_ROOT;
        minor_root = DEFAULT_MINOR_ROOT;
    }

    fprintf(stderr, "Root device is (%d, %d)\n", major_root, minor_root);

    if ((major_root != 2) && (major_root != 3) && (major_root != 0))
    {
        fprintf(stderr, "Illegal root device (major = %d)\n", major_root);
        die("Bad root device --- major #");
    }
    for (i = 0; i < sizeof buf; i++)
        buf[i] = 0;
    if ((id = open(argv[1], O_RDONLY | O_BINARY, 0)) < 0)//二进制方式不然读出大小会小于512
        die("Unable to open 'boot'");
    i = read(id, buf, sizeof buf);
    fprintf(stderr, "Boot sector %d bytes. offset 0x0, end 0x200\n", i);
    if (i != 512)
        die("Boot block must be exactly 512 bytes");
    if ((*(unsigned short *)(buf + 510)) != 0xAA55)
        die("Boot block hasn't got boot flag (0xAA55)");
    buf[508] = 0;//--必须为零,否则mout根文件系统时出错
    buf[509] = 0;
    i = fwrite(buf, 1, 512, imageFile); /* 写入bootsect.bin, 0号扇区, 偏移0x0000 */
    if (i != 512)
        die("Write call failed");
    close(id);
    if ((id = open(argv[2], O_RDONLY | O_BINARY, 0)) < 0)
        die("Unable to open 'setup'");
    for (i = 0; (c = read(id, buf, sizeof buf)) > 0; i += c)
        if (fwrite(buf, 1, c, imageFile) != c) /* 写入setup.bin, 1号扇区, 偏移0x0200 */
            die("Write call failed");
    close(id);
    if (i > SETUP_SECTS * 512)
        die("Setup exceeds " STRINGIFY(SETUP_SECTS)" sectors - rewrite build/boot/setup");
    fprintf(stderr,"Setup is %d bytes. offset 0x%04x, end 0x%04x\n", i, 0x200, 0xA00);
    for (c = 0; c < sizeof(buf); c++)
        buf[c] = '\0';
    while (i < SETUP_SECTS * 512)
    {
        c = SETUP_SECTS * 512 - i;

        if (c > sizeof(buf))
            c = sizeof(buf);

        if (c != fwrite(buf, 1, c, imageFile)) /* setup.bin补全4个扇区 */
            die("Write call failed");

        i += c;
    }

    if ((id = open(argv[3], O_RDONLY | O_BINARY, 0)) < 0)
        die("Unable to open 'system'");

    for (i = 0; (c = read(id, buf, sizeof buf)) > 0; i += c)
        if (fwrite(buf, 1, c, imageFile) != c) /* 写入system.bin4号扇区开始写偏移0x0A00 */
            die("Write call failed");

    close(id);
    size += i;

    /* MinGW32 编译然后用objcopy.cpp提取的system.bin占用空间136.5K */
    fprintf(stderr, "System is %d bytes. offset 0x%04x, end 0x%08x\n", i, 0x0A00, 0x0A00 + i);
    if (i > SYS_SIZE * 16)
        die("System is too big");
    memset(buf, 0, sizeof(buf));
    fprintf(stderr, "total size is  %d bytes.\n", 1440 * 1024);

    while (size < 1440 * 1024)//1475084, 1474560
    {
        c = 1440 * 1024 - size;

        if (c > sizeof(buf))
        {
            c = sizeof(buf);
        }

        c = fwrite(buf, 1, c, imageFile);
        size += c;
    }
    fclose(imageFile);
    return(0);
}
