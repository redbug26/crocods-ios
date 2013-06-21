/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// !CONFIG!=/L/* /R/* /W"* Nom : "
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!WIN-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Chips!
// Définition du module        !CONFIG!=/V4!UPD 765!
/******************************************************************************/

/********************************************************* !NAME! **************
* !./FLE!
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*          SYSTEME         |      SOUS SYSTEME       |      SOUS ENSEMBLE
* ------------------------------------------------------------------------------
*  EMULATEUR CPC           | WIN-CPC                 | Chips
* ------------------------------------------------------------------------------
*  Fichier     : UPD.C                 | Version : 0.1w
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Emulation du UPD 765
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
*  06/02/2003              | L.DEPLANQUE             | Version 0.1v : mémorise
*                          |                         | la longueur du fichier lu
*                          |                         | pour réécrire
*                          |                         | si nécessaire
* ------------------------------------------------------------------------------
*  18/02/2003              | L.DEPLANQUE             | Version 0.1w : prise en
*                          |                         | compte de l'information
*                          |                         | disc missing dans le
*                          |                         | registre ST0
* ------------------------------------------------------------------------------
********************************************************** !END! **************/


#include  "nds.h"

#include "types.h"
#include "plateform.h"

#include "upd.h"

//
// Constantes generales...
//
#define SECTSIZE   512
#define NBSECT     9


// Bits de Status
#define STATUS_CB       0x10
#define STATUS_NDM      0x20
#define STATUS_DIO      0x40
#define STATUS_RQM      0x80

// ST0
#define ST0_NR          0x08
#define ST0_SE          0x20
#define ST0_IC1         0x40
#define ST0_IC2         0x80

// ST1
#define ST1_ND          0x04
#define ST1_EN          0x80

// ST3
#define ST3_TS          0x08
#define ST3_T0          0x10
#define ST3_RY          0x20


/********************************************************* !NAME! **************
* Nom : ImgDsk
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : ImgDsk de stockage du fichier image disquette
*
********************************************************** !0! ****************/
//u8 *ImgDsk=NULL;
u8 ImgDsk[1024 * 1024]; // 1024 au lieu de 512 pour la lecture des D7 3.5


int LongFic;

/********************************************************* !NAME! **************
* Nom : CPCEMUEnt
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Structures
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : En-tête d'un fichier DSK
*
********************************************************** !0! ****************/
#pragma pack(1)
typedef struct
    {
    char  debut[ 0x30 ]; // "MV - CPCEMU Disk-File\r\nDisk-Info\r\n"
    UBYTE NbTracks;
    UBYTE NbHeads;
    SHORT DataSize; // 0x1300 = 256 + ( 512 * nbsecteurs )
    UBYTE Unused[ 0xCC ];
    } CPCEMUEnt;


/********************************************************* !NAME! **************
* Nom : CPCEMUSect
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Structures
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Description d'un secteur
*
********************************************************** !0! ****************/
typedef struct
    {
    UBYTE C;                // track
    UBYTE H;                // head
    UBYTE R;                // sect
    UBYTE N;                // size
    UBYTE ST1;              // Valeur ST1
    UBYTE ST2;              // Valeur ST2
    SHORT SectSize;         // Taille du secteur en octets
    } CPCEMUSect;


/********************************************************* !NAME! **************
* Nom : CPCEMUTrack
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Structures
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Description d'une piste
*
********************************************************** !0! ****************/
typedef struct
    {
    char        ID[ 0x10 ];   // "Track-Info\r\n"
    UBYTE       Track;
    UBYTE       Head;
    SHORT       Unused;
    UBYTE       SectSize; // 2
    UBYTE       NbSect;   // 9
    UBYTE       Gap3;     // 0x4E
    UBYTE       OctRemp;  // 0xE5
    CPCEMUSect  Sect[ 29 ];
    } CPCEMUTrack;
#pragma pack()


typedef int ( * pfctUPD )( int );


static int Rien( int );


/********************************************************* !NAME! **************
* Nom : fct
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Pointeur de fonction du UPD
*
********************************************************** !0! ****************/
static pfctUPD fct = Rien;


/********************************************************* !NAME! **************
* Nom : etat
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Compteur phase fonction UPD
*
********************************************************** !0! ****************/
static int etat;


/********************************************************* !NAME! **************
* Nom : CurrTrackDatasDSK
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Image d'une piste
*
********************************************************** !0! ****************/
static CPCEMUTrack CurrTrackDatasDSK;


/********************************************************* !NAME! **************
* Nom : Infos
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : En-tête du fichier image
*
********************************************************** !0! ****************/
static CPCEMUEnt Infos;


/********************************************************* !NAME! **************
* Nom : FlagWrite
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Indique si écriture sur disquette effectuée
*
********************************************************** !0! ****************/
static int FlagWrite;

/********************************************************* !NAME! **************
* Nom : Image
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Indique si fichier image chargé
*
********************************************************** !0! ****************/
static int Image;


/********************************************************* !NAME! **************
* Nom : PosData
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Position des données dans l'image
*
********************************************************** !0! ****************/
static int PosData;


/********************************************************* !NAME! **************
* Nom : NomFic
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Nom du fichier image ouvert
*
********************************************************** !0! ****************/

int DriveBusy;

/********************************************************* !NAME! **************
* Nom : Status
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Status UPD
*
********************************************************** !0! ****************/
static int Status;


/********************************************************* !NAME! **************
* Nom : ST0
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Status interne 0
*
********************************************************** !0! ****************/
static int ST0;


/********************************************************* !NAME! **************
* Nom : ST1
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Status interne 1
*
********************************************************** !0! ****************/
static int ST1;


/********************************************************* !NAME! **************
* Nom : ST2
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Status interne 2
*
********************************************************** !0! ****************/
static int ST2;


/********************************************************* !NAME! **************
* Nom : ST3
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Status interne 3
*
********************************************************** !0! ****************/
static int ST3;


/********************************************************* !NAME! **************
* Nom : C
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Cylindre (nø piste)
*
********************************************************** !0! ****************/
static int C;


/********************************************************* !NAME! **************
* Nom : H
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Head     (nø tête)
*
********************************************************** !0! ****************/
static int H;


/********************************************************* !NAME! **************
* Nom : R
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Record   (nø secteur)
*
********************************************************** !0! ****************/
static int R;


/********************************************************* !NAME! **************
* Nom : N
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Number   (nbre d'octet poids forts)
*
********************************************************** !0! ****************/
static int N;


/********************************************************* !NAME! **************
* Nom : Drive
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Drive    ( 0=A, 1=B)
*
********************************************************** !0! ****************/
static int Drive;


/********************************************************* !NAME! **************
* Nom : EOT
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Secteur final;
*
********************************************************** !0! ****************/
static int EOT;


/********************************************************* !NAME! **************
* Nom : Busy
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : UPD occupé
*
********************************************************** !0! ****************/
static int Busy;


/********************************************************* !NAME! **************
* Nom : Inter
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Génération d'une interruption UPD
*
********************************************************** !0! ****************/
static int Inter;


/********************************************************* !NAME! **************
* Nom : Moteur
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Etat moteur UPD
*
********************************************************** !0! ****************/
static int Moteur;


/********************************************************* !NAME! **************
* Nom : IndexSecteur
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Indique la position du secteur en cour sur la piste courante
*
********************************************************** !0! ****************/
static int IndexSecteur = 0;


/********************************************************* !NAME! **************
* Nom : RechercheSecteur
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Recherche un secteur sur la piste courrante
*
* Résultat    : L'index du secteur trouvé
*
* Variables globales modifiées : ST0, ST1
*
********************************************************** !0! ****************/
static int RechercheSecteur( int newSect, int * pos )
{
    int i;

    * pos = 0;
    for ( i = 0; i < CurrTrackDatasDSK.NbSect; i++ )
        if ( CurrTrackDatasDSK.Sect[ i ].R == newSect )
            return( ( UBYTE )i );
        else
            * pos += CurrTrackDatasDSK.Sect[ i ].SectSize;

    ST0 |= ST0_IC1;
    ST1 |= ST1_ND;
#ifdef USE_LOG
    sprintf( MsgLog, "secteur (C:%02X,H:%02X,R:%02X) non trouvé", C, H, R );
    Log( MsgLog, LOG_DEBUG );
#endif
    return( 0 );
}


/********************************************************* !NAME! **************
* Nom : ReadCHRN
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Renvoie les paramètres C, H, R, N de la piste courante
*
* Résultat    : /
*
* Variables globales modifiées : C, H, R, N, IndexSecteur
*
********************************************************** !0! ****************/
static void ReadCHRN( void )
{
    C = CurrTrackDatasDSK.Sect[ IndexSecteur ].C;
    H = CurrTrackDatasDSK.Sect[ IndexSecteur ].H;
    R = CurrTrackDatasDSK.Sect[ IndexSecteur ].R;
    N = CurrTrackDatasDSK.Sect[ IndexSecteur ].N;
    if ( ++IndexSecteur == CurrTrackDatasDSK.NbSect )
        IndexSecteur = 0;
}


/********************************************************* !NAME! **************
* Nom : SetST0
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Calcul des registres ST0, ST1, ST2
*
* Résultat    : /
*
* Variables globales modifiées : ST0, ST1, ST2
*
********************************************************** !0! ****************/
static void SetST0( void )
{
    ST0 = 0; // drive A
    if ( ! Moteur || Drive || ! Image )
        ST0 |= ST0_IC1 | ST0_NR;

    ST1 = 0;
    ST2 = 0;
}


/********************************************************* !NAME! **************
* Nom : Rien
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction non reconnue du UPD
*
* Résultat    : /
*
* Variables globales modifiées : etat, ST0
*
********************************************************** !0! ****************/
static int Rien( int val )
{
    Status &= ~STATUS_CB & ~STATUS_DIO;
    etat = 0;
    ST0 = ST0_IC2;
#ifdef USE_LOG
    Log( "Appel fonction FDC Rien", LOG_DEBUG );
#endif
    return( Status );
}


/********************************************************* !NAME! **************
* Nom : ReadST0
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD ReadST0
*
* Résultat    : ST0, C
*
* Variables globales modifiées : etat, ST0, ST1, Inter, Busy
*
********************************************************** !0! ****************/
static int ReadST0( int val )
{
    if ( ! Inter )
        ST0 = ST0_IC2;
    else
        {
        Inter = 0;
        if ( Busy )
            {
            ST0 = ST0_SE;
            Busy = 0;
            }
        else
            ST0 |= ST0_IC1 | ST0_IC2;
        }
    if ( Moteur && Image && ! Drive )
        ST0 &= ~ST0_NR;
    else
        {
        ST0 |= ST0_NR;
        if ( ! Image )
            ST0 |= ( ST0_IC1 | ST0_IC2 );
        }

    if ( etat++ == 1 )
        {
        Status |= STATUS_DIO;
        return( ST0 );
        }

    etat = val = 0;
    Status &= ~STATUS_CB & ~STATUS_DIO;
    ST0 &= ~ST0_IC1 & ~ST0_IC2;
    ST1 &= ~ST1_ND;
    return( C );
}


/********************************************************* !NAME! **************
* Nom : ReadST3
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD ReadST3
*
* Résultat    : 0, ST3
*
* Variables globales modifiées : etat, Status, ST3
*
********************************************************** !0! ****************/
static int ReadST3( int val )
{
    if ( etat++ == 1 )
        {
        Drive = val;
        Status |= STATUS_DIO;
        return( 0 );
        }
    etat = 0;
    Status &= ~STATUS_CB & ~STATUS_DIO;
    if ( Moteur && ! Drive )
        ST3 |= ST3_RY;
    else
        ST3 &= ~ST3_RY;

    return( ST3 );
}


/********************************************************* !NAME! **************
* Nom : Specify
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD Specify, non émulée
*
* Résultat    : 0
*
* Variables globales modifiées : etat, Status
*
********************************************************** !0! ****************/
static int Specify( int val )
{
    if ( etat++ == 1 )
        return( 0 );

    etat = val = 0;
    Status &= ~STATUS_CB & ~STATUS_DIO;
    return( 0 );
}


/********************************************************* !NAME! **************
* Nom : ReadID
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD ReadID
*
* Résultat    : ST0, ST1, ST2, C, H, R, N
*
* Variables globales modifiées : Drive, Status, Inter, etat
*
********************************************************** !0! ****************/
static int ReadID( int val )
{
    switch( etat++ )
        {
        case 1 :
            Drive = val;
            Status |= STATUS_DIO;
            Inter = 1;
            break;

        case 2 :
            return( 0 /*ST0*/ );

        case 3 :
            return( ST1 );

        case 4 :
            return( ST2 );

        case 5 :
            ReadCHRN();
            return( C );

        case 6 :
            return( H );

        case 7 :
            return( R );

        case 8 :
            etat = 0;
            Status &= ~STATUS_CB & ~STATUS_DIO;
            return( N );
        }
    return( 0 );
}



/********************************************************* !NAME! **************
* Nom : FormatTrack
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD FormatTrack, non émulée
*
* Résultat    : 0
*
* Variables globales modifiées : etat, Status
*
********************************************************** !0! ****************/
static int FormatTrack( int val )
{
#ifdef USE_LOG
    Log( "Appel fonction FDC format", LOG_DEBUG );
#endif
    etat = val = 0;
    Status &= ~STATUS_CB & ~STATUS_DIO;
    return( 0 );
}


/********************************************************* !NAME! **************
* Nom : Scan
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD Scan, non émulée
*
* Résultat    : 0
*
* Variables globales modifiées : etat
*
********************************************************** !0! ****************/
static int Scan( int val )
{
#ifdef USE_LOG
    Log( "Appel fonction FDC scan", LOG_DEBUG );
#endif
    etat = val = 0;
    return( 0 );
}


/********************************************************* !NAME! **************
* Nom : ChangeCurrTrack
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Changement de la piste courrante
*
* Résultat    : /
*
* Variables globales modifiées : C, H, R, N, ST3
*
********************************************************** !0! ****************/
static void ChangeCurrTrack( int newTrack )
{
    ULONG Pos = 0;
    int t, s;

    if ( ! Infos.DataSize )
        {
        memcpy( &CurrTrackDatasDSK, ImgDsk, sizeof( CurrTrackDatasDSK ) );
        for ( t = 0; t < newTrack; t++ )
            {
            for ( s = 0; s < CurrTrackDatasDSK.NbSect; s++ )
                Pos += CurrTrackDatasDSK.Sect[ s ].SectSize;

            Pos += sizeof( CurrTrackDatasDSK );
            memcpy( &CurrTrackDatasDSK, &ImgDsk[ Pos ], sizeof( CurrTrackDatasDSK ) );
            }
        }
    else
        Pos += Infos.DataSize * newTrack;

    memcpy( &CurrTrackDatasDSK, &ImgDsk[ Pos ], sizeof( CurrTrackDatasDSK ) );

    PosData = Pos + sizeof( CurrTrackDatasDSK );
    IndexSecteur = 0;
    ReadCHRN();

    if ( ! newTrack )
        ST3 |= ST3_T0;
    else
        ST3 &= ~ST3_T0;
}


/********************************************************* !NAME! **************
* Nom : MoveTrack
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD MoveTrack
*
* Résultat    : 0
*
* Variables globales modifiées : etat, Drive, Status, Busy, Inter
*
********************************************************** !0! ****************/
static int MoveTrack( int val )
{
    switch( etat++ )
        {
        case 1 :
            Drive = val;
            SetST0();            
            Status |= STATUS_NDM;
            break;

        case 2 :
            ChangeCurrTrack( C = val );
            etat = 0;
            Status &= ~STATUS_CB & ~STATUS_DIO & ~STATUS_NDM;
            Busy = 1;
            Inter = 1;
            break;
        }
    return( 0 );
}


/********************************************************* !NAME! **************
* Nom : MoveTrack0
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD MoveTrack0
*
* Résultat    : 0
*
* Variables globales modifiées : etat, C, Status, Busy, Inter
*
********************************************************** !0! ****************/
static int MoveTrack0( int val )
{
    Drive = val;
    ChangeCurrTrack( C = 0 );
    etat = 0;
    Status &= ~STATUS_CB & ~STATUS_DIO & ~STATUS_NDM;
    SetST0();
    Busy = 1;
    Inter = 1;
    return( 0 );
}


/********************************************************* !NAME! **************
* Nom : ReadData
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD ReadData
*
* Résultat    : Datas, ST0, ST1, ST2, C, H, R, N
*
* Variables globales modifiées : C, H, R, N, EOT, etat, Status
*
********************************************************** !0! ****************/
static int ReadData( int val )
{
    static int sect = 0, cntdata = 0, newPos;
    static signed int TailleSect;

    switch( etat++ )
        {
        case 1 :
            Drive = val;
            SetST0();            
            break;

        case 2 :
            C = val;
            break;

        case 3 :
            H = val;
            break;

        case 4 :
            R = val;
            break;

        case 5 :
            N = val;
            break;

        case 6 :
            EOT = val;
            break;

        case 7 :
            sect = RechercheSecteur( R, &newPos );
                if (sect!=-1) {
            TailleSect = 128 << CurrTrackDatasDSK.Sect[ sect ].N;
            if ( ! newPos )
                cntdata = ( sect * CurrTrackDatasDSK.SectSize ) << 8;
            else
                cntdata = newPos;
                }
            break;

        case 8 :
            Status |= STATUS_DIO | STATUS_NDM;
            break;

        case 9 :
           if ( ! ( ST0 & ST0_IC1 ) )
                {
                if ( --TailleSect )
                    etat--;
                else
                    {
                    if ( R++ < EOT )
                        etat = 7;
                    else
                        Status &= ~STATUS_NDM;
                    }
                return( ImgDsk[ PosData + cntdata++ ] );
                }
            Status &= ~STATUS_NDM;
            return( 0 );

        case 10 :
            return( ST0 );

        case 11 :
            return( ST1 | ST1_EN );       // ### ici PB suivant logiciels... ###

        case 12 :
            return( ST2 );

        case 13 :
            return( C );

        case 14 :
            return( H );

        case 15 :
            return( R );

        case 16 :
            etat = 0;
            Status &= ~STATUS_CB & ~STATUS_DIO;
            return( N );
        }
    return( 0 );
}


/********************************************************* !NAME! **************
* Nom : WriteData
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD WriteData
*
* Résultat    : ST0, ST1, ST2, C, H, R, N
*
* Variables globales modifiées : C, H, R, N, EOT, etat, Status
*
********************************************************** !0! ****************/
static int WriteData( int val )
{
    static int sect = 0, cntdata = 0, newPos;
    static signed int TailleSect;

    switch( etat++ )
        {
        case 1 :
            Drive = val;
            SetST0();            
            break;

        case 2 :
            C = val;
            break;

        case 3 :
            H = val;
            break;

        case 4 :
            R = val;
            break;

        case 5 :
            N = val;
            break;

        case 6 :
            EOT = val;
            break;

        case 7 :
            sect = RechercheSecteur( R, &newPos );
                if (sect!=-1) {

            TailleSect = 128 << CurrTrackDatasDSK.Sect[ sect ].N;
            if ( ! newPos )
                cntdata = ( sect * CurrTrackDatasDSK.SectSize ) << 8;
            else
                cntdata = newPos;
                }
            break;

        case 8 :
            Status |= STATUS_DIO | STATUS_NDM;
            break;

        case 9 :
            if ( ! ( ST0 & ST0_IC1 ) )
                {
                ImgDsk[ PosData + cntdata++ ] = ( UBYTE )val;
                if ( --TailleSect )
                    etat--;
                else
                    {
                    if ( R++ < EOT )
                        etat = 7;
                    else
                        Status &= ~STATUS_NDM;
                    }
                return( 0 );
                }
            Status &= ~STATUS_NDM;
            return( 0 );

        case 10 :
            if ( ! ( ST0 & ST0_IC1 ) )
                FlagWrite = 1;

            return( ST0 );

        case 11 :
            return( ST1 );

        case 12 :
            return( ST2 );

        case 13 :
            return( C );

        case 14 :
            return( H );

        case 15 :
            return( R );

        case 16 :
            etat = 0;
            Status &= ~STATUS_CB & ~STATUS_DIO;
            return( N );
        }
    return( 0 );
}


/********************************************************* !NAME! **************
* Nom : ReadUPD
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Lecture d'un registre/donnée du UPD
*
* Résultat    : Valeur registre/donnée du UPD
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
int ReadUPD( int port )
{
    if ( port & 1 )
        return( fct( port ) );

    return( Status );
}


/********************************************************* !NAME! **************
* Nom : WriteUPD
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Ecriture d'un registre/donnée du UPD
*
* Résultat    : /
*
* Variables globales modifiées : Status, etat, fct, Moteur
*
********************************************************** !0! ****************/
void WriteUPD( int port, int val )
{
    DriveBusy=1;
    
    if ( port == 0xFB7F )
        {
        if ( etat )
            fct( val );
        else
            {
            Status |= STATUS_CB;
            etat = 1;
            switch( val & 0x1F )
                {
                case 0x03 :
                    // Specify
                    fct = Specify;
                    break;

                case 0x04 :
                    // Lecture ST3
                    fct = ReadST3;
                    break;

                case 0x05 :
                    // Ecriture données
                    fct = WriteData;
                    break;

                case 0x06 :
                    // Lecture données
                    fct = ReadData;
                    break;

                case 0x07 :
                    // Déplacement tête piste 0
//                    Status |= STATUS_NDM;
                    fct = MoveTrack0;
                    break;

                case 0x08 :
                    // Lecture ST0, track
                    Status |= STATUS_DIO;
                    fct = ReadST0;
                    break;

                case 0x09 :
                    // Ecriture données
                    fct = WriteData;
                    break;

                case 0x0A :
                    // Lecture champ ID
                    fct = ReadID;
                    break;

                case 0x0C :
                    // Lecture données
                    fct = ReadData;
                    break;

                case 0x0D :
                    // Formattage piste
                    fct = FormatTrack;
                    break;

                case 0x0F :
                    // Déplacement tête
                    fct = MoveTrack;
                    break;

                case 0x11 :
                    fct = Scan;
                    break;

                default :
                    Status |= STATUS_DIO;
                    fct = Rien;
                }
            }
        }
    else
        if ( port == 0xFA7E )
            Moteur = val & 1;
}


/********************************************************* !NAME! **************
* Nom : ResetUPD
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Effectue un reset du UPD
*
* Résultat    : /
*
* Variables globales modifiées : Status, ST0, ST1, ST2, ST3, Busy, Inter, etat
*
********************************************************** !0! ****************/
void ResetUPD( void )
{
    Status = STATUS_RQM;
    ST0 = ST0_SE;
    ST1 = 0;
    ST2 = 0;
    ST3 = ST3_RY | ST3_TS;
    Busy = 0;
    Inter = 0;
    etat = 0;
}


/********************************************************* !NAME! **************
* Nom : EjectDiskUPD
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Termine l'accès à une disquette (ejection)
*
* Résultat    : /
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
void EjectDiskUPD( void )
{
    Image = 0;
            /*
    FILE * handle;

    if ( FlagWrite )
        {
            handle = fopen( NomFic, "wb" );
            if ( handle )
                {
                fwrite( &Infos, sizeof( Infos ), 1, handle );
                fwrite( ImgDsk, LongFic, 1, handle );
                fclose( handle );
                }
        FlagWrite = 0;
        }
*/
        /*
    if (ImgDsk!=NULL) {
        free(ImgDsk);
    }
    ImgDsk=NULL;
            */
}


/********************************************************* !NAME! **************
* Nom : SetDiskUPD
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Initialise l'accès à une disquette (insertion)
*
* Résultat    : /
*
* Variables globales modifiées : NomFic, Image, FlagWrite
*
********************************************************** !0! ****************/
void SetDiskUPD( char * n )
{
    /*
    FILE * handle;
    
    EjectDiskUPD();
    NomFic = n;
    handle = fopen( NomFic, "rb" );
    FAT_fseek(handle,0,SEEK_END);
    LongFic = ftell(handle) - sizeof(Infos);
    ImgDsk = (u8*)MyAlloc(dsksize, "UPD Disk");
    FAT_fseek(handle,0,SEEK_SET);
        
    if ( handle )
        {
        fread( &Infos, sizeof( Infos ), 1, handle );
        fread( ImgDsk, 1, LongFic, handle );
        fclose( handle );
        Image = 1;
        }
    FlagWrite = 0;
    ChangeCurrTrack( 0 );
    */
}


/********************************************************* !NAME! **************
* Nom : GetCurrTrack
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Retourne la piste courrante
*
* Résultat    : C
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
int GetCurrTrack( void )
{
    return( C );
}

void LireDiskMem(u8 *rom, u32 romsize, char *autofile)
{
	int j,pos;
	char usefile[256];
	char filename[256];

    // myprintf("EjectDiskUPD");
    EjectDiskUPD();

    LongFic=romsize-sizeof(Infos);
    
    // ImgDsk=(u8*)MyAlloc(LongFic, "Loading IMAGE");
        
    memcpy(&Infos, rom, sizeof(Infos));
    memcpy(ImgDsk, rom+sizeof(Infos), LongFic);
    Image=1;
    FlagWrite=0;

    // myprintf("ChangeCurrTrack");
    ChangeCurrTrack(0);  // Met a jour posdata
    // myprintf("Check autofile");
	if (autofile!=NULL) {
		usefile[0]=0;
		
		pos = PosData;

		
		while(1) {  
			char ext[4];
			u8 user;
			u8 unused[2];
	//		u8 extent, rec; 		

			
			user = ImgDsk[pos];
			if (user==0xE5) {
				break;
			}
			
			for (j=0;j<8;j++) {
				filename[j] = (ImgDsk[pos+j+1] & 0x7F);
				if (filename[j]==32) filename[j]=0;
			}   
			filename[8]=0; 
			
			for (j=0;j<3;j++) {
				ext[j]  = (ImgDsk[pos+j+9] & 0x7F);  
				if (ext[j]==32) ext[j]=0;
			}
			ext[3]=0;
			if ((ext[0]!=0) & (ext[0]!=32)) {
				strcat(filename, ".");
				strcat(filename,ext);
			}
			
		//	extent	= ImgDsk[pos+12];
			unused[0]	= ImgDsk[pos+13];
			unused[1]	= ImgDsk[pos+14];
		//	rec	= ImgDsk[pos+15]; 
			
			if ( (usefile[0]==0) || (!strcasecmp(ext,"bas")) ) {
				strcpy(usefile, filename);
			}
			
			// myprintf("%s %s\n", filename, usefile);
			pos+=32;
		}
		
		if (usefile[0]!=0) {
			strcpy(autofile, usefile);
		}
	}
}
