#if 0
mkdir pictures_vga
gcc parser.c -std=c11 -o parser && ./parser $@
exit
#endif

// scaled images will be unfiltered to preserve the original feel
#define STBIR_DEFAULT_FILTER_UPSAMPLE STBIR_FILTER_BOX

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <ctype.h>

unsigned char PAL[256][3];
int picOffset = 0;
int picCount = 0;
int scale = 1;
int alpha = 0;

void loadPalette(const char *palName)
{
    FILE *palFile = fopen(palName, "rb");
    if (!palFile)
    {
        printf("Could not open palette file: %s\n", palName);
        exit(1);
    }

    // palette file contains 256 triplets of RGB values
    fread(PAL, sizeof(unsigned char), 256 * 3, palFile);
    fclose(palFile);
}

void saveImage(int w, int h, int channels, unsigned char *buffer)
{
    unsigned char rgbData[w * h * channels];
    char filename[64];

    int index = 0;
    for (int i = 0; i < w * h; ++i)
    {

        rgbData[index++] = PAL[buffer[i]][0];
        rgbData[index++] = PAL[buffer[i]][1];
        rgbData[index++] = PAL[buffer[i]][2];
        if (alpha)
            rgbData[index++] = PAL[buffer[i]][0] == 0xFF && PAL[buffer[i]][1] == 0x00 && PAL[buffer[i]][2] == 0xFF ? 0x00 : 0xFF;
    }

    memset(filename, 0, sizeof(filename));
    sprintf(filename, "pictures_vga/%04d.png", picCount);

    if (scale > 1)
    {
        unsigned char *rgbDataScaled = (unsigned char *)malloc(scale * scale * w * h * channels);

        stbir_resize_uint8(rgbData, w, h, 0, rgbDataScaled, scale * w, scale * h, 0, channels);
        stbi_write_png(filename, scale * w, scale * h, channels, rgbDataScaled, scale * w * channels);

        free(rgbDataScaled);
    }
    else
    {
        stbi_write_png(filename, w, h, channels, rgbData, w * channels);
    }
}

void fetchImages(FILE *picturesVga, unsigned char *buffer, int w, int h, int c)
{
    int channels = alpha ? 4 : 3;
    for (int i = 0; i < c; ++i)
    {
        fseek(picturesVga, picOffset, 0);
        picOffset += fread(buffer, sizeof(unsigned char), w * h, picturesVga);
        saveImage(w, h, channels, buffer);

        picCount++;
    }
}

int main(int argc, char **argv)
{
    unsigned char buffer[318 * 198];
    
    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-s") && i < argc - 1)
        {
            if (isdigit(argv[i+1][0]))
                scale = atoi(argv[i+1]);
        }
        
        if (!strcmp(argv[i], "-a"))
        {
            alpha = 1;
        }
    }
    
    
    FILE *picturesVga = fopen("PICTURES.VGA", "rb");
    if (!picturesVga)
    {
        printf("Could not load PICTURES.VGA!\n");
        return 1;
    }

    // fullscreen backgrounds
    loadPalette("sw_logo.pal");
    fetchImages(picturesVga, buffer, 318, 198, 1);
    loadPalette("primary.pal");
    fetchImages(picturesVga, buffer, 318, 198, 4);
    loadPalette("restoration.pal");
    fetchImages(picturesVga, buffer, 318, 198, 1);
    loadPalette("intro.pal");
    fetchImages(picturesVga, buffer, 318, 198, 6);
    loadPalette("title.pal");
    fetchImages(picturesVga, buffer, 318, 198, 1);
    loadPalette("primary.pal");
    fetchImages(picturesVga, buffer, 318, 198, 2);
    loadPalette("order.pal");
    fetchImages(picturesVga, buffer, 318, 198, 1);
    loadPalette("webfoot.pal");
    fetchImages(picturesVga, buffer, 318, 198, 1);
    loadPalette("primary.pal");
    fetchImages(picturesVga, buffer, 318, 198, 6);
    // scene objects and walls
    fetchImages(picturesVga, buffer, 210, 105, 34);
    loadPalette("sw_logo.pal");
    fetchImages(picturesVga, buffer, 210, 105, 12);
    loadPalette("primary.pal");
    fetchImages(picturesVga, buffer, 210, 105, 110);
    // narrow objects and enemies
    fetchImages(picturesVga, buffer, 140, 155, 270);
    // lower + wider objects and enemies
    fetchImages(picturesVga, buffer, 190, 110, 238);
    // floors and ceilings
    fetchImages(picturesVga, buffer, 224, 74, 28);
    // lower floors and ceilings
    fetchImages(picturesVga, buffer, 224, 62, 14);
    // paper dolls and paper doll effects
    fetchImages(picturesVga, buffer, 56, 136, 70);
    // portraits and portrait effects
    fetchImages(picturesVga, buffer, 32, 32, 180);
    // icons
    fetchImages(picturesVga, buffer, 16, 16, 340);
    // smaller icons
    fetchImages(picturesVga, buffer, 8, 8, 145);

    fclose(picturesVga);

    return 0;
}