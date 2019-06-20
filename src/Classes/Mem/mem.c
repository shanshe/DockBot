/************************************
**
**  DockBot - A Dock For AmigaOS 3
**
**  � 2019 Andrew Kennan
**
************************************/

#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/cghooks.h>
#include <exec/memory.h>
#include <dos/dos.h>

#include <proto/graphics.h>
#include <proto/exec.h>
#include <proto/intuition.h>

#include <string.h>

#include "dockbot.h"

#include "dockbot_protos.h"
#include "dockbot_pragmas.h"

#include "class_def.h"

extern struct GfxBase *GfxBase;

VOID __saveds set_text_font(struct IntuiText *text)
{
    struct Screen *screen;
    struct DrawInfo *drawInfo;
    
    if( screen = LockPubScreen(NULL) ) {
    
        if( drawInfo = GetScreenDrawInfo(screen) ) {

            text->ITextFont->ta_Name = drawInfo->dri_Font->tf_Message.mn_Node.ln_Name;
            text->ITextFont->ta_YSize = drawInfo->dri_Font->tf_YSize;
            text->ITextFont->ta_Style = drawInfo->dri_Font->tf_Style;
            text->ITextFont->ta_Flags = drawInfo->dri_Font->tf_Flags;
            text->FrontPen = drawInfo->dri_Pens[TEXTPEN];
            text->BackPen = drawInfo->dri_Pens[BACKGROUNDPEN];
            text->DrawMode = JAM2;

            FreeScreenDrawInfo(screen, drawInfo);
        }

        UnlockPubScreen(NULL, screen);
    }     
}

ULONG __saveds mem_lib_init(struct MemLibData *mld)
{
    if( mld->graphicsLib = OpenLibrary("graphics.library", 37) ) {
        GfxBase = (struct GfxBase *)mld->graphicsLib;
        return 1;
    }
    return 0;
}

ULONG __saveds mem_lib_expunge(struct MemLibData *mld)
{
    if( mld->graphicsLib ) {
        CloseLibrary(mld->graphicsLib);
    }
    return 1;
}

DB_METHOD_M(DRAW,DockMessageDraw)

    struct Screen *screen;
    struct DrawInfo *drawInfo;
    struct Rect b, barFrame;
    struct IntuiText l1, l2;
    struct TextAttr ta1, ta2;
    UWORD w;
    ULONG totalChip, freeChip, totalFast, freeFast;
    
    totalChip = AvailMem(MEMF_CHIP | MEMF_TOTAL);
    freeChip = AvailMem(MEMF_CHIP);
    
    totalFast = AvailMem(MEMF_FAST | MEMF_TOTAL);
    freeFast = AvailMem(MEMF_FAST);

    DB_GetDockGadgetBounds(o, &b);

    DB_DrawOutsetFrame(msg->rp, &b);

    l1.ITextFont = &ta1;
    set_text_font(&l1);

    l1.LeftEdge = b.x + 3;
    l1.TopEdge = b.y + 1 + ((b.h - ((l1.ITextFont->ta_YSize + 2) * 2)) / 2);
    l1.IText = "C:";
    l1.NextText = NULL;
            
    if( totalFast > 0 ) {
      l1.NextText = &l2;  

      l2.ITextFont = &ta2;
      set_text_font(&l2);

      l2.LeftEdge = l1.LeftEdge;
      l2.TopEdge = l1.TopEdge + 2 + l1.ITextFont->ta_YSize;
      l2.IText = "F:";
      l2.NextText = NULL;
    }

    w = IntuiTextLength(&l1);

    PrintIText(msg->rp, &l1, 0, 0);

    barFrame.x = b.x + w + 2;
    barFrame.w = b.w - w - 5;
    barFrame.y = l1.TopEdge;
    barFrame.h = l1.ITextFont->ta_YSize;

    if( screen = LockPubScreen(NULL) ) {
    
        if( drawInfo = GetScreenDrawInfo(screen) ) {

            SetAPen(msg->rp, drawInfo->dri_Pens[BACKGROUNDPEN]);
            RectFill(msg->rp, barFrame.x, barFrame.y, barFrame.w + barFrame.x, barFrame.h + barFrame.y);

            DB_DrawInsetFrame(msg->rp, &barFrame);

            w = (UWORD)((((totalChip - freeChip) * 100) / totalChip) * (barFrame.w - 2)) / 100;
            SetAPen(msg->rp, drawInfo->dri_Pens[FILLPEN]);
            RectFill(msg->rp, barFrame.x + 1, barFrame.y + 1, barFrame.x + 1 + w, barFrame.h + barFrame.y - 2);
            
            if( totalFast > 0 ) {

                barFrame.y = l2.TopEdge;

                SetAPen(msg->rp, drawInfo->dri_Pens[BACKGROUNDPEN]);
                RectFill(msg->rp, barFrame.x, barFrame.y, barFrame.w + barFrame.x, barFrame.h + barFrame.y);

                DB_DrawInsetFrame(msg->rp, &barFrame);

                w = (UWORD)((((totalFast - freeFast) * 100) / totalFast) * (barFrame.w - 2)) / 100;
                SetAPen(msg->rp, drawInfo->dri_Pens[FILLPEN]);
                RectFill(msg->rp, barFrame.x + 1, barFrame.y + 1, barFrame.x + 1 + w, barFrame.h + barFrame.y - 2);
            }

            FreeScreenDrawInfo(screen, drawInfo);
        }

        UnlockPubScreen(NULL, screen);
    }     
    return 1;    
}

DB_METHOD_D(TICK)

    if( data->counter > 0 ) {
        data->counter--;
        return 1;
    }

    data->counter = 50;

    DB_RequestDockGadgetDraw(o);

    return 1;
}

DB_METHOD_M(GETSIZE,DockMessageGetSize)

    struct IntuiText text;
    struct TextAttr ta;
    UWORD rows;    

    text.ITextFont = &ta;
    set_text_font(&text);

    rows = (AvailMem(MEMF_CHIP|MEMF_TOTAL) > 0 ? 1 : 0)
            + (AvailMem(MEMF_FAST|MEMF_TOTAL) > 0 ? 1 : 0);

    msg->w = DEFAULT_SIZE;
    msg->h = rows * (2 + text.ITextFont->ta_YSize) + 4;
 
    return 1;
}

