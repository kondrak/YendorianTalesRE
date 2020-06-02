#if 0
mkdir voc_y2
mkdir voc_y3
mkdir cmf_y2
mkdir cmf_y3
gcc parse_sfx.c -O3 -std=c11 -o parse_sfx && ./parse_sfx $@
exit
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int yendor_version = 3;

void fetchVoc(FILE *registerExe, FILE *worldDat, int32_t startOffset)
{
    int32_t vocOffset = 0x00FFFFFF;
    uint16_t vocSize = 0;
    int vocCount = -1;
    char filename[32];

    fseek(registerExe, startOffset, 0);
    
    // count sounds first
    do
    {
        fread(&vocOffset, sizeof(int32_t), 1, registerExe);
        
        if (feof(registerExe))
        {
            printf("Reached EOF.\n");
            break;
        }

        vocCount++;
    } while ((vocOffset & 0xFF000000) == 0);

    for (int i = 0; i < vocCount; ++i)
    {
        fseek(registerExe, startOffset + i * sizeof(int32_t), 0);
        fread(&vocOffset, sizeof(int32_t), 1, registerExe);
        fseek(registerExe, startOffset + vocCount * sizeof(int32_t) + i * sizeof(uint16_t), 0);
        fread(&vocSize, sizeof(uint16_t), 1, registerExe);

        memset(filename, 0, sizeof(filename));
        sprintf(filename, "voc_y%d/%03d.voc", yendor_version, i);

        fseek(worldDat, vocOffset, 0);
        uint8_t *vocData = (uint8_t *)malloc(vocSize * sizeof(uint8_t));    
        fread(vocData, sizeof(uint8_t), vocSize, worldDat);

        if ((vocData[3] | (vocData[2] << 8) | (vocData[1] << 16) | (vocData[0] << 24)) == 0x43726561)
        {
            printf("Extracting: %s\n", filename);
            FILE *vocFile = fopen(filename, "wb");
            fwrite(vocData, sizeof(uint8_t), vocSize, vocFile);
            fclose(vocFile);
        }
        free(vocData);
    }
}

void fetchCmf(FILE *registerExe, FILE *worldDat, int32_t startOffset)
{
    int32_t cmfOffset = 0x00FFFFFF;
    uint16_t cmfSize = 0;
    int cmfCount = -1;
    char filename[32];

    fseek(registerExe, startOffset, 0);
    
    // count music first
    do
    {
        fread(&cmfOffset, sizeof(int32_t), 1, registerExe);
        
        if (feof(registerExe))
        {
            printf("Reached EOF.\n");
            break;
        }

        cmfCount++;
    } while ((cmfOffset & 0xFF000000) == 0);

    for(int i = 0; i < cmfCount; ++i)
    {
        fseek(registerExe, startOffset + i * sizeof(int32_t), 0);
        fread(&cmfOffset, sizeof(int32_t), 1, registerExe);
        fseek(registerExe, startOffset + cmfCount * sizeof(int32_t) + i * sizeof(uint16_t), 0);
        fread(&cmfSize, sizeof(uint16_t), 1, registerExe);

        memset(filename, 0, sizeof(filename));
        sprintf(filename, "cmf_y%d/%03d.cmf", yendor_version, i);
        printf("Extracting: %s\n", filename);
        FILE *cmfFile = fopen(filename, "wb");
        fseek(worldDat, cmfOffset, 0);
        uint8_t *cmfData = (uint8_t *)malloc(cmfSize * sizeof(uint8_t));
        fread(cmfData, sizeof(uint8_t), cmfSize, worldDat);
        fwrite(cmfData, sizeof(uint8_t), cmfSize, cmfFile);
        fclose(cmfFile);
        free(cmfData);
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
            memset(world_dat, 0, sizeof(world_dat));
            strncpy(world_dat, argv[i+1], sizeof(world_dat));
        }

        if (!strcmp(argv[i], "-y2"))
        {
            yendor_version = 2;
            memset(register_exe, 0, sizeof(register_exe));
            strncpy(register_exe, "SWREG.EXE", sizeof(register_exe));
        }
        
        if (!strcmp(argv[i], "-y3"))
        {
            yendor_version = 3;
            memset(register_exe, 0, sizeof(register_exe));
            strncpy(register_exe, "REGISTER.EXE", sizeof(register_exe));
        }
        
        if (!strcmp(argv[i], "-?"))
        {
            printf("Usage: %s <optional parameters>\n", argv[0]);
            printf("-f X - specify name of the WORLD.DAT file (default: WORLD.DAT)\n");
            printf("-y2  - extract Yendorian Tales 2 data\n");
            printf("-y3  - extract Yendorian Tales 3 data (default)\n");
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
        printf("Extracting Yendorian Tales 2 data.\n");
        fetchVoc(registerExe, worldDat, 0x0002EBF1);
        fetchCmf(registerExe, worldDat, 0x0002EB73);
    }

    if (yendor_version == 3)
    {
        printf("Extracting Yendorian Tales 3 data.\n");
        fetchVoc(registerExe, worldDat, 0x0002D057);
        fetchCmf(registerExe, worldDat, 0x0002CFC7);
    }

    fclose(registerExe);

    return 0;
}