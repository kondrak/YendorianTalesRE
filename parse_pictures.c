#if 0
mkdir pictures_y2
mkdir pictures_y3
gcc parse_pictures.c -O3 -std=c11 -o parse_pictures && ./parse_pictures $@
exit
#endif

// scaled images will be unfiltered to preserve the original feel
#define STBIR_DEFAULT_FILTER_UPSAMPLE STBIR_FILTER_BOX

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <ctype.h>

unsigned char PAL[256][3];
int picOffset = 0;
int picCount = 0;
int scale = 1;
int alpha = 0;
int yendor_version = 3;

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
    sprintf(filename, "pictures_y%d/%04d.png", yendor_version, picCount);
    printf("Extracting: %s\n", filename);

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
        int bytesRead = fread(buffer, sizeof(unsigned char), w * h, picturesVga);
        
        if (feof(picturesVga))
        {
            printf("Reached EOF.\n");
            break;
        }

        saveImage(w, h, channels, buffer);

        picOffset += bytesRead;
        picCount++;
    }
}

int main(int argc, char **argv)
{
    unsigned char buffer[318 * 198];
    char pictures_vga[32] = { "PICTURES.VGA" };
    
    for (int i = 1; i < argc; ++i)
    {        
        if (!strcmp(argv[i], "-a"))
        {
            alpha = 1;
        }

        if (!strcmp(argv[i], "-s") && i < argc - 1)
        {
            if (isdigit(argv[i+1][0]))
                scale = atoi(argv[i+1]);
        }

        if (!strcmp(argv[i], "-f") && i < argc - 1)
        {
            memset(pictures_vga, 0, sizeof(pictures_vga));
            strncpy(pictures_vga, argv[i+1], sizeof(pictures_vga));
        }

        if (!strcmp(argv[i], "-y2"))
        {
            yendor_version = 2;
        }

        if (!strcmp(argv[i], "-y3"))
        {
            yendor_version = 3;
        }

        if (!strcmp(argv[i], "-?"))
        {
            printf("Usage: %s <optional parameters>\n", argv[0]);
            printf("-a   - export images to RGBA PNGs (default: RGB with magenta set for color-key)\n");
            printf("-s X - export images scaled by an integer factor of X (default: unscaled)\n");
            printf("-f X - specify name of the PICTURES.VGA file (default: PICTURES.VGA)\n");
            printf("-y2  - use Yendorian Tales 2 format\n");
            printf("-y3  - use Yendorian Tales 3 format (default)\n");
            return 0;
        }
    }

    FILE *picturesVga = fopen(pictures_vga, "rb");
    if (!picturesVga)
    {
        printf("Could not load %s!\n", pictures_vga);
        return 1;
    }

    if (yendor_version == 2)
    {
        printf("Using Yendorian Tales 2 PICTURES.VGA format.\n");

        // fullscreen backgrounds
        loadPalette("palettes/sw_logo.pal");
        fetchImages(picturesVga, buffer, 318, 198, 1);
        loadPalette("palettes/primary.pal");
        fetchImages(picturesVga, buffer, 318, 198, 4);
        loadPalette("palettes/y2_intro.pal");
        fetchImages(picturesVga, buffer, 318, 198, 2);
        loadPalette("palettes/primary.pal");
        fetchImages(picturesVga, buffer, 318, 198, 5);
        loadPalette("palettes/y2_cluebook.pal");
        fetchImages(picturesVga, buffer, 318, 198, 1);
        loadPalette("palettes/primary.pal");
        fetchImages(picturesVga, buffer, 318, 198, 2);
        loadPalette("palettes/order.pal");
        fetchImages(picturesVga, buffer, 318, 198, 1);
        loadPalette("palettes/webfoot.pal");
        fetchImages(picturesVga, buffer, 318, 198, 1);
        loadPalette("palettes/primary.pal");
        fetchImages(picturesVga, buffer, 318, 198, 3);
        PAL[255][1] = 0xFF;
        fetchImages(picturesVga, buffer, 318, 198, 1);
        PAL[255][1] = 0x00;
        fetchImages(picturesVga, buffer, 318, 198, 2);
        // scene objects and walls
        fetchImages(picturesVga, buffer, 210, 105, 73);
        loadPalette("palettes/y2_intro.pal");
        fetchImages(picturesVga, buffer, 210, 105, 6);
        loadPalette("palettes/primary.pal");
        fetchImages(picturesVga, buffer, 210, 105, 22);
        // narrow objects and enemies
        fetchImages(picturesVga, buffer, 140, 155, 10);
        loadPalette("palettes/y2_intro.pal");
        fetchImages(picturesVga, buffer, 140, 155, 6);
        loadPalette("palettes/primary.pal");
        fetchImages(picturesVga, buffer, 140, 155, 199);
        // lower + wider objects and enemies
        fetchImages(picturesVga, buffer, 190, 110, 162);
        // floors and ceilings
        fetchImages(picturesVga, buffer, 224, 74, 18);
        // lower floors and ceilings
        PAL[255][1] = 0xFF;
        fetchImages(picturesVga, buffer, 224, 62, 2);
        PAL[255][1] = 0x00;
        fetchImages(picturesVga, buffer, 224, 62, 10);
        // paper dolls and paper doll effects
        fetchImages(picturesVga, buffer, 56, 136, 23);
        loadPalette("palettes/y2_intro.pal");
        fetchImages(picturesVga, buffer, 56, 136, 3);
        loadPalette("palettes/primary.pal");
        fetchImages(picturesVga, buffer, 56, 136, 29);
        // portraits and portrait effects
        fetchImages(picturesVga, buffer, 32, 32, 270);
        // icons
        fetchImages(picturesVga, buffer, 16, 16, 510);
        // smaller icons
        fetchImages(picturesVga, buffer, 8, 8, 116);
    }

    if (yendor_version == 3)
    {
        printf("Using Yendorian Tales 3 PICTURES.VGA format.\n");

        // fullscreen backgrounds
        loadPalette("palettes/sw_logo.pal");
        fetchImages(picturesVga, buffer, 318, 198, 1);
        loadPalette("palettes/primary.pal");
        fetchImages(picturesVga, buffer, 318, 198, 4);
        loadPalette("palettes/y3_cluebook.pal");
        fetchImages(picturesVga, buffer, 318, 198, 1);
        loadPalette("palettes/y3_intro.pal");
        fetchImages(picturesVga, buffer, 318, 198, 6);
        loadPalette("palettes/y3_title.pal");
        fetchImages(picturesVga, buffer, 318, 198, 1);
        loadPalette("palettes/primary.pal");
        fetchImages(picturesVga, buffer, 318, 198, 2);
        loadPalette("palettes/order.pal");
        fetchImages(picturesVga, buffer, 318, 198, 1);
        loadPalette("palettes/webfoot.pal");
        fetchImages(picturesVga, buffer, 318, 198, 1);
        loadPalette("palettes/primary.pal");
        fetchImages(picturesVga, buffer, 318, 198, 6);
        // scene objects and walls
        fetchImages(picturesVga, buffer, 210, 105, 34);
        loadPalette("palettes/sw_logo.pal");
        fetchImages(picturesVga, buffer, 210, 105, 12);
        loadPalette("palettes/primary.pal");
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
    }

    fclose(picturesVga);

    return 0;
}
