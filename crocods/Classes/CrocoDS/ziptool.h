/* ziptool.h internal header */
#ifndef _ZIPTOOL
#define _ZIPTOOL

#ifdef __cplusplus
extern "C" {
#endif 

u8 *unzip(u8 *zipbuf, u32 zipsize, char *filename, u32 *size);
    int compare( const void *arg1, const void *arg2 );
    
#ifdef __cplusplus
}
#endif 


#endif
