/*
 * weapon_creator.h — porte fiel de code/utils/WeaponCreator.java (Quantum Engine J2ME)
 *
 * Cria uma Weapon a partir de /weapons.txt lendo o grupo [<index>].
 */
#ifndef QE_UTILS_WEAPON_CREATOR_H
#define QE_UTILS_WEAPON_CREATOR_H

typedef struct Weapon Weapon;

Weapon *WeaponCreator_createWeapon(int index);

#endif
