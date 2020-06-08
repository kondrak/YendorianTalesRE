#if 0
mkdir voc_y2
mkdir voc_y3
mkdir cmf_y2
mkdir cmf_y3
gcc parse_audio.c -O3 -std=c11 -o parse_audio && ./parse_audio $@
exit
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * Yendorian Tales 2 & 3 audio extraction: digitized sounds (.VOC) and music (.CMF)
 * both data formats are stored uncompressed in WORLD.DAT file, offsts and file sized are kept in game executables.
 *
 * Yendorian Tales 2: digitized audio data starting at offset 0x2EBF1 followed by file sizes, music data starting at offset 0x2EB73 followed by file sizes
 * Yendorian Tales 3: digitized audio data starting at offset 0x2D057 followed by file sizes, music data starting at offset 0x2CFC7 followed by file sizes
 */

int yendor_version = 3;

void fetchAudio(FILE *executable, FILE *worldDat, const char *extension, int32_t startOffset, uint32_t headerSignature)
{
    int32_t fileOffset = 0x00FFFFFF;
    uint16_t fileSize = 0;
    int filesWritten = 0;
    int fileCount = -1;
    char fileName[32];

    fseek(executable, startOffset, 0);

    // count files first
    do
    {
        fread(&fileOffset, sizeof(int32_t), 1, executable);
        
        if (feof(executable))
        {
            printf("Reached EOF.\n");
            break;
        }

        fileCount++;
    } while ((fileOffset & 0xFF000000) == 0);

    // extract data
    for (int i = 0; i < fileCount; ++i)
    {
        fseek(executable, startOffset + i * sizeof(int32_t), 0);
        fread(&fileOffset, sizeof(int32_t), 1, executable);
        fseek(executable, startOffset + fileCount * sizeof(int32_t) + i * sizeof(uint16_t), 0);
        fread(&fileSize, sizeof(uint16_t), 1, executable);
        fseek(worldDat, fileOffset, 0);

        uint8_t *data = (uint8_t *)malloc(fileSize * sizeof(uint8_t));    
        fread(data, sizeof(uint8_t), fileSize, worldDat);

        // verify header
        if ((data[3] | (data[2] << 8) | (data[1] << 16) | (data[0] << 24)) == headerSignature)
        {
            memset(fileName, 0, sizeof(fileName));
            sprintf(fileName, "%s_y%d/%03d.%s", extension, yendor_version, i, extension);
            printf("Extracting: %s\n", fileName);

            FILE *vocFile = fopen(fileName, "wb");
            fwrite(data, sizeof(uint8_t), fileSize, vocFile);
            fclose(vocFile);

            filesWritten++;
        }

        free(data);
    }

    printf("Successfuly written %d .%s files.\n", filesWritten, extension);
}


int main(int argc, char **argv)
{
    char game_exe[32] = { "REGISTER.EXE" };
    char world_dat[32] = { "WORLD.DAT" };

    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-f") && i < argc - 1)
        {
            memset(world_dat, 0, sizeof(world_dat));
            strncpy(world_dat, argv[i+1], sizeof(world_dat));
        }

        if (!strcmp(argv[i], "-y2"))
        {
            yendor_version = 2;
            memset(game_exe, 0, sizeof(game_exe));
            strncpy(game_exe, "SWREG.EXE", sizeof(game_exe));
        }

        if (!strcmp(argv[i], "-y3"))
        {
            yendor_version = 3;
            memset(game_exe, 0, sizeof(game_exe));
            strncpy(game_exe, "REGISTER.EXE", sizeof(game_exe));
        }

        if (!strcmp(argv[i], "-?"))
        {
            printf("Usage: %s <optional parameters>\n", argv[0]);
            printf("-f X - specify name of the WORLD.DAT file (default: WORLD.DAT)\n");
            printf("-y2  - extract Yendorian Tales 2 data (requires original SWREG.EXE file)\n");
            printf("-y3  - extract Yendorian Tales 3 data (default, requires original REGISTER.EXE file)\n");
            return 0;
        }
    }

    FILE *executable = fopen(game_exe, "rb");
    if (!executable)
    {
        printf("Could not load %s!\n", game_exe);
        return 1;
    }

    FILE *worldDat = fopen(world_dat, "rb");
    if (!worldDat)
    {
        printf("Could not load %s!\n", world_dat);
        fclose(executable);
        return 1;
    }

    if (yendor_version == 2)
    {
        printf("Extracting Yendorian Tales 2 audio.\n");
        fetchAudio(executable, worldDat, "voc", 0x0002EBF1, 0x43726561);
        fetchAudio(executable, worldDat, "cmf", 0x0002EB73, 0x43544D46);
    }

    if (yendor_version == 3)
    {
        printf("Extracting Yendorian Tales 3 audio.\n");
        fetchAudio(executable, worldDat, "voc", 0x0002D057, 0x43726561);
        fetchAudio(executable, worldDat, "cmf", 0x0002CFC7, 0x43544D46);
    }

    fclose(executable);
    fclose(worldDat);

    return 0;
}