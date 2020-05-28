#if 0
gcc parse.c -std=c11 -o parser && ./parser
exit
#endif

#include <stdio.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

unsigned char PAL[256][3];
int picOffset = 0;
int picCount = 0;

void loadPalette(const char *palName)
{
    int i = 0;
    FILE *palFile = fopen(palName, "rb");
    if (!palFile)
    {
        printf("Could not open palette file: %s\n", palName);
        exit(1);
    }
    
    while(fread(PAL[i++], sizeof(unsigned char), 3, palFile) == 3);
    fclose(palFile);
}

void save_image(int w, int h, int channels_num, unsigned char *buffer)
{
    unsigned char rgbData[w * h * 3];

    int index = 0;
    for (int i = 0; i < w * h; ++i)
    {

        rgbData[index++] = PAL[buffer[i]][0];
        rgbData[index++] = PAL[buffer[i]][1];
        rgbData[index++] = PAL[buffer[i]][2];
    }

    char filename[64];
    memset(filename, 0, sizeof(filename));
    sprintf(filename, "output/%d.bmp", picCount);
    
    stbi_write_bmp(filename, w, h, channels_num, rgbData);
}

void fetchImages(FILE *picturesVga, unsigned char *buffer, int w, int h, int c)
{
    for (int i = 0; i < c; ++i)
    {
        fseek(picturesVga, picOffset, 0);
        int bytesRead = fread(buffer, sizeof(unsigned char), w * h, picturesVga);
        save_image(w, h, 3, buffer);
    
        picOffset += bytesRead;
        picCount++;
    }
}

int main(int argc, char **argv)
{
    unsigned char buffer[318 * 198];
    loadPalette("primary.pal");
    FILE *picturesVga = fopen("PICTURES.VGA", "rb");
    if (!picturesVga)
    {
        printf("Could not load PICTURES.VGA!\n");
        return 1;
    }

    // fullscreen backgrounds
    fetchImages(picturesVga, buffer, 318, 198, 23);
    // scene objects and walls
    fetchImages(picturesVga, buffer, 210, 105, 156);
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