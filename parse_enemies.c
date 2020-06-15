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
    uint16_t unknown_0;
    uint16_t health;             // creature stats
    uint16_t unknown_1;
    uint16_t accuracy;           // creature stats
    uint16_t dexterity;          // creature stats
    uint16_t absorption;         // creature stats
    uint16_t damage;             // creature stats
    uint16_t attack_snd_index;   // .VOC file number
    uint16_t unknown_2;
    uint16_t projectile_snd_index; // .VOC file number - no ranged attack if 0x00
    uint16_t projectile_image;   // offset to projectile image
    uint16_t ranged_acc;         // creature stats
    uint16_t ranged_dam;         // creature stats
    uint16_t unknown_3;
    uint16_t unknown_4;
    uint16_t unknown_5;
    uint16_t special_attack1;    // first special attack flag
    uint16_t projectile_attack_type; // kill/effect type for projectile hit
    uint16_t unknown_6;
    uint16_t unknown_7;
    uint16_t unknown_8;
    uint16_t unknown_9;
    uint32_t UNUSED_0;           // always zero
    uint32_t gold;               // kill reward, decimal encoding - max 99,999,999
    uint32_t nuore;              // kill reward, decimal encoding - max 99,999,999
    uint32_t food_magic_ore;     // kill reward, decimal encoding - max 99,999,999
    uint32_t experience;         // kill reward, decimal encoding - max 99,999,999
    uint16_t UNUSED_1;           // always zero
    uint16_t unknown_10;
    uint8_t  animation_flags;    // upper 4 bits - idle animation type: 0 - none, 1 - ping-pong, 2 - restart; lower 4 bits: still unknown, looks like some sort of additional offset
    uint8_t  special_attack2;    // second special attack flag
    uint8_t  mobility;           // creature is immobile - like Fungus or Dwarf Towers (0x02) or mobile (other)
    uint8_t  translucency;       // determines if sprite is translucent (Ghost, Phase Titan) - 0x80 - on, 0x00 - off
    uint16_t immunity_bitmask;   // creature's immunities
    uint8_t  UNUSED_2;           // always zero
    uint8_t  resistance_bitmask; // creature's resistances
    uint16_t UNUSED_3;           // always zero
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
    POISON    = 1 << 15
};

enum Resistances {
    MAGIC    = 0x20,
    PHYSICAL = 0x80
};

const char *animation_types[] = { "none", "ping-pong", "restart" };

uint32_t decodeHex(uint32_t v)
{
    uint32_t b0 =       1 * ((v >> 24) & 0x0000000F) +       10 * (((v >> 24) & 0x000000F0) >> 4);
    uint32_t b1 =     100 * ((v >> 16) & 0x0000000F) +     1000 * (((v >> 16) & 0x000000F0) >> 4);
    uint32_t b2 =   10000 * ((v >>  8) & 0x0000000F) +   100000 * (((v >>  8) & 0x000000F0) >> 4);
    uint32_t b3 = 1000000 * ( v        & 0x0000000F) + 10000000 * (( v        & 0x000000F0) >> 4);
    
    return b0 + b1 + b2 + b3;
}

const char *attack1Name(uint16_t attack1)
{
    switch(attack1)
    {
        case 0x0F: return "Steal Gold";
        case 0x10: if (yendor_version == 2) return "Party Attack"; else return "Steal Food";  // "Steal Food" is never used
        case 0x11: return "Steal Nuore"; // never used
        case 0x18: return "Sick";
        case 0x19: return "Jinxing";
        case 0x1D: return "Disease";
        case 0x1E: return "Sick, Poison, Disease";
        case 0x24: return "Paralyze";
        case 0x25: return "Hexing";
        case 0x28: return "Frozen";
        case 0x29: return "Cursing";
        case 0x2C: return "Poison";
        default:   return "";
    }
}

const char *attack2Name(uint16_t attack2)
{
    switch(attack2)
    {
        case 0x02: return "Break Shield";         // never used
        case 0x04: return "Break Weapon";         // never used
        case 0x06: return "Break Weapon, Shield"; // never used
        case 0x08: return "Break Projectile";     // never used
        case 0xB0:
        case 0xD0: return "Party Attack";
        default:   return "";
    }
}

void printEnemy(Enemy *e)
{
    const char *attack1 = attack1Name(e->special_attack1);
    const char *attack2 = attack2Name(e->special_attack2);

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
    printf(" Special Attack: %s%s%s\n\n", attack2, strlen(attack2) > 0 && strlen(attack1) > 0 ? ", " : "", attack1);
    printf("  Animation: %s\n", animation_types[e->animation_flags >> 4]);
    printf("Translucent: %s\n", e->translucency == 0x80 ? "yes" : "no");
    printf("     Mobile: %s\n", e->mobility != 0x02 ? "yes" : "no");
    printf("    Unknown: %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X\n", e->unknown_0, e->unknown_1, e->unknown_2, e->unknown_3, e->unknown_4,
                                                                                    e->unknown_5, e->unknown_6, e->unknown_7, e->unknown_8, e->unknown_9, e->unknown_10);
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