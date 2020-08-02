#if 0
gcc parse_spells.c -O3 -std=c11 -o parse_spells && ./parse_spells $@
exit
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * Yendorian Tales 2 & 3 spell info extraction.
 * Data is stored uncompressed in WORLD.DAT file.
 *
 * Yendorian Tales 2: spell data starts at offset 0x1AA65E
 * Yendorian Tales 3: spell data starts at offset 0x41B5BF
 */

int yendor_version = 3;

typedef struct Spell {
    char name[22];            // name of the spell
    uint16_t unknown_0;
    uint16_t mp;              // required magic points
    uint16_t nuore;           // required nuore
    uint16_t unknown_1;
    uint16_t unknown_2;
    uint16_t attack_sound_num;
    uint16_t heal_points;
    uint16_t unknown_5;
    uint16_t unknown_6;
    uint16_t unknown_7;
    uint16_t unknown_8;
    uint16_t unknown_9;
    uint16_t max_damage;
    uint16_t unknown_10;
    uint16_t unknown_11;
    uint16_t unknown_12;
    uint16_t unknown_13;
    uint16_t unknown_14;
    uint16_t unknown_15;
    uint16_t unknown_16;
    uint16_t unknown_17;
    uint16_t unknown_18;
    uint16_t unknown_19;
    uint16_t unknown_20;
    uint16_t unknown_21;
    uint16_t unknown_22;
    uint16_t unknown_23;
    uint16_t unknown_24;
    uint16_t unknown_25;
} Spell;

void printSpell(Spell *s)
{
    printf("%s\n", s->name);
    printf("-----------------------\n");
    printf("          MP: %d\n", s->mp);
    printf("       Nuore: %d\n", s->nuore);
    printf("Attack Sound: ");
    if (s->attack_sound_num) 
        printf("%03d.voc\n", s->attack_sound_num);
    else
        printf("-\n");
  /* printf("   Hit Sound: ");
    if (s->sound_num) 
        printf("%03d.voc\n", s->hit_sound_num);
    else
        printf("-\n");*/
    printf(" Heal Points: %d\n", s->heal_points);
    printf("  Max Damage: %d\n", s->max_damage);
    //printf("    Unknown: %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X\n", e->unknown_0, e->unknown_1, e->unknown_2, e->unknown_3, e->unknown_4,
    //                                                                                e->unknown_5, e->unknown_6, e->unknown_7, e->unknown_8, e->unknown_9, e->unknown_10);
    printf("\n");
}

void fetchSpells(FILE *worldDat, int32_t startOffset)
{
    Spell nextSpell;
    int spellCount = yendor_version == 3 ? 107 : 105;

    // extract data (80 bytes oer entry)
    for (int i = 0; i < spellCount; ++i)
    {
        fseek(worldDat, startOffset + i * 0x50, 0);
        fread(&nextSpell, 0x50, 1, worldDat);
        printSpell(&nextSpell);
    }
}


int main(int argc, char **argv)
{
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
        }

        if (!strcmp(argv[i], "-y3"))
        {
            yendor_version = 3;
        }

        if (!strcmp(argv[i], "-?"))
        {
            printf("Usage: %s <optional parameters>\n", argv[0]);
            printf("-f X - specify name of the WORLD.DAT file (default: WORLD.DAT)\n");
            printf("-y2  - extract Yendorian Tales 2 spells\n");
            printf("-y3  - extract Yendorian Tales 3 spells\n");
            return 0;
        }
    }

    FILE *worldDat = fopen(world_dat, "rb");
    if (!worldDat)
    {
        printf("Could not load %s!\n", world_dat);
        return 1;
    }

    if (yendor_version == 2)
    {
        printf("Extracting Yendorian Tales 2 spells.\n");
        fetchSpells(worldDat, 0x001AA65E);
    }

    if (yendor_version == 3)
    {
        printf("Extracting Yendorian Tales 3 spells.\n");
        fetchSpells(worldDat, 0x0041B5BF);
    }

    fclose(worldDat);

    return 0;
}
