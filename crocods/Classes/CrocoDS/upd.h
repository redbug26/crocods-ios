/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!WIN-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Chips!
// Définition du module        !CONFIG!=/V4!UPD 765!
/********************************************************* !NAME! **************
* !./V1!\!./V2!\!./V3!\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*  Fichier     :                       | Version : V0.1w
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Définitions pour le module UPD.C
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
*  18/02/2003              | L.DEPLANQUE             | Version 0.1w : prise en
*                          |                         | compte de l'information
*                          |                         | disc missing dans le
*                          |                         | registre ST0
* ------------------------------------------------------------------------------
********************************************************** !END! **************/


#ifndef UPD_H
#define UPD_H

#ifdef __cplusplus
extern "C" {
#endif

extern int DriveBusy;


int ReadUPD( int port );

void WriteUPD( int Port, int val );

void ResetUPD( void );

void SetDiskUPD( char * );

void EjectDiskUPD( void );

int GetCurrTrack( void );

void LireDiskMem(u8 *rom, u32 romsize, char *autofile);
    
#ifdef __cplusplus
    }
#endif

#endif
