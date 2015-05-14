#ifndef XENOTYPE_H
#define XENOTYPE_H

typedef int AMMO_TYPE;
// item defs are in file data/ammotype.def

typedef unsigned long PROPERTIES;

#define TYPE_NORMAL         0x00000001
#define TYPE_ARMOR_PERCING  0x00000002 // przebija armor/broni przed tym
#define TYPE_CONVERT_CPU    0x00000002 // -> dla reprogramatora
#define TYPE_EXPLOSIVE      0x00000004 // wybucha/antyodlamkowe
#define TYPE_WRITE_PROGRAM  0x00000004 // -> dla reprogramatora
#define TYPE_SMOKE          0x00000008 // dym
#define TYPE_RADIOACTIVE    0x00000010 // radioaktywne/chroni przed 
#define TYPE_CHEM_POISON    0x00000020 // trucizna/itd
#define TYPE_PARALYZE       0x00000040 // wszelkiego typu spowolnienia
#define TYPE_STUN           0x00000080 // ogluszanie
#define TYPE_POWER_SHIELD   0x00000100 // pocisk przebija tarcze/tarcza ochronna
#define TYPE_INCENDIARY     0x00000200 // plomien/ochrona przed ogniem
#define TYPE_SENSOR         0x00000400 // wykrywacz of movement
#define TYPE_INVISIBLE	    0x00000800 // item jest niewidoczny
#define TYPE_VAMPIRIC	    0x00001000 // tylko dla broni unarmed - polowe zadanych obrazen humanoidowi dodaje do HP
#define TYPE_ELECTRIC		0x00002000 // radioaktywne/chroni przed 
#define TYPE_ENERGY			0x00004000 // energy like plasma or laser
#define TYPE_LAST_UNUSED    0x00008000 // --- ostatni, nieuzywany, ale potrzebne

typedef int FIRE_TYPE;

#define FIRE_SINGLE 1
#define FIRE_DOUBLE 2
#define FIRE_TRIPLE 4
#define FIRE_BURST  8

#define PURPOSE_NONE 0
#define PURPOSE_AUTO_DOOR_OPEN 10
#define PURPOSE_WEAK_CORRIDOR_IN_MINES 11
#define PURPOSE_SMOKE_GENERATOR 12
#define PURPOSE_TUTORIAL_ADVANCE 13

#define PURPOSE_INCRASE_HP	100
#define PURPOSE_DECRASE_HP	101
#define PURPOSE_INCRASE_SPEED 	102
#define PURPOSE_DECRASE_SPEED	103
#define PURPOSE_INCRASE_STRENGTH	104
#define PURPOSE_DECRASE_STRENGTH	105
#define PURPOSE_INCRASE_ENDURANCE 106
#define PURPOSE_DECRASE_ENDURANCE 107
#define PURPOSE_INCRASE_INTELLIGENCE 108
#define PURPOSE_DECRASE_INTELLIGENCE 109
#define PURPOSE_INCRASE_METABOLISM 110
#define PURPOSE_DECRASE_METABOLISM 111
#define PURPOSE_INCRASE_FOV	112
#define PURPOSE_DECRASE_FOV	113

#define PURPOSE_INCRASE_RADIOACTIVE	150
#define PURPOSE_DECRASE_RADIOACTIVE	151
#define PURPOSE_INCRASE_CHEM_POISON	152
#define PURPOSE_DECRASE_CHEM_POISON	153

#define PURPOSE_REPAIR_HP		1000
#define PURPOSE_REPAIR_FIX_ARMOR	1001

#define PURPOSE_GENETIC_MACHINE_CARD	2000

#define PURPOSE_TEMP_MISSILE 3000



#define DANGER_NONE			0x00000000
#define DANGER_EXPLOSION	0x00000001
#define DANGER_GAS			0x00000002
#define DANGER_RADIOACTIVE	0x00000004
#define DANGER_CHEM_POISON	0x00000008
#define DANGER_FIRE			0x00000010
#define DANGER_COLD			0x00000020
#define DANGER_ENEMY 		0x00000040
#define DANGER_STUN			0x00000080
#define DANGER_PARALYSE     0x00000100
#define DANGER_SENSOR       0x00000200
#define DANGER_IGNITION     0x00000400
#define DANGER_GRENADE      0x00000800

#endif

