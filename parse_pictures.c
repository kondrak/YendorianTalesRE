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

/*
 * Yendorian Tales 2 & 3 texture extraction: VGA bitmaps converted to PNGs (RGB or RGBA)
 * Game images are stored uncompressed in PICTURES.VGA as a stream of bytes referencing respective color index in the palette.
 * Palettes have been extracted using DosBox's screen capture feature.
 */

unsigned char PALETTE[256][3];
FILE *pictures_vga = NULL;
int pictures_vga_offset = 0;
int total_imgs = 0;
int img_scale = 1;
int rgba_export = 0;
int yendor_version = 3;

void loadPalette(const char *palName)
{
    FILE *palFile = fopen(palName, "rb");
    if (!palFile)
    {
        printf("Could not open palette file: %s\n", palName);
        fclose(pictures_vga);
        exit(1);
    }

    // palette file contains 256 triplets of RGB values
    fread(PALETTE, sizeof(unsigned char), 256 * 3, palFile);
    fclose(palFile);
}

void saveImage(int w, int h, int channels, unsigned char *buffer)
{
    unsigned char rgbData[w * h * channels];
    char filename[64];
    int index = 0;

    for (int i = 0; i < w * h; ++i)
    {
        rgbData[index++] = PALETTE[buffer[i]][0];
        rgbData[index++] = PALETTE[buffer[i]][1];
        rgbData[index++] = PALETTE[buffer[i]][2];
        // treat magenta as transparency color
        if (rgba_export)
            rgbData[index++] = PALETTE[buffer[i]][0] == 0xFF && PALETTE[buffer[i]][1] == 0x00 && PALETTE[buffer[i]][2] == 0xFF ? 0x00 : 0xFF;
    }

    memset(filename, 0, sizeof(filename));
    sprintf(filename, "pictures_y%d/%04d.png", yendor_version, total_imgs);
    printf("Extracting: %s\n", filename);

    if (img_scale > 1)
    {
        unsigned char *rgbDataScaled = (unsigned char *)malloc(img_scale * img_scale * w * h * channels);

        stbir_resize_uint8(rgbData, w, h, 0, rgbDataScaled, img_scale * w, img_scale * h, 0, channels);
        stbi_write_png(filename, img_scale * w, img_scale * h, channels, rgbDataScaled, img_scale * w * channels);

        free(rgbDataScaled);
    }
    else
    {
        stbi_write_png(filename, w, h, channels, rgbData, w * channels);
    }
}

void fetchImages(unsigned char *buffer, int w, int h, int c)
{
    int channels = rgba_export ? 4 : 3;
    for (int i = 0; i < c; ++i)
    {
        fseek(pictures_vga, pictures_vga_offset, 0);
        int bytesRead = fread(buffer, sizeof(unsigned char), w * h, pictures_vga);
        
        if (feof(pictures_vga))
        {
            printf("Reached EOF.\n");
            break;
        }

        saveImage(w, h, channels, buffer);

        pictures_vga_offset += bytesRead;
        total_imgs++;
    }
}

int main(int argc, char **argv)
{
    unsigned char buffer[318 * 198]; // maximum image size is 318x198
    char picturesVga[32] = { "PICTURES.VGA" };
    
    for (int i = 1; i < argc; ++i)
    {        
        if (!strcmp(argv[i], "-a"))
        {
            rgba_export = 1;
        }

        if (!strcmp(argv[i], "-s") && i < argc - 1)
        {
            if (isdigit(argv[i+1][0]))
                img_scale = atoi(argv[i+1]);
        }

        if (!strcmp(argv[i], "-f") && i < argc - 1)
        {
            memset(picturesVga, 0, sizeof(picturesVga));
            strncpy(picturesVga, argv[i+1], sizeof(picturesVga));
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

    pictures_vga = fopen(picturesVga, "rb");
    if (!picturesVga)
    {
        printf("Could not load %s!\n", picturesVga);
        return 1;
    }

    if (yendor_version == 2)
    {
        printf("Using Yendorian Tales 2 PICTURES.VGA format.\n");

        // fullscreen backgrounds
        loadPalette("palettes/sw_logo.pal");
        fetchImages(buffer, 318, 198, 1);
        loadPalette("palettes/primary.pal");
        fetchImages(buffer, 318, 198, 4);
        loadPalette("palettes/y2_intro.pal");
        fetchImages(buffer, 318, 198, 2);
        loadPalette("palettes/primary.pal");
        fetchImages(buffer, 318, 198, 4);
        loadPalette("palettes/y2_thaine.pal");
        fetchImages(buffer, 318, 198, 1);
        loadPalette("palettes/y2_cluebook.pal");
        fetchImages(buffer, 318, 198, 1);
        loadPalette("palettes/primary.pal");
        fetchImages(buffer, 318, 198, 2);
        loadPalette("palettes/order.pal");
        fetchImages(buffer, 318, 198, 1);
        loadPalette("palettes/webfoot.pal");
        fetchImages(buffer, 318, 198, 1);
        loadPalette("palettes/primary.pal");
        fetchImages(buffer, 318, 198, 3);
        PALETTE[255][1] = 0xFF;
        fetchImages(buffer, 318, 198, 1);
        PALETTE[255][1] = 0x00;
        fetchImages(buffer, 318, 198, 2);
        // scene objects and walls
        fetchImages(buffer, 210, 105, 73);
        loadPalette("palettes/y2_intro.pal");
        fetchImages(buffer, 210, 105, 6);
        loadPalette("palettes/primary.pal");
        fetchImages(buffer, 210, 105, 22);
        // narrow objects and enemies
        fetchImages(buffer, 140, 155, 10);
        loadPalette("palettes/y2_intro.pal");
        fetchImages(buffer, 140, 155, 6);
        loadPalette("palettes/primary.pal");
        fetchImages(buffer, 140, 155, 199);
        // lower + wider objects and enemies
        fetchImages(buffer, 190, 110, 162);
        // floors and ceilings
        fetchImages(buffer, 224, 74, 18);
        // lower floors and ceilings
        PALETTE[255][1] = 0xFF;
        fetchImages(buffer, 224, 62, 2);
        PALETTE[255][1] = 0x00;
        fetchImages(buffer, 224, 62, 10);
        // paper dolls and paper doll effects
        fetchImages(buffer, 56, 136, 23);
        loadPalette("palettes/y2_intro.pal");
        fetchImages(buffer, 56, 136, 3);
        loadPalette("palettes/primary.pal");
        fetchImages(buffer, 56, 136, 29);
        // portraits and portrait effects
        fetchImages(buffer, 32, 32, 270);
        // icons
        fetchImages(buffer, 16, 16, 510);
        // smaller icons
        fetchImages(buffer, 8, 8, 116);
    }

    if (yendor_version == 3)
    {
        printf("Using Yendorian Tales 3 PICTURES.VGA format.\n");

        // fullscreen backgrounds
        loadPalette("palettes/sw_logo.pal");
        fetchImages(buffer, 318, 198, 1);
        loadPalette("palettes/primary.pal");
        fetchImages(buffer, 318, 198, 4);
        loadPalette("palettes/y3_cluebook.pal");
        fetchImages(buffer, 318, 198, 1);
        loadPalette("palettes/y3_intro.pal");
        fetchImages(buffer, 318, 198, 6);
        loadPalette("palettes/y3_title.pal");
        fetchImages(buffer, 318, 198, 1);
        loadPalette("palettes/primary.pal");
        fetchImages(buffer, 318, 198, 2);
        loadPalette("palettes/order.pal");
        fetchImages(buffer, 318, 198, 1);
        loadPalette("palettes/webfoot.pal");
        fetchImages(buffer, 318, 198, 1);
        loadPalette("palettes/primary.pal");
        fetchImages(buffer, 318, 198, 6);
        // scene objects and walls
        fetchImages(buffer, 210, 105, 34);
        loadPalette("palettes/sw_logo.pal");
        fetchImages(buffer, 210, 105, 12);
        loadPalette("palettes/primary.pal");
        fetchImages(buffer, 210, 105, 110);
        // narrow objects and enemies
        fetchImages(buffer, 140, 155, 270);
        // lower + wider objects and enemies
        fetchImages(buffer, 190, 110, 238);
        // floors and ceilings
        fetchImages(buffer, 224, 74, 28);
        // lower floors and ceilings
        fetchImages(buffer, 224, 62, 14);
        // paper dolls and paper doll effects
        fetchImages(buffer, 56, 136, 70);
        // portraits and portrait effects
        fetchImages(buffer, 32, 32, 180);
        // icons
        fetchImages(buffer, 16, 16, 340);
        // smaller icons
        fetchImages(buffer, 8, 8, 145);
    }

    fclose(pictures_vga);

    return 0;
}
