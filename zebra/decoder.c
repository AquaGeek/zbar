/*------------------------------------------------------------------------
 *  Copyright 2007 (c) Jeff Brown <spadix@users.sourceforge.net>
 *
 *  This file is part of the Zebra Barcode Library.
 *
 *  The Zebra Barcode Library is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU Lesser Public License as
 *  published by the Free Software Foundation; either version 2.1 of
 *  the License, or (at your option) any later version.
 *
 *  The Zebra Barcode Library is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser Public License
 *  along with the Zebra Barcode Library; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301  USA
 *
 *  http://sourceforge.net/projects/zebra
 *------------------------------------------------------------------------*/

#include <config.h>
#ifdef DEBUG_DECODER
# include <stdio.h>     /* fprintf */
#endif
#include <stdlib.h>     /* malloc, free */
#include <string.h>     /* memset */
#include <assert.h>

#include "decoder.h"

#ifdef DEBUG_DECODER
# define dprintf(...) \
    fprintf(stderr, __VA_ARGS__)
#else
# define dprintf(...)
#endif

zebra_decoder_t *zebra_decoder_create ()
{
    zebra_decoder_t *dcode = malloc(sizeof(zebra_decoder_t));
    dcode->buflen = BUFFER_MIN;
    dcode->buf = malloc(dcode->buflen);
    zebra_decoder_reset(dcode);
    return(dcode);
}

void zebra_decoder_destroy (zebra_decoder_t *dcode)
{
    free(dcode);
}

void zebra_decoder_reset (zebra_decoder_t *dcode)
{
    memset(dcode, 0, (long)&dcode->buf - (long)dcode);
    ean_reset(&dcode->ean);
    code128_reset(&dcode->code128);
}

void zebra_decoder_new_scan (zebra_decoder_t *dcode)
{
    /* soft reset decoder */
    memset(dcode->w, 0, sizeof(dcode->w));
    dcode->idx = 0;
    ean_new_scan(&dcode->ean);
    code128_reset(&dcode->code128);
}

const char *zebra_decoder_get_data (const zebra_decoder_t *dcode)
{
    return(dcode->buf);
}

zebra_decoder_handler_t *
zebra_decoder_set_handler (zebra_decoder_t *dcode,
                           zebra_decoder_handler_t handler)
{
    zebra_decoder_handler_t *result = dcode->handler;
    dcode->handler = handler;
    return(result);
}

void zebra_decoder_set_userdata (zebra_decoder_t *dcode,
                                 void *userdata)
{
    dcode->userdata = userdata;
}

void *zebra_decoder_get_userdata (const zebra_decoder_t *dcode)
{
    return(dcode->userdata);
}

zebra_symbol_type_t zebra_decoder_get_type (const zebra_decoder_t *dcode)
{
    return(dcode->type);
}

const char *zebra_get_symbol_name (zebra_symbol_type_t sym)
{
    switch(sym & ZEBRA_SYMBOL) {
    case ZEBRA_EAN8: return("EAN8");
    case ZEBRA_UPCE: return("UPC-E");
    case ZEBRA_UPCA: return("UPC-A");
    case ZEBRA_EAN13: return("EAN-13");
    case ZEBRA_CODE128: return("CODE-128");
    default: return("UNKNOWN");
    }
}

const char *zebra_get_addon_name (zebra_symbol_type_t sym)
{
    switch(sym & ZEBRA_ADDON) {
    case ZEBRA_ADDON2: return("+2");
    case ZEBRA_ADDON5: return("+5");
    default: return("");
    }
}

zebra_symbol_type_t zebra_decode_width (zebra_decoder_t *dcode,
                                        unsigned w)
{
    dcode->w[dcode->idx & 7] = w;
    dprintf("    decode[%x]: w=%d\n", dcode->idx, w);

    /* each decoder processes width stream in parallel */
    zebra_symbol_type_t sym = dcode->type = ZEBRA_NONE;

#define ENABLE_EAN
#ifdef ENABLE_EAN
    if((sym = zebra_decode_ean(dcode)))
        dcode->type = sym;
#endif

#define ENABLE_CODE128
#ifdef ENABLE_CODE128
    if((sym = zebra_decode_code128(dcode)) > ZEBRA_PARTIAL)
        dcode->type = sym;
#endif

    dcode->idx++;
    if(dcode->type) {
        if(dcode->handler)
            dcode->handler(dcode);
        if(dcode->lock)
            dcode->lock = 0;
    }
    return(dcode->type);
}
