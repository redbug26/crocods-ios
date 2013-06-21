/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// !CONFIG!=/L/* /R/* /W"* Nom : "
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!WIN-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Accessoires!
// Définition du module        !CONFIG!=/V4!Snapshots!
/******************************************************************************/

/********************************************************* !NAME! **************
* !./FLE!
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*          SYSTEME         |      SOUS SYSTEME       |      SOUS ENSEMBLE
* ------------------------------------------------------------------------------
*  EMULATEUR CPC           | WIN-CPC                 | Accessoires
* ------------------------------------------------------------------------------
*  Fichier     : SNAPSHOT.C            | Version : 0.1p
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Lecture/écriture des fichiers snapshots
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
*  13/11/2002              | L.DEPLANQUE             | Passage du nom de fichier
*                          |                         | snapshot en paramètre aux
*                          |                         | fonctions LireSnap() et
*                          |                         | SauveSnap().  
* ------------------------------------------------------------------------------
********************************************************** !END! **************/


#include  "nds.h"

#include  "types.h"
#include  "plateform.h"
#include  "config.h"
#include  "sound.h"
#include  "crtc.h"
#include  "ppi.h"
#include  "vga.h"
#include  "z80.h"
#ifdef USE_LOG
#include  "log.h"
#endif

#include "snapshot.h"



#ifdef USE_SNAPSHOT




/********************************************************* !NAME! **************
* Nom : StSnapShot
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Structures
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Structure en-tête fichier snapshot
*
********************************************************** !0! ****************/
#pragma pack(1)
typedef struct
    {
    u16    AF;
    u16    BC;
    u16    DE;
    u16    HL;
    u16    IR;
    UBYTE  IFF1;
    UBYTE  IFF2;
    u16    IX;
    u16    IY;
    u16    SP;
    u16    PC;
    UBYTE  InterruptMode;
    u16    _AF;
    u16    _BC;
    u16    _DE;
    u16    _HL;
} _SRegs;

typedef struct
    {
    char  Id[ 0x10 ]; // "MV - SNA'
    UBYTE Version;
    _SRegs Z80;
    UBYTE InkReg;
    UBYTE InkData[ 17 ];
    UBYTE VGARom;
    UBYTE VGARam;
    UBYTE CRTCIndex;
    UBYTE CRTCReg[ 18 ];
    UBYTE NumRom;
    UBYTE PPI[ 4 ];
    UBYTE PsgIndex;
    UBYTE PsgData[ 16 ];
    u8 ram_size[2];
    UBYTE Unused[ 0x93 ];
    } StSnapShot;
#pragma pack()


/********************************************************* !NAME! **************
* Nom : SnapShot
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Structure Snapshot
*
********************************************************** !0! ****************/
static StSnapShot SnapShot;


void SauveScreen(char *Nom)
{
#ifdef USE_FAT
/*
    FILE *fp;
    fp = fopen( "Nom", "w" );
    fwrite( MemCPC, 1, 80*50, fp );
    fclose( fp );

    fp = fopen( "Nom2", "w" );
    fwrite( MemCPC, 1, 0x20000, fp );
    fclose( fp );
*/
#endif
}

/********************************************************* !NAME! **************
* Nom : SauveSnap
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Sauvegarde d'un fichier snapshot
*
* Résultat    : /
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
void SauveSnap(char * Nom)
{
#ifdef USE_FAT
    int i;
    FILE * fp = fopen( Nom, "wb" );
    SRegs Z0;

    if ( fp )
        {
        strcpy( SnapShot.Id, "MV - SNA" );
        SnapShot.Version = 1;

            ReadZ80(&Z0);
            SnapShot.Z80.AF = Z0.AF.Word;
            SnapShot.Z80.BC = Z0.BC.Word;
            SnapShot.Z80.DE = Z0.DE.Word;
            SnapShot.Z80.HL = Z0.HL.Word;
            SnapShot.Z80.IR = Z0.IR.Word;
            SnapShot.Z80.IFF1 = Z0.IFF1;
            SnapShot.Z80.IFF2 = Z0.IFF2;
            SnapShot.Z80.IX = Z0.IX.Word;
            SnapShot.Z80.IY = Z0.IY.Word;
            SnapShot.Z80.SP = Z0.SP.Word;
            SnapShot.Z80.PC = Z0.PC.Word;
            SnapShot.Z80.InterruptMode = Z0.InterruptMode;
            SnapShot.Z80._AF = Z0._AF.Word;
            SnapShot.Z80._BC = Z0._BC.Word;
            SnapShot.Z80._DE = Z0._DE.Word;
            SnapShot.Z80._HL = Z0._HL.Word;

        SnapShot.InkReg = ( UBYTE )PenSelect;
        for ( i = 0; i < 16; i++ )
            SnapShot.InkData[ i ] = ( UBYTE )TabCoul[ i ];

        SnapShot.VGARom = ( UBYTE )DecodeurAdresse;
        SnapShot.VGARam = ( UBYTE )RamSelect;
        SnapShot.CRTCIndex = ( UBYTE )RegCRTCSel;
        for ( i = 0; i < 18; i++ )
            SnapShot.CRTCReg[ i ] = ( UBYTE )RegsCRTC[ i ];

        SnapShot.NumRom = ( UBYTE )RomExt;
        for ( i = 0; i < 4; i++ )
            SnapShot.PPI[ i ] = ( UBYTE )RegsPPI[ i ];

        SnapShot.PsgIndex = ( UBYTE )RegPSGSel;
        for ( i = 0; i < 16; i++ )
            SnapShot.PsgData[ i ] = 0; // ( UBYTE )RegsPSG[ i ]; // On ne sauve pas les registres PSG

        SnapShot.ram_size[0] = 128;
        SnapShot.ram_size[1] = 0;
        fwrite( &SnapShot, 1, sizeof(SnapShot), fp );
        fwrite( MemCPC, 1, 0x20000, fp );
        fclose( fp );
        }
#endif
}


/********************************************************* !NAME! **************
* Nom : LireSnap
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Lecture d'un fichier snapshot
*
* Résultat    : /
*
* Variables globales modifiées : Z80, PenSenect, RegPSGSel, RomExt, RegCRTCSel
*
********************************************************** !0! ****************/


/********************************************************* !NAME! **************
* Nom : LireSnap
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Lecture d'un fichier snapshot
*
* Résultat    : /
*
* Variables globales modifiées : Z80, PenSenect, RegPSGSel, RomExt, RegCRTCSel
*
********************************************************** !0! ****************/
void LireSnapshotMem(u8 *snap)
{
    int i;
    SRegs Z0;

        memcpy(&SnapShot, snap, sizeof( SnapShot ));
        if ( ! strncmp( SnapShot.Id, "MV - SNA", 8 ) ) {
            int dwSnapSize;
            dwSnapSize = SnapShot.ram_size[0] + (SnapShot.ram_size[1] * 256); // memory dump size
            dwSnapSize &= ~0x3f; // limit to multiples of 64

            memcpy(MemCPC, snap+sizeof(SnapShot), dwSnapSize * 1024);

            Z0.AF.Word = SnapShot.Z80.AF;
            Z0.BC.Word = SnapShot.Z80.BC;
            Z0.DE.Word = SnapShot.Z80.DE;
            Z0.HL.Word = SnapShot.Z80.HL;
            Z0.IR.Word = SnapShot.Z80.IR;
            Z0.IFF1 = SnapShot.Z80.IFF1;
            Z0.IFF2 = SnapShot.Z80.IFF2;
            Z0.IX.Word = SnapShot.Z80.IX;
            Z0.IY.Word = SnapShot.Z80.IY;
            Z0.SP.Word = SnapShot.Z80.SP;
            Z0.PC.Word = SnapShot.Z80.PC;
            Z0.InterruptMode = SnapShot.Z80.InterruptMode;
            Z0._AF.Word = SnapShot.Z80._AF;
            Z0._BC.Word = SnapShot.Z80._BC;
            Z0._DE.Word = SnapShot.Z80._DE;
            Z0._HL.Word = SnapShot.Z80._HL;
            WriteZ80(&Z0);
            

// PPI
            WritePPI( 0xF400, SnapShot.PPI[ 0 ] );
   //         WritePPI( 0xF500, SnapShot.PPI[ 1 ] );
            WritePPI( 0xF600, SnapShot.PPI[ 2 ] );
            WritePPI( 0xF700, SnapShot.PPI[ 3 ] );

            Reset8912();
            for ( i = 0; i < 16; i++ ) {
                Write8912( i, SnapShot.PsgData[ i ] );
            }

            RegPSGSel = SnapShot.PsgIndex;

// GA
            for (i=0;i<17;i++) {
                WriteVGA( i );
                WriteVGA( 0x40 | SnapShot.InkData[ i ] );
                }
            PenSelect = SnapShot.InkReg;


            RomExt = SnapShot.NumRom;
            WriteVGA( 0x80 | SnapShot.VGARom );
            WriteVGA( 0xC0 | SnapShot.VGARam );

// CRTC
            for ( i = 0; i < 18; i++ ) {
                WriteCRTC( 0xBC00, i );
                WriteCRTC( 0xBD00, SnapShot.CRTCReg[i] );
                }
            RegCRTCSel = SnapShot.CRTCIndex;

            UpdateInk=1;

//            printDebug("Snapshot Loaded\n");
            }
}

#endif
