#if 0
gcc parse_enemies.c -O3 -std=c11 -o parse_enemies && ./parse_enemies $@
exit
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * Yendorian Tales 2 & 3 enemy info extraction.
 * Data is stored uncompressed in WORLD.DAT file.
 *
 * Yendorian Tales 2: enemy data starts at offset 0x1A7994
 * Yendorian Tales 3: enemy data starts at offset 0x4170DF
 */

int yendor_version = 3;

typedef struct Enemy {
    char name1[13];              // creature name: first part
    char name2[13];              // creature name: second part (optional)
    uint16_t start_frame_offset; // presumably offset to first idle frame in PICTURES.VGA
    uint16_t unknown0;
    uint16_t health;             // creature stats
    uint16_t unknown1;
    uint16_t accuracy;           // creature stats
    uint16_t dexterity;          // creature stats
    uint16_t absorption;         // creature stats
    uint16_t damage;             // creature stats
    uint16_t attack_snd_index;   // .VOC file number
    uint16_t unknown2;
    uint16_t projectile_snd_index; // .VOC file number - no ranged attack if 0x00
    uint16_t projectile_image;   // offset to projectile image
    uint16_t ranged_acc;         // creature stats
    uint16_t ranged_dam;         // creature stats
    uint16_t unknown3;
    uint16_t unknown4;
    uint16_t unknown5;
    uint16_t special_attack1;    // 0x0F - Steal Gold, 0x10 - Steal Food, 0x11 - Steal Nuore, 0x15 - Poison, 0x18 - Sick, 0x19 - Jinxing, 0x1D - Disease, 0x1E - Sick+Poison+Disease, 0x24 - Paralyze, 0x25 - Hexing, 0x27 - Stoning, 0x28 - Frozen, 0x29 - Cursing
    uint16_t projectile_attack_type; // kill/effect type for projectile hit
    uint16_t unknown6;
    uint16_t unknown7;
    uint16_t unknown8;
    uint16_t unknown9;
    uint16_t unknown10;
    uint16_t unknown11;
    uint32_t gold;               // kill reward, decimal encoding - max 99,999,999
    uint32_t nuore;              // kill reward, decimal encoding - max 99,999,999
    uint32_t food_magic_ore;     // kill reward, decimal encoding - max 99,999,999
    uint32_t experience;         // kill reward, decimal encoding - max 99,999,999
    uint16_t unknown12;
    uint16_t unknown13;
    uint8_t  animation_flags;    // upper 4 bits - idle animation type: 0 - none, 1 - ping-pong, 2 - restart; lower 4 bits: still unknown, looks like some sort of additional offset
    uint8_t  special_attack2;    // 0x10 - Party Attack, 0x12 - Break Shield, 0x14 - Break Weapon, 0x16 - Break Weapon+Shield, 0x18 - Break Projectile - needs more investigation
    uint8_t  mobility;           // creature is mobile (0x00) or immobile - like Fungus or Dwarf Towers (0x02)
    uint8_t  translucency;       // determines if sprite is translucent (Ghost, Phase Titan) - 0x80 - on, 0x00 - off
    uint16_t immunity_bitmask;   // creature's immunities
    uint8_t  unknown14;
    uint8_t  resistance_bitmask; // creature's resistances
    uint16_t unknown15;
} Enemy;

enum Immunities {
    POWER    = 1 << 0,
    ELECTRIC = 1 << 1,
    COLD     = 1 << 2,
    FIRE     = 1 << 3,
    MAGIC_RESISTANCE = 1 << 4,
    CURSING   = 1 << 10,
    HEXING    = 1 << 11,
    FREEZING  = 1 << 12,
    PARALYSIS = 1 << 13,
    DISEASE   = 1 << 14,
    POISON   = 1 << 15
};

enum Resistances {
    MAGIC    = 0x20,
    PHYSICAL = 0x80
};

uint32_t decodeHex(uint32_t v)
{
    uint32_t b0 =       1 * ((v >> 24) & 0x0000000F) +       10 * (((v >> 24) & 0x000000F0) >> 4);
    uint32_t b1 =     100 * ((v >> 16) & 0x0000000F) +     1000 * (((v >> 16) & 0x000000F0) >> 4);
    uint32_t b2 =   10000 * ((v >>  8) & 0x0000000F) +   100000 * (((v >>  8) & 0x000000F0) >> 4);
    uint32_t b3 = 1000000 * ( v        & 0x0000000F) + 10000000 * (( v        & 0x000000F0) >> 4);
    
    return b0 + b1 + b2 + b3;
}

void printEnemy(Enemy *e)
{
    printf("%s%s\n", e->name1, e->name2);
    printf("-----------------------\n");
    printf("     Experience: %u\n", decodeHex(e->experience));
    printf("           Gold: %d\n", decodeHex(e->gold));
    printf(" Food/Magic Ore: %d\n", decodeHex(e->food_magic_ore));
    printf("          Nuore: %d\n", decodeHex(e->nuore));
    printf("\n");
    printf("         Health: %d\n", e->health);
    printf("       Accuracy: %d\n", e->accuracy);
    printf("      Dexterity: %d\n", e->dexterity);
    printf("     Absorption: %d\n", e->absorption);
    printf("         Damage: %d\n", e->damage);
    printf("    Ranged acc.: %d\n", e->ranged_acc);
    printf("    Ranged dam.: %d\n", e->ranged_dam);
    printf("\n");
    printf("         Poison: %s\n", e->immunity_bitmask & POISON ? "Immune" : "");
    printf("        Disease: %s\n", e->immunity_bitmask & DISEASE ? "Immune" : "");
    printf("      Paralysis: %s\n", e->immunity_bitmask & PARALYSIS ? "Immune" : "");
    printf("       Freezing: %s\n", e->immunity_bitmask & FREEZING ? "Immune" : "");
    printf("         Hexing: %s\n", e->immunity_bitmask & HEXING ? "Immune" : "");
    printf("        Cursing: %s\n", e->immunity_bitmask & CURSING ? "Immune" : "");
    printf("           Fire: %s\n", e->immunity_bitmask & FIRE ? "Immune" : "");
    printf("           Cold: %s\n", e->immunity_bitmask & COLD ? "Immune" : "");
    printf("       Electric: %s\n", e->immunity_bitmask & ELECTRIC ? "Immune" : "");
    printf("          Power: %s\n", e->immunity_bitmask & POWER ? "Immune" : "");
    printf("   Magic Damage: %s\n", e->immunity_bitmask & MAGIC_RESISTANCE || e->resistance_bitmask & MAGIC ? "Resistant" : "");
    printf("Physical Damage: %s\n", e->resistance_bitmask & PHYSICAL ? "Resistant" : "");
    printf(" Special Attack:\n");
    printf("\n");
}

void fetchEnemies(FILE *worldDat, int32_t startOffset)
{
    Enemy nextEnemy;
    int enemyCount = yendor_version == 3 ? 72 : 61;

    // extract data (106 bytes oer entry)
    for (int i = 0; i < enemyCount; ++i)
    {
        fseek(worldDat, startOffset + i * 0x6A, 0);
        fread(&nextEnemy, 0x6A, 1, worldDat);
        printEnemy(&nextEnemy);
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
            printf("-y2  - extract Yendorian Tales 2 enemies\n");
            printf("-y3  - extract Yendorian Tales 3 enemies\n");
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
        printf("Extracting Yendorian Tales 2 enemies.\n");
        fetchEnemies(worldDat, 0x001A7994);
    }

    if (yendor_version == 3)
    {
        printf("Extracting Yendorian Tales 3 enemies.\n");
        fetchEnemies(worldDat, 0x004170DF);
    }

    fclose(worldDat);

    return 0;
}