#if 0
mkdir voc_y2
mkdir voc_y3
gcc parse_sfx.c -O3 -std=c11 -o parse_sfx && ./parse_sfx $@
exit
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int yendor_version = 3;
int32_t sfxOffset = 0x0002D057;
int sfxCount = -1;

void fetchVoc(FILE *registerExe, FILE *worldDat)
{
    int32_t vocOffset = 0x00FFFFFF;
    uint16_t vocSize = 0;

    fseek(registerExe, sfxOffset, 0);
    
    // count sounds first
    do {
        int bytesRead = fread(&vocOffset, sizeof(int32_t), 1, registerExe);
        
        if (feof(registerExe))
        {
            printf("Reached EOF.\n");
            break;
        }

        sfxCount++;
    } while ((vocOffset & 0xFF000000) == 0);

    printf("Found %d sounds\n", sfxCount); 

    for(int i = 0; i < sfxCount; ++i)
    {
        fseek(registerExe, sfxOffset + i * sizeof(int32_t), 0);
        fread(&vocOffset, sizeof(int32_t), 1, registerExe);
        fseek(registerExe, sfxOffset + sfxCount * sizeof(int32_t) + i * sizeof(uint16_t), 0);
        fread(&vocSize, sizeof(uint16_t), 1, registerExe);
        
        printf("%X %X\n", vocOffset, vocSize);
        
        char filename[32];
        memset(filename, 0, sizeof(filename));
        sprintf(filename, "voc_y%d/%03d.voc", yendor_version, i);
        FILE *vocFile = fopen(filename, "wb");
        fseek(worldDat, vocOffset, 0);
        uint8_t *vocData = (uint8_t *)malloc(vocSize * sizeof(uint8_t));
        fread(vocData, sizeof(uint8_t), vocSize, worldDat);
        fwrite(vocData, sizeof(uint8_t), vocSize, vocFile);
        fclose(vocFile);
        free(vocData);
    }
}

int main(int argc, char **argv)
{
    unsigned char buffer[318 * 198];
    char register_exe[32] = { "REGISTER.EXE" };
    char world_dat[32] = { "WORLD.DAT" };
    
    for (int i = 1; i < argc; ++i)
    {        
        if (!strcmp(argv[i], "-f") && i < argc - 1)
        {
            memset(register_exe, 0, sizeof(register_exe));
            strncpy(register_exe, argv[i+1], sizeof(register_exe));
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
            printf("-y2  - use Yendorian Tales 2 format\n");
            printf("-y3  - use Yendorian Tales 3 format (default)\n");
            return 0;
        }
    }


    FILE *registerExe = fopen(register_exe, "rb");
    if (!registerExe)
    {
        printf("Could not load %s!\n", register_exe);
        return 1;
    }

    FILE *worldDat = fopen(world_dat, "rb");
    if (!worldDat)
    {
        printf("Could not load %s!\n", world_dat);
        fclose(registerExe);
        return 1;
    }   

    if (yendor_version == 2)
    {
        printf("Using Yendorian Tales 2 format.\n");
    }

    if (yendor_version == 3)
    {
        printf("Using Yendorian Tales 3 format.\n");
        fetchVoc(registerExe, worldDat);
    }

    fclose(registerExe);

    return 0;
}