#include "aros_driver_functions.h"


#include <aros/debug.h>
#include <proto/cybergraphics.h>


#include "AROSMesa_intern.h"

#include <GL/AROSMesa.h>
#include "swrast/swrast.h"
#include "tnl/tnl.h"
#include "swrast_setup/swrast_setup.h"
#include "vbo/vbo.h"

const GLubyte * aros_get_string(GLcontext *ctx, GLenum name)
{
    D(bug("[AROSMESA] aros_renderer_string()\n"));

    (void) ctx;
    switch (name)
    {
        case GL_RENDERER:
            return "AROS TrueColor Rasterizer";
        default:
            return NULL;
    }
}

/* implements glClearColor - Set the color used to clear the color buffer. */
void aros_clear_color( GLcontext *ctx, const GLfloat color[4] )
{
    AROSMesaContext amesa = (AROSMesaContext)ctx->DriverCtx;
    amesa->clearpixel = TC_ARGB((GLbyte)(color[RCOMP] * 255), 
                                (GLbyte)(color[GCOMP] * 255), 
                                (GLbyte)(color[BCOMP] * 255), 
                                (GLbyte)(color[ACOMP] * 255));
    
    D(bug("[AROSMESA] aros_clear_color(c=%x,r=%d,g=%d,b=%d,a=%d) = %x\n", 
        ctx, (GLbyte)(color[RCOMP] * 255), (GLbyte)(color[GCOMP] * 255), (GLbyte)(color[BCOMP] * 255),
        (GLbyte)(color[ACOMP] * 255), amesa->clearpixel));
}


/*
* Clear the specified region of the color buffer using the clear color
* or index as specified by one of the two functions above.
*/
void aros_clear(GLcontext* ctx, GLbitfield mask)
{
    AROSMesaContext amesa = (AROSMesaContext)ctx->DriverCtx;

    D(bug("[AROSMESA] aros_clear(x:%d,y:%d,w:%d,h:%d)\n", ctx->Viewport.X, 
        ctx->Viewport.Y, ctx->Viewport.Width, ctx->Viewport.Height));

    if ((mask & (BUFFER_BIT_FRONT_LEFT)))
    {  
        if(amesa->front_rb->rp != NULL)
        {
            D(bug("[AROSMESA] aros_clear: front_rp->Clearing viewport (ALL)\n"));
            FillPixelArray (amesa->front_rb->rp, FIXx(ctx->Viewport.X), (FIXy(ctx->Viewport.Y) - ctx->Viewport.Height) + 1, 
                /*FIXx(ctx->Viewport.X)+*/ctx->Viewport.Width - FIXx(ctx->Viewport.X), 
                ctx->Viewport.Height/*FIXy(ctx->Viewport.Y)*/, amesa->clearpixel);

            mask &= ~BUFFER_BIT_FRONT_LEFT;
        }
        else
        {
            D(bug("[AROSMESA] aros_clear: Serious ERROR amesa->front_rp = NULL detected\n"));
        }
    }

    if ((mask & (BUFFER_BIT_BACK_LEFT)))
    {  
        if(amesa->back_rb->rp != NULL)
        {
            D(bug("[AROSMESA] aros_clear: back_rp->Clearing viewport (ALL)\n"));
            FillPixelArray (amesa->back_rb->rp, FIXx(ctx->Viewport.X), (FIXy(ctx->Viewport.Y) - ctx->Viewport.Height) + 1, 
                /*FIXx(ctx->Viewport.X)+*/ctx->Viewport.Width - FIXx(ctx->Viewport.X), 
                ctx->Viewport.Height/*FIXy(ctx->Viewport.Y)*/, amesa->clearpixel);

            mask &= ~BUFFER_BIT_BACK_LEFT;
        }
        else
        {
            D(bug("[AROSMESA] aros_clear: Serious ERROR amesa->back_rp = NULL detected\n"));
        }    
    }
  
    if (mask)
        _swrast_Clear( ctx, mask ); /* Remaining buffers to be cleared .. */
}

void aros_update_state(GLcontext *ctx, GLbitfield new_state)
{
    _swrast_InvalidateState(ctx, new_state);
    _swsetup_InvalidateState(ctx, new_state);
    _vbo_InvalidateState(ctx, new_state);
    _tnl_InvalidateState(ctx, new_state);
}

void _aros_init_driver_functions(struct dd_function_table * functions)
{
    functions->GetString = aros_get_string;
    functions->UpdateState = aros_update_state;
    functions->GetBufferSize = NULL;
    functions->Clear = aros_clear;
    functions->ClearColor = aros_clear_color;
}
