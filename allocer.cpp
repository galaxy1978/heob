
//          Copyright Hannes Domani 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <direct.h>


#ifdef __GNUC__
__declspec(dllimport) void *operator new( size_t );
__declspec(dllimport) void operator delete( void* );
#endif
__declspec(dllimport) void *operator new[]( size_t );
__declspec(dllimport) void operator delete[]( void* );

extern "C" __declspec(dllimport) void *dll_alloc( size_t );


void choose( int arg )
{
  char *mem = (char*)malloc( 15 );
  mem[0] = 0;

  switch( arg )
  {
    case 1:
      {
        // memory leaks
        char *copy = strdup( "abcd" );
        char *zeroes = (char*)calloc( 2,500 );
        wchar_t *wcopy = (wchar_t*)wcsdup( L"efgh" );
        mem[1] = copy[0];
        mem[2] = zeroes[0];
        mem[3] = wcopy[0];
        char *newChars = new char[50];
        mem[4] = newChars[0];

        chdir( "\\" );
        char *cwd = _getcwd( NULL,0 );
        mem[5] = cwd[0];
        wchar_t *wcwd = _wgetcwd( NULL,0 );
        mem[6] = wcwd[0];
        cwd = _getdcwd( 0,NULL,0 );
        mem[7] = cwd[0];
        wcwd = _wgetdcwd( 0,NULL,0 );
        mem[8] = wcwd[0];
        char *fp = _fullpath( NULL,".",0 );
        mem[9] = fp[0];
        wchar_t *wfp = _wfullpath( NULL,L".",0 );
        mem[10] = wfp[0];
      }
      break;

    case 2:
      // access after allowed area
      mem[1] = mem[20];
      mem[25] = 5;
      break;

    case 3:
      // access before allowed area
      mem[1] = mem[-10];
      mem[-5] = 3;
      break;

    case 4:
      // allocation size alignment
      {
        int sum = 0;
        char t[100];
        int i;
        t[0] = 0;
        for( i=0; i<64; i++ )
        {
          char *tc = strdup( t );
          sum += strlen( tc );
          strcat( t,"x" );
        }
        mem[1] = sum;
      }
      break;

    case 5:
      // failed allocation
      {
#ifndef _WIN64
#define BIGNUM 2000000000
#else
#define BIGNUM 0x1000000000000000
#endif
        char *big = (char*)malloc( BIGNUM );
        mem[1] = big[0];
      }
      break;

    case 6:
      // reference pointer after being freed
      {
        char *tmp = (char*)malloc( 15 );
        char *tmp2;
        mem[1] = tmp[0];
        free( tmp );
        tmp2 = (char*)malloc( 15 );
        printf( "ptr1=0x%p; ptr2=0x%p -> %s\n",
            tmp,tmp2,tmp==tmp2?"same":"different" );
        fflush( stdout );
        mem[2] = tmp[1];
        mem[3] = tmp2[0];
        free( tmp2 );
      }
      break;

    case 7:
      // multiple free / free of invalid pointer
      free( mem );
      free( (void*)0x80000000 );
      break;

    case 8:
      // mismatch of allocation/release method
      printf( "%s",mem );
      delete mem;
      mem = new char;
      mem[0] = 0;

      printf( "%s",mem );
      delete[] mem;
      mem = new char[100];
      mem[0] = 0;
      break;

    case 9:
      // missing return address of strcmp()
      {
        mem[15] = 'a';
        char *s = strdup( "abc" );
        if( !strcmp(mem+15,s) )
          strcat( mem,"a" );
        free( s );
      }
      break;

    case 10:
      // leak in dll
      {
        char *leak = (char*)dll_alloc( 10 );
        mem[1] = leak[0];

#ifndef _WIN64
#define BITS "32"
#else
#define BITS "64"
#endif
        HMODULE mod = LoadLibrary( "dll-alloc-shared" BITS ".dll" );
        if( mod )
        {
          typedef void *dll_alloc_func( size_t );
          dll_alloc_func *func =
            (dll_alloc_func*)GetProcAddress( mod,"dll_alloc" );
          if( func )
          {
            leak = (char*)func( 20 );
            mem[2] = leak[0];
          }
          FreeLibrary( mod );
        }
      }
  }

  mem = (char*)realloc( mem,30 );
  if( mem ) printf( "%s",mem );
  free( mem );
}


int main( int argc,char **argv )
{
  int arg;
  printf( "allocer: main()\n" );
  fflush( stdout );

  arg = argc>1 ? atoi( argv[1] ) : 0;
  choose( arg );

  return( arg );
}
