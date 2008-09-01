/* $Id: AROSMesa.c 24372 2006-04-24 01:32:58Z NicJA $ */

/*
 * Mesa 3-D graphics library
 * Copyright (C) 1995  Brian Paul  (brianp@ssec.wisc.edu)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "AROSMesa_intern.h"

#include <exec/memory.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/utility.h>

#include "aros_driver_functions.h"
#include "aros_rb_functions.h"
#include "aros_fb_functions.h"
#include "aros_visual_functions.h"
#include "aros_context_functions.h"

#include <aros/debug.h>
#include <proto/cybergraphics.h>
#include <cybergraphx/cybergraphics.h>

#include <graphics/rpattr.h>

#include <GL/AROSMesa.h>

#include <stdlib.h>
#include <stdio.h>

#include "glheader.h"
#include <GL/gl.h>
#include "context.h"
#include "extensions.h"
#include "framebuffer.h"
#include "swrast/swrast.h"
#include "swrast_setup/swrast_setup.h"
#include "tnl/tnl.h"
#include "tnl/t_context.h"
#include "tnl/t_pipeline.h"
#include "drivers/common/driverfuncs.h"
#include "vbo/vbo.h"

#include "renderbuffer.h"

/**********************************************************************/
/*****        Internal Data                     *****/
/**********************************************************************/

GLenum         LastError;                       /* The last error generated*/
struct Library * AROSMesaCyberGfxBase = NULL;    /* Base address for cybergfx */

/**********************************************************************/
/*****          AROS/Mesa API Functions          *****/
/**********************************************************************/
/*
 * Implement the client-visible AROS/Mesa interface functions defined
 * in Mesa/include/GL/AROSmesa.h
 *
 **********************************************************************/
/*
 * Implement the public AROS/Mesa interface functions defined
 * in Mesa/include/GL/AROSMesa.h
 */

//UBYTE Version[]="$VER: AROSMesa (linklib) 3.4.2";




/* Main AROSMesa API Functions */
void GLAPIENTRY
AROSMesaDestroyContext(AROSMesaContext amesa)
{
    /* destroy a AROS/Mesa context */
    D(bug("[AROSMESA] AROSMesaDestroyContext(amesa @ %x)\n", amesa));

    if (!amesa)
        return;

    GLcontext * ctx = GET_GL_CTX_PTR(amesa);

    if (ctx)
    {
        _mesa_make_current(NULL, NULL, NULL);


        _swsetup_DestroyContext(ctx);
        _swrast_DestroyContext(ctx);
        _tnl_DestroyContext(ctx);
        _vbo_DestroyContext(ctx);


        aros_delete_visual(amesa->visual);
        aros_delete_framebuffer(amesa->framebuffer);

        aros_delete_renderbuffer(amesa->front_rb);

        if (amesa->back_rb)
            aros_delete_renderbuffer(amesa->back_rb);

        aros_delete_context(amesa);
    }

}


void GLAPIENTRY
AROSMesaSwapBuffers(AROSMesaContext amesa)
{        
    /* copy/swap back buffer to front if applicable */

    D(bug("[AROSMESA] AROSMesaSwapBuffers(amesa @ %x)\n", amesa));
    

    if (amesa->back_rb)
    {
        UBYTE minterm = 0xc0;

        ClipBlit(amesa->back_rb->rp, amesa->left, amesa->top,  
            /* from */
            amesa->front_rb->rp, amesa->left, amesa->top,  
            /* to */
            amesa->width, amesa->height,  
            /* size */
            minterm);
    }
    else
    {
        D(bug("[AROSMESA] AROSMesaSwapBuffers: NOP - SINGLE buffer\n"));
    }
}

/*
 * Create a new rastport to use as a back buffer.
 * Input:  width, height - size in pixels
 *    depth - number of bitplanes
 */

static struct RastPort *
arosRasterizer_make_rastport( int width, int height, int depth, struct BitMap *friendbm )
{
    struct RastPort *rp = NULL;
    struct BitMap *bm = NULL;
    
    D(bug("[AROSMESA:RAST] arosRasterizer_make_rastport()\n"));


    if ((bm = AllocBitMap(width, height, depth, BMF_CLEAR|BMF_INTERLEAVED, friendbm)))
    {
        if ((rp = CreateRastPort()))
        {
            rp->BitMap = bm;
            return rp;
        }
        else
        {
            FreeBitMap(bm);
            return NULL;
        }
    }
    else return NULL;
}

static BOOL
aros_Standard_init(AROSMesaContext amesa, struct TagItem *tagList)
{
	struct Rectangle rastcliprect;
	struct TagItem crptags[] =
	{
	    { RPTAG_ClipRectangle      , (IPTR)&rastcliprect },
	    { RPTAG_ClipRectangleFlags , (RPCRF_RELRIGHT | RPCRF_RELBOTTOM) },
	    { TAG_DONE }
	};

    D(bug("[AROSMESA] aros_Standard_init(amesa @ %x, taglist @ %x)\n", amesa, tagList));

    if (!(amesa->visible_rp))
    {
        D(bug("[AROSMESA] aros_Standard_init: WARNING - NULL RastPort in context\n"));
        if (!(amesa->visible_rp = (struct RastPort *)GetTagData(AMA_RastPort, 0, tagList)))
        {
            D(bug("[AROSMESA] aros_Standard_init: ERROR - Launched with NULL RastPort\n"));
            return FALSE;
        }
    }	  
    
    D(bug("[AROSMESA] aros_Standard_init: Using RastPort @ %x\n", amesa->rp));

    amesa->visible_rp = CloneRastPort(amesa->visible_rp);

    D(bug("[AROSMESA] aros_Standard_init: Cloned RastPort @ %x\n", amesa->rp));

    amesa->visible_rp_width = 
        amesa->visible_rp->Layer->bounds.MaxX - amesa->visible_rp->Layer->bounds.MinX + 1;

    amesa->visible_rp_height = 
        amesa->visible_rp->Layer->bounds.MaxY - amesa->visible_rp->Layer->bounds.MinY + 1;

    amesa->left = GetTagData(AMA_Left, 0, tagList);
    amesa->right = GetTagData(AMA_Right, 0, tagList);
    amesa->top = GetTagData(AMA_Top, 0, tagList);
    amesa->bottom = GetTagData(AMA_Bottom, 0, tagList);

    amesa->width = GetTagData(AMA_Width, (amesa->visible_rp_width - amesa->left - amesa->right), tagList);
    amesa->height = GetTagData(AMA_Height, (amesa->visible_rp_height - amesa->top - amesa->bottom), tagList);

    amesa->clearpixel = 0;   /* current drawing/clearing pens */

    if (amesa->window)
    {
        D(bug("[AROSMESA] aros_Standard_init: Clipping Rastport to Window's dimensions\n"));
        /* Clip the rastport to the visible area */
        rastcliprect.MinX = amesa->left;
        rastcliprect.MinY = amesa->top;
        rastcliprect.MaxX = amesa->left + amesa->width;
        rastcliprect.MaxY = amesa->top + amesa->height;
        SetRPAttrsA(amesa->visible_rp, crptags);
    }

    D(bug("[AROSMESA] aros_Standard_init: Context Base dimensions set -:\n"));
    D(bug("[AROSMESA] aros_Standard_init:    amesa->RealWidth  = %d\n", amesa->RealWidth));
    D(bug("[AROSMESA] aros_Standard_init:    amesa->RealHeight = %d\n", amesa->RealHeight));
    D(bug("[AROSMESA] aros_Standard_init:    amesa->width      = %d\n", amesa->width));
    D(bug("[AROSMESA] aros_Standard_init:    amesa->height     = %d\n", amesa->height));
    D(bug("[AROSMESA] aros_Standard_init:    amesa->left       = %d\n", amesa->left));
    D(bug("[AROSMESA] aros_Standard_init:    amesa->right      = %d\n", amesa->right));
    D(bug("[AROSMESA] aros_Standard_init:    amesa->top        = %d\n", amesa->top));
    D(bug("[AROSMESA] aros_Standard_init:    amesa->bottom     = %d\n", amesa->bottom));
    D(bug("[AROSMESA] aros_Standard_init:    amesa->depth      = %d\n", amesa->depth));

    if (amesa->Screen)
    {
        if (amesa->depth == 0)
        {
            D(bug("[AROSMESA] aros_Standard_init: WARNING - Illegal RastPort Depth, attempting to correct\n"));
            amesa->depth = GetCyberMapAttr(amesa->visible_rp->BitMap, CYBRMATTR_DEPTH);
        }
    }

    return TRUE;
}


AROSMesaContext GLAPIENTRY
AROSMesaCreateContextTags(long Tag1, ...)
{
  return AROSMesaCreateContext((struct TagItem *)&Tag1);
}

AROSMesaContext GLAPIENTRY
AROSMesaCreateContext(struct TagItem *tagList)
{
    AROSMesaContext amesa = NULL;
    GLcontext * ctx = NULL;
    LastError = 0;
    struct dd_function_table functions;
    GLboolean db_flag = GL_FALSE;


    D(bug("[AROSMESA] AROSMesaCreateContext(taglist @ %x)\n", tagList));

    /* try to open cybergraphics.library */
    if (CyberGfxBase == NULL)
    {
        if (!(CyberGfxBase = OpenLibrary((UBYTE*)"cybergraphics.library",0)))
            return NULL;
    }

    if (!(amesa = (AROSMesaContext)GetTagData(AMA_Context, GL_FALSE, tagList)))
    {
        

        /* allocate arosmesa_context struct initialized to zeros */
        if (!(amesa = (AROSMesaContext)AllocVec(sizeof(struct arosmesa_context), MEMF_PUBLIC|MEMF_CLEAR)))
        {
            D(bug("[AROSMESA] AROSMesaCreateContext: Failed to allocate AROSMesaContext\n"));
            LastError = AMESA_OUT_OF_MEM;
            return NULL;
        }
        
        ctx = GET_GL_CTX_PTR(amesa);

        D(bug("[AROSMESA] AROSMesaCreateContext: AROSMesaContext Allocated @ %x\n", amesa));
        D(bug("[AROSMESA] AROSMesaCreateContext:          gl_ctx Allocated @ %x\n", ctx));

        /* CHANGE TO WORK EXCLUSIVELY WITH RASTPORTS! */
        amesa->Screen = (struct Screen *)GetTagData(AMA_Screen, 0, tagList);
        amesa->window = (struct Window *)GetTagData(AMA_Window, 0, tagList);
        amesa->visible_rp = (struct RastPort *)GetTagData(AMA_RastPort, 0, tagList);

        if (amesa->Screen)
        {
            D(bug("[AROSMESA] AROSMesaCreateContext: Screen @ %x\n", amesa->Screen));
            if (amesa->window)
            {
                D(bug("[AROSMESA] AROSMesaCreateContext: window @ %x\n", amesa->window));
                if (!(amesa->visible_rp))
                {
                    /* Use the windows rastport */
                    amesa->visible_rp = amesa->window->RPort;
                    D(bug("[AROSMESA] AROSMesaCreateContext: Windows RastPort @ %x\n", amesa->rp));
                }
            }
            else
            {
                if (!(amesa->visible_rp))
                {
                    /* Use the screens rastport */
                    amesa->visible_rp = &amesa->Screen->RastPort;
                    D(bug("[AROSMESA] AROSMesaCreateContext: Screens RastPort @ %x\n", amesa->rp));
                }
            }
        }
        else
        {
            /* Not passed a screen */
            if (amesa->window)
            {
                D(bug("[AROSMESA] AROSMesaCreateContext: Window @ %x\n", amesa->window));
                /* Use the windows Screen */
                amesa->Screen = amesa->window->WScreen;
                D(bug("[AROSMESA] AROSMesaCreateContext: Windows Screen @ %x\n", amesa->Screen));

                if (!(amesa->visible_rp))
                {
                    /* Use the windows rastport */
                    amesa->visible_rp = amesa->window->RPort;
                    D(bug("[AROSMESA] AROSMesaCreateContext: Windows RastPort @ %x\n", amesa->rp));
                }
            }
            else
            {
    		    /* Only Passed A Rastport */
                D(bug("[AROSMESA] AROSMesaCreateContext: Experimental: Using RastPort only!\n"));
            }
        }

        D(bug("[AROSMESA] AROSMesaCreateContext: Using RastPort @ %x\n", amesa->rp));

        D(bug("[AROSMESA] AROSMesaCreateContext: Creating new AROSMesaVisual\n"));
        if (((BOOL)GetTagData(AMA_DoubleBuf, TRUE, tagList) == TRUE))
            db_flag = GL_TRUE;

        if (!(amesa->visual = aros_new_visual(db_flag)))
        {
            D(bug("[AROSMESA] AROSMesaCreateContext: ERROR -  failed to create AROSMesaVisual\n"));
            FreeVec( amesa );
            LastError = AMESA_OUT_OF_MEM;
            return NULL;
        }

        D(bug("[AROSMESA] AROSMesaCreateContext: [ASSERT] RastPort @ %x\n", amesa->rp));

        /* build table of device driver functions */
        _mesa_init_driver_functions(&functions);
        _aros_init_driver_functions(&functions);


   

        if (!_mesa_initialize_context(  ctx,
                                        GET_GL_VIS_PTR(amesa->visual),
                                        NULL,
                                        &functions,
                                        (void *) amesa))
        {
	        aros_delete_visual(amesa->visual);
            FreeVec(amesa);
            return NULL;
        }

        _mesa_enable_sw_extensions(ctx);
        _mesa_enable_1_3_extensions(ctx);
        _mesa_enable_1_4_extensions(ctx);
        _mesa_enable_1_5_extensions(ctx);
        _mesa_enable_2_0_extensions(ctx);
        _mesa_enable_2_1_extensions(ctx);


        if ((aros_Standard_init(amesa, tagList)) == FALSE)
        {
            goto amccontextclean;
        }

  
        if(!(amesa->framebuffer = aros_new_framebuffer(GET_GL_VIS_PTR(amesa->visual))))
        {
            LastError = AMESA_OUT_OF_MEM;
            goto amccontextclean;
        }
    
        amesa->front_rb = aros_new_renderbuffer(amesa->visible_rp, GL_FALSE);   

        _mesa_add_renderbuffer(GET_GL_FB_PTR(amesa->framebuffer), BUFFER_FRONT_LEFT, 
            GET_GL_RB_PTR(amesa->front_rb));
        
        /* Set draw buffer as front */
        ctx->Color.DrawBuffer[0] = GL_FRONT;


        if (amesa->visual->db_flag == GL_TRUE)
        {
            /* Enable double buffer */
            struct RastPort * back_rp = arosRasterizer_make_rastport(amesa->visible_rp_width, 
                                            amesa->visible_rp_height, amesa->depth, amesa->visible_rp->BitMap);

            if (back_rp)
            {
                amesa->back_rb = aros_new_renderbuffer(back_rp, GL_TRUE);
      
                _mesa_add_renderbuffer(GET_GL_FB_PTR(amesa->framebuffer), BUFFER_BACK_LEFT, 
                    GET_GL_RB_PTR(amesa->back_rb));

                /* Set draw buffer as back */
                ctx->Color.DrawBuffer[0] = GL_BACK;
            }
        }


        _mesa_add_soft_renderbuffers(GET_GL_FB_PTR(amesa->framebuffer),
                                   GL_FALSE, /* color */
                                   (GET_GL_VIS_PTR(amesa->visual))->haveDepthBuffer,
                                   (GET_GL_VIS_PTR(amesa->visual))->haveStencilBuffer,
                                   (GET_GL_VIS_PTR(amesa->visual))->haveAccumBuffer,
                                   GL_FALSE, /* alpha */
                                   GL_FALSE /* aux */ );


    } /* if(!(amesa = (AROSMesaContext)GetTagData(AMA_Context, GL_FALSE, tagList))) */
 
    ctx = GET_GL_CTX_PTR(amesa);

    _swrast_CreateContext(ctx);
    _vbo_CreateContext(ctx);
    _tnl_CreateContext(ctx);
    _swsetup_CreateContext(ctx);


    _swsetup_Wakeup(ctx);


    /* use default TCL pipeline */
    TNL_CONTEXT(ctx)->Driver.RunPipeline = _tnl_run_pipeline;

    return amesa;

amccontextclean:
    if (amesa->visual)
        aros_delete_visual(amesa->visual);

    aros_delete_context(amesa);

    return NULL;
}













void GLAPIENTRY
AROSMesaMakeCurrent(AROSMesaContext amesa)
{
  /* Make the specified context the current one */
  /* the order of operations here is very important! */
  D(bug("[AROSMESA] AROSMesaMakeCurrent(amesa @ %x)\n", amesa));


  if (amesa)
  {
    struct gl_framebuffer * fb = GET_GL_FB_PTR(amesa->framebuffer);
    GLcontext * ctx = GET_GL_CTX_PTR(amesa);
    _glapi_check_multithread();

   //osmesa->rb->Width = osmesa->rb->Height = 0;

   /* Set the framebuffer's size.  This causes the
    * osmesa_renderbuffer_storage() function to get called.
    */
   _mesa_resize_framebuffer(ctx, fb, amesa->width, amesa->height);
   fb->Initialized = GL_TRUE; /* XXX TEMPORARY? */

    _mesa_make_current(ctx, fb, fb);

   /* Remove renderbuffer attachment, then re-add.  This installs the
    * renderbuffer adaptor/wrapper if needed (for bpp conversion).
    */

   /* this updates the visual's red/green/blue/alphaBits fields */
   _mesa_update_framebuffer_visual(fb);

   /* update the framebuffer size */
   _mesa_resize_framebuffer(ctx, fb, amesa->width, amesa->height);
   
    
    D(bug("[AROSMESA] AROSMesaMakeCurrent: set current mesa context/buffer\n"));



    D(bug("[AROSMESA] AROSMesaMakeCurrent: initialised rasterizer driver functions\n"));
    

  }
}

