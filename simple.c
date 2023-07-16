/*----------------------------------------
   SIMPLE.C -- Simple PM Window Demo Program
  ----------------------------------------*/

#define INCL_DOS                        /* Select part of header        */
#define INCL_WIN                        /* Select part of header        */
#define INCL_GPI                        /* Select part of header        */
#define INCL_DEV
#include <os2.h>                        /* PM header file               */
#include <stdio.h>                        /* PM header file               */
#include <string.h>                        /* PM header file               */
#include <stdlib.h>                        /* PM header file               */
#include "simple.h"

#define ID_WINDOW    255
#define MAXTXTLEN    255
/************************************************************************/
/* Function prototypes                                                  */
/************************************************************************/
MRESULT EXPENTRY MyWindowProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
void DrawText(HWND hwnd,HPS hps,char *pszText);

HAB   hab;                              /* PM anchor block handle       */
typedef struct _SIMPLETEXT {
  CHAR    Text[MAXTXTLEN+1];
  USHORT  CurLength;
} SIMPLETEXT;
typedef SIMPLETEXT * PSIMPLETEXT;
/**********************  Start of main procedure  ***********************/
int  main(  )
{
  HMQ  hmq;                             /* Message queue handle         */
  HWND hwndFrame;                       /* Frame window handle          */
  HWND hwndClient;                      /* Frame window handle          */
  QMSG qmsg;                            /* Message from message queue   */
  ULONG flCreate;                       /* Window creation control flags*/
  flCreate = FCF_SYSMENU | FCF_SIZEBORDER | FCF_TITLEBAR | FCF_MINMAX;
  hab = WinInitialize( 0 );          /* Initialize PM                */
  hmq = WinCreateMsgQueue( hab, 0 );    /* Create a message queue       */


  WinRegisterClass(                     /* Register window class        */
     hab,                               /* Anchor block handle          */
     (PCSZ) "MyWindow",                        /* Window class name            */
     MyWindowProc,                      /* Address of window procedure  */
     0L,                                /* No special class style       */
     4                                  /* 1  extra window double word  */
     );
  hwndFrame = WinCreateStdWindow(
               HWND_DESKTOP,            /* Desktop window is parent     */
               0L,                      /* No class style               */
               &flCreate,               /* Frame control flag           */
               (PCSZ) "MyWindow",              /* Client window class name     */
               (PCSZ) "Simple Sample",         /* No window text               */
               0,                       /* No special class style       */
               0,                       /* Resource is in .EXE file     */
               ID_WINDOW,               /* Frame window identifier      */
               &hwndClient              /* Client window handle         */
               );
  if (hwndFrame!=0) {
     WinSetWindowPos( hwndFrame,           /* Set the size and position of */
                   HWND_TOP,            /* the window before showing.   */
                   100, 50, 250, 200,
                   SWP_SIZE | SWP_MOVE | SWP_ACTIVATE | SWP_SHOW
                 );
     /************************************************************************/
     /* Get and dispatch messages from the application message queue         */
     /* until WinGetMsg returns FALSE, indicating a WM_QUIT message.         */
     /************************************************************************/
       if (hwndFrame!=(HWND)0) {
         while( WinGetMsg( hab, &qmsg, 0, 0, 0 ) )
            WinDispatchMsg( hab, &qmsg );
       } /* endif */
  }

  WinDestroyWindow( hwndFrame );        /* Tidy up...                   */
  WinDestroyMsgQueue( hmq );            /* and                          */
  WinTerminate( hab );                  /* terminate the application    */
}
/*********************  Start of window procedure  **********************/
HPS    hps;                            /* Presentation Space handle    */
MRESULT EXPENTRY MyWindowProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   PSIMPLETEXT pSimpleText;
   RECTL  clientrect;                     /* Rectangle coordinates        */
   CHAR ch;
   // CHAR Buffer[80];
   // POINTL pt;
   HPS hpsPaint;
  switch( msg )
  {
    case WM_CREATE:
      pSimpleText =(PSIMPLETEXT)malloc(sizeof(SIMPLETEXT));
      pSimpleText->CurLength=0;
      pSimpleText->Text[0]=0x00;
      WinSetWindowPtr(hwnd,0,pSimpleText);
      break;
    case WM_DESTROY:
      pSimpleText=WinQueryWindowPtr(hwnd,0);
      free(pSimpleText);
      break;
    case WM_PAINT:
      hpsPaint=WinBeginPaint( hwnd,0, &clientrect );
      pSimpleText =WinQueryWindowPtr(hwnd,0);
      if (pSimpleText) {
         DrawText(hwnd,hpsPaint,pSimpleText->Text);
      } /* endif */
      WinEndPaint( hpsPaint );                      /* Drawing is complete   */
      break;
    case WM_CHAR:
      /******************************************************************/
      /* Character input is processed here.                             */
      /* The first two bytes of message parameter 2 contain             */
      /* the character code.                                            */
      /******************************************************************/
      if( SHORT2FROMMP( mp2 ) == VK_F3 )    /* If the key pressed is F3,*/
        WinPostMsg( hwnd, WM_QUIT, 0L, 0L );/* post a quit message to   */
      if( SHORT1FROMMP( mp1 ) & KC_CHAR ) { /* If it is a valid character */
         pSimpleText =WinQueryWindowPtr(hwnd,0);
         if (pSimpleText->CurLength>=MAXTXTLEN) {
            pSimpleText->CurLength=0;
         } /* endif */
         ch=CHAR1FROMMP(mp2);
         pSimpleText->Text[pSimpleText->CurLength]=ch;
         pSimpleText->CurLength++;
         WinInvalidateRegion( hwnd, 0, FALSE ); /* Force redraw */
      }
      break;                                /* end the application.     */
   case WM_SETWINDOWPARAMS:
      pSimpleText =WinQueryWindowPtr(hwnd,0);
      switch (((PWNDPARAMS)mp1)->fsStatus) {
      case WPM_TEXT:
         //strncpy(pSimpleText->Text,((PWNDPARAMS)mp1)->pszText,MAXTXTLEN-1);
         strncpy(pSimpleText->Text,(const char * restrict) ((PWNDPARAMS)mp1)->pszText,MAXTXTLEN-1);
         pSimpleText->Text[MAXTXTLEN-1]=0x00;
         pSimpleText->CurLength=strlen(pSimpleText->Text);
         return (MRESULT) TRUE;
         break;
      default:
         return (MRESULT) FALSE;
      } /* endswitch */
      break;
    case WM_QUERYWINDOWPARAMS:
      pSimpleText =WinQueryWindowPtr(hwnd,0);
      if (WPM_CCHTEXT & (((PWNDPARAMS)mp1)->fsStatus)) {
         if (WPM_TEXT & (((PWNDPARAMS)mp1)->fsStatus)) {
            ((PWNDPARAMS)mp1)->cchText =
                 min(pSimpleText->CurLength,((PWNDPARAMS)mp1)->cchText);
         } else {
            ((PWNDPARAMS)mp1)->cchText=pSimpleText->CurLength;
            return (MPARAM)TRUE;
         }
      } /* endif */
      if (WPM_TEXT&(((PWNDPARAMS)mp1)->fsStatus)) {
         if (((PWNDPARAMS)mp1)->cchText>0) {
            memset(((PWNDPARAMS)mp1)->pszText,0x00,((PWNDPARAMS)mp1)->cchText);
            strncpy((char * restrict) ((PWNDPARAMS)mp1)->pszText,
                    pSimpleText->Text,
                    ((PWNDPARAMS)mp1)->cchText);
            return (MPARAM)TRUE;
         }
      }
      break;
    case WM_CLOSE:
      /******************************************************************/
      /* This is the place to put your termination routines             */
      /******************************************************************/
      WinPostMsg( hwnd, WM_QUIT, 0L, 0L );  /* Cause termination        */
      break;
    default:
      /******************************************************************/
      /* Everything else comes here.  This call MUST exist              */
      /* in your window procedure.                                      */
      /******************************************************************/

      return WinDefWindowProc( hwnd, msg, mp1, mp2 );
  }
  return FALSE;

}
void DrawText(HWND hwnd,HPS hps,char *pszText) {
 RECTL  rcl;             /* update region                        */
 LONG   cyCharHeight;     /* set character height                 */
 LONG   cchText;
 LONG   cchTotalDrawn;
 LONG   cchDrawn;
 FONTMETRICS  metrics;
 //CHAR Buffer[80];
 //POINTL pt;
 //ERRORID Error;

 //Error=WinGetLastError(hab);
 WinQueryWindowRect(hwnd, &rcl);         /* get window dimensions */

/* WinFillRect(hps,&rcl,CLR_BACKGROUND);*/
 GpiErase(hps);
 cchText =  (LONG)strlen(pszText);       /* get length of string  */
 //Error=WinGetLastError(hab);
 GpiQueryFontMetrics( hps, sizeof(metrics), &metrics);
 //Error=WinGetLastError(hab);
 cyCharHeight = metrics.lMaxAscender+metrics.lMaxDescender;  /* set character height  */

 /* until all chars drawn */
  for (cchTotalDrawn = 0;
       cchTotalDrawn <  cchText;
       rcl.yTop -= cyCharHeight)
     {
      if (rcl.yBottom>=rcl.yTop) break;
      /* draw the text */
      cchDrawn = WinDrawText(hps,      /* presentation-space handle */
          cchText -  cchTotalDrawn,    /* length of text to draw    */
          (PCCH) pszText +  cchTotalDrawn,    /* address of the text       */
          &rcl,                        /* rectangle to draw in      */
          0L,                          /* foreground color          */
          0L,                          /* background color          */
          DT_WORDBREAK | DT_TOP | DT_LEFT | DT_TEXTATTRS );
     if (cchDrawn) cchTotalDrawn +=  cchDrawn;
     else break;                      /* text could not be drawn   */
  }
#ifdef DEBUG
  pt.x=10;
  pt.y=10;
  sprintf(Buffer,"Height =%d, Total=%d, Ltext= %d Top=%d Error %p",
         cyCharHeight,cchTotalDrawn,cchText, rcl.yTop,Error);
  GpiCharStringAt( hps, &pt, (LONG)strlen(Buffer), Buffer );
#endif
}
