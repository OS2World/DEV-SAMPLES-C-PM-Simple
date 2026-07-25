/*********************************************************************
 *
 *  simple.c - Simple PM Window Demo Program
 *
 *  A minimal OS/2 Presentation Manager (PM) application that creates
 *  a standard frame window and demonstrates fundamental PM programming
 *  concepts: window class registration, message-loop processing,
 *  keyboard input, and owner-drawn client-area text rendering.
 *
 *  The program registers a custom window class ("MyWindow") that
 *  allocates per-instance storage for a text buffer.  Characters
 *  typed by the user are appended to the buffer and drawn in the
 *  client area with automatic word-wrapping.  Pressing F3 exits.
 *
 *  Build:
 *    GCC (bitwiseworks):   make -f makefile-gcc
 *    OpenWatcom 2.0:       wmake -f makefile-ow
 *
 *  Authors:
 *    Original Author
 *    Martin Iturbide / OS2World (2023 - ArcaOS/GCC port)
 *
 *  License: BSD 3-Clause — see LICENSE file.
 *
 *  Version history:
 *    1.0    - Original version
 *    1.01   - 2023-07-16  Ported to ArcaOS 5.0.7 with GCC 9.2
 *    1.03   - 2026-07-24  Reorganised build system, added OpenWatcom
 *                         support, improved documentation
 *
 *********************************************************************/

#define INCL_DOS                        /* OS/2 base API                */
#define INCL_WIN                        /* Window Manager API           */
#define INCL_GPI                        /* Graphics Programming API     */
#define INCL_DEV
#include <os2.h>                        /* PM master header file        */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "simple.h"

#define MAXTXTLEN    255                /* Max text buffer length       */

/************************************************************************/
/* Function prototypes                                                  */
/************************************************************************/

/*
 * MyWindowProc - Client window procedure.
 *
 * Handles all messages for the client area of the main application
 * window.  Manages a per-instance SIMPLETEXT buffer that accumulates
 * characters typed by the user and repaints them using DrawText().
 */
MRESULT EXPENTRY MyWindowProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );

/*
 * DrawText - Render text into the client area.
 *
 * Uses WinDrawText with DT_WORDBREAK to wrap the accumulated text
 * across multiple lines within the client rectangle.  Called from
 * the WM_PAINT handler.
 */
void DrawText(HWND hwnd,HPS hps,char *pszText);

/* ------------------------------------------------------------------ */
/* Global anchor-block handle.  Required by several Win* API calls.   */
/* ------------------------------------------------------------------ */
HAB   hab;

/* ------------------------------------------------------------------ */
/* Per-instance window data: a simple text buffer that grows as the   */
/* user types characters.  Allocated in WM_CREATE, freed in           */
/* WM_DESTROY, and stored via WinSetWindowPtr / WinQueryWindowPtr.    */
/* ------------------------------------------------------------------ */
typedef struct _SIMPLETEXT {
  CHAR    Text[MAXTXTLEN+1];           /* Character buffer             */
  USHORT  CurLength;                   /* Current number of chars      */
} SIMPLETEXT;
typedef SIMPLETEXT * PSIMPLETEXT;

/**********************  Start of main procedure  ***********************/
/*
 * main - Application entry point.
 *
 * Initialises PM (WinInitialize), creates a message queue, registers
 * the "MyWindow" window class, creates a standard frame + client
 * window pair, and enters the message loop.  On exit the window and
 * message queue are destroyed and PM is terminated.
 */
int  main(  )
{
  HMQ  hmq;                             /* Message queue handle         */
  HWND hwndFrame;                       /* Frame window handle          */
  HWND hwndClient;                      /* Client window handle         */
  QMSG qmsg;                            /* Message from message queue   */
  ULONG flCreate;                       /* Window creation control flags*/

  /*
   * FCF_SYSMENU    - system menu (top-left icon)
   * FCF_SIZEBORDER - thick sizing border
   * FCF_TITLEBAR   - title bar
   * FCF_MINMAX     - minimise / maximise buttons
   * FCF_ICON       - load icon resource matching the frame window ID
   */
  flCreate = FCF_SYSMENU | FCF_SIZEBORDER | FCF_TITLEBAR | FCF_MINMAX | FCF_ICON;

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
  return 0;
}

/*********************  Start of window procedure  **********************/

HPS    hps;                            /* Presentation Space handle    */

/*
 * MyWindowProc - Client window procedure.
 *
 * Message handlers:
 *
 *   WM_CREATE   - Allocates a SIMPLETEXT buffer and stores the pointer
 *                 in the extra window words (index 0).
 *
 *   WM_DESTROY  - Frees the SIMPLETEXT buffer.
 *
 *   WM_PAINT    - Obtains a cached-micro PS, retrieves the text buffer,
 *                 and calls DrawText() to render it with word-wrap.
 *
 *   WM_CHAR     - Appends printable characters to the text buffer and
 *                 invalidates the window so WM_PAINT redraws.
 *                 F3 posts WM_QUIT to close the application.
 *
 *   WM_SETWINDOWPARAMS / WM_QUERYWINDOWPARAMS
 *               - Allow external callers to set or query the window
 *                 text via the standard WPM_TEXT / WPM_CCHTEXT protocol.
 *
 *   WM_CLOSE    - Posts WM_QUIT to end the message loop.
 */
MRESULT EXPENTRY MyWindowProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   PSIMPLETEXT pSimpleText;
   RECTL  clientrect;                     /* Rectangle coordinates        */
   CHAR ch;
   HPS hpsPaint;

  switch( msg )
  {
    /* ----------------------------------------------------------------
     * WM_CREATE - Allocate per-instance data (SIMPLETEXT struct).
     * The pointer is stored in the extra window words reserved during
     * class registration (cbWindowData = 4).
     * ---------------------------------------------------------------- */
    case WM_CREATE:
      pSimpleText =(PSIMPLETEXT)malloc(sizeof(SIMPLETEXT));
      pSimpleText->CurLength=0;
      pSimpleText->Text[0]=0x00;
      WinSetWindowPtr(hwnd,0,pSimpleText);
      break;

    /* ----------------------------------------------------------------
     * WM_DESTROY - Free the text buffer allocated in WM_CREATE.
     * ---------------------------------------------------------------- */
    case WM_DESTROY:
      pSimpleText=WinQueryWindowPtr(hwnd,0);
      free(pSimpleText);
      break;

    /* ----------------------------------------------------------------
     * WM_PAINT - Repaint the client area.
     * Retrieves the current text buffer and delegates rendering to
     * DrawText(), which uses WinDrawText with word-break support.
     * ---------------------------------------------------------------- */
    case WM_PAINT:
      hpsPaint=WinBeginPaint( hwnd,0, &clientrect );
      pSimpleText =WinQueryWindowPtr(hwnd,0);
      if (pSimpleText) {
         DrawText(hwnd,hpsPaint,pSimpleText->Text);
      } /* endif */
      WinEndPaint( hpsPaint );                      /* Drawing is complete   */
      break;

    /* ----------------------------------------------------------------
     * WM_CHAR - Keyboard input.
     *
     * mp1 SHORT2 flags: KC_CHAR indicates a translatable character.
     * mp2 SHORT2: virtual key code (checked for VK_F3 to quit).
     *
     * Printable characters are appended to the SIMPLETEXT buffer; the
     * buffer wraps around to position 0 when MAXTXTLEN is reached.
     * After each character the client area is invalidated so that
     * WM_PAINT redraws the updated text.
     * ---------------------------------------------------------------- */
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

    /* ----------------------------------------------------------------
     * WM_SETWINDOWPARAMS - Set window text via the standard protocol.
     * External callers (e.g. WinSetWindowText) use this message to
     * replace the text buffer contents.
     * ---------------------------------------------------------------- */
   case WM_SETWINDOWPARAMS:
      pSimpleText =WinQueryWindowPtr(hwnd,0);
      switch (((PWNDPARAMS)mp1)->fsStatus) {
      case WPM_TEXT:
         strncpy(pSimpleText->Text,(const char *) ((PWNDPARAMS)mp1)->pszText,MAXTXTLEN-1);
         pSimpleText->Text[MAXTXTLEN-1]=0x00;
         pSimpleText->CurLength=strlen(pSimpleText->Text);
         return (MRESULT) TRUE;
         break;
      default:
         return (MRESULT) FALSE;
      } /* endswitch */
      break;

    /* ----------------------------------------------------------------
     * WM_QUERYWINDOWPARAMS - Query window text / text length.
     * Supports both WPM_CCHTEXT (return length) and WPM_TEXT (copy
     * text into caller-supplied buffer).
     * ---------------------------------------------------------------- */
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
            strncpy((char *) ((PWNDPARAMS)mp1)->pszText,
                    pSimpleText->Text,
                    ((PWNDPARAMS)mp1)->cchText);
            return (MPARAM)TRUE;
         }
      }
      break;

    /* ----------------------------------------------------------------
     * WM_CLOSE - Post WM_QUIT to terminate the message loop.
     * ---------------------------------------------------------------- */
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

/*********************************************************************
 *  DrawText - Render word-wrapped text in the client area.
 *
 *  Erases the client rectangle, queries the current font metrics to
 *  determine line height, then iterates through the text string
 *  calling WinDrawText with DT_WORDBREAK.  Each iteration draws as
 *  many characters as fit on one line; the rectangle's yTop is then
 *  reduced by the character height to advance to the next line.
 *
 *  Arguments:
 *    hwnd    - client window handle (used to query dimensions)
 *    hps     - presentation-space handle from WinBeginPaint
 *    pszText - null-terminated string to draw
 *********************************************************************/
void DrawText(HWND hwnd,HPS hps,char *pszText) {
 RECTL  rcl;             /* update region                        */
 LONG   cyCharHeight;     /* set character height                 */
 LONG   cchText;
 LONG   cchTotalDrawn;
 LONG   cchDrawn;
 FONTMETRICS  metrics;

 WinQueryWindowRect(hwnd, &rcl);         /* get window dimensions */

 GpiErase(hps);
 cchText =  (LONG)strlen(pszText);       /* get length of string  */
 GpiQueryFontMetrics( hps, sizeof(metrics), &metrics);
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
          (const char *) pszText +  cchTotalDrawn,    /* address of the text       */
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
