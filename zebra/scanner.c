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
#ifdef DEBUG_SCANNER
# include <stdio.h>     /* fprintf */
#endif
#include <stdlib.h>     /* calloc, free, abs */

#include "zebra.h"

#ifdef DEBUG_SCANNER
# define dprintf(...) \
    fprintf(stderr, __VA_ARGS__)
#else
# define dprintf(...)
#endif

#ifndef ZEBRA_FIXED
# define ZEBRA_FIXED 5
#endif

/* scanner state */
struct zebra_scanner_s {
    zebra_decoder_t *decoder; /* associated bar width decoder */

    unsigned x;             /* relative scan position of next sample */
    int y0[4];              /* short circular buffer of average intensities */

    int y1_sign;            /* slope at last crossing */
    unsigned y1_thresh;     /* current slope threshold */
    unsigned y1_min_thresh; /* minimum threshold */

    unsigned cur_edge;      /* interpolated position of tracking edge */
    unsigned last_edge;     /* interpolated position of last located edge */
    unsigned width;         /* last element width */
};

zebra_scanner_t *zebra_scanner_create (zebra_decoder_t *dcode)
{
    zebra_scanner_t *scn = calloc(1, sizeof(zebra_scanner_t));
    scn->decoder = dcode;
    scn->y1_thresh = scn->y1_min_thresh = 8;
    return(scn);
}

void zebra_scanner_destroy (zebra_scanner_t *scn)
{
    free(scn);
}

unsigned zebra_scanner_get_width (const zebra_scanner_t *scn)
{
    return(scn->width);
}

zebra_color_t zebra_scanner_get_color (const zebra_scanner_t *scn)
{
    return((scn->y1_sign <= 0) ? ZEBRA_SPACE : ZEBRA_BAR);
}

static inline unsigned calc_thresh (zebra_scanner_t *scn)
{
    /* threshold 1st to improve noise rejection */
    unsigned thresh = scn->y1_thresh;
    if((thresh <= scn->y1_min_thresh) || !scn->width) {
        dprintf(" tmin=%d", scn->y1_min_thresh);
        return(scn->y1_min_thresh);
    }
    /* slowly return threshold to min */
    unsigned long t = thresh * ((scn->x << ZEBRA_FIXED) - scn->last_edge);
    t /= scn->width;
    t /= 4; /* FIXME add config API */
    t = ((t >> (ZEBRA_FIXED - 1)) + 1) >> 1;
    thresh -= t;
    dprintf(" thr=%d t=%ld x=%d last=%d.%d",
            thresh, t, scn->x, scn->last_edge >> ZEBRA_FIXED,
            scn->last_edge & ((1 << ZEBRA_FIXED) - 1));
    if(thresh < scn->y1_min_thresh)
        thresh = scn->y1_thresh = scn->y1_min_thresh;
    return(thresh);
}

static inline zebra_symbol_type_t process_edge (zebra_scanner_t *scn,
                                                int y1)
{
    scn->width = scn->cur_edge - scn->last_edge;
    dprintf(" sgn=%d cur=%d.%d w=%d\n",
            scn->y1_sign, scn->cur_edge >> ZEBRA_FIXED,
            scn->cur_edge & ((1 << ZEBRA_FIXED) - 1), scn->width);
    scn->last_edge = scn->cur_edge;

    /* adaptive thresholding */
    /* start at 1/4 new min/max */
    scn->y1_thresh = abs((y1 + 1) / 2);
    if(scn->y1_thresh < scn->y1_min_thresh)
        scn->y1_thresh = scn->y1_min_thresh;

    /* pass to decoder */
    if(scn->width) {
        scn->y1_sign = y1;
        if(scn->decoder)
            return(zebra_decode_width(scn->decoder, scn->width));
        return(ZEBRA_PARTIAL);
    }
    /* skip initial transition */
    return(ZEBRA_NONE);
}

zebra_symbol_type_t zebra_scan_y (zebra_scanner_t *scn,
                                  int y)
{
    /* retrieve short value history */
    register int y0_1 = scn->y0[(scn->x - 1) & 3];
    register int y0_0;
    if(scn->x)
        /* update weighted moving average */
        y0_0 = scn->y0[scn->x & 3] = y0_1 + ((y - y0_1 + 1) / 2);
    else
        y0_0 = scn->y0[0] = scn->y0[1] = scn->y0[2] = scn->y0[3] = y;
    register int y0_2 = scn->y0[(scn->x - 2) & 3];
    register int y0_3 = scn->y0[(scn->x - 3) & 3];
    /* 1st differential @ x-1 */
    register int y1_1 = y0_0 - y0_2;
    {
        register int y1_2 = y0_1 - y0_3;
        if(abs(y1_1) < abs(y1_2)) y1_1 = y1_2;
    }
    /* 2nd differentials @ x-1 & x-2 */
    register int y2_1 = y0_0 - (y0_1 * 2) + y0_2;
    register int y2_2 = y0_1 - (y0_2 * 2) + y0_3;

    dprintf("scan: y=%d y0=%d y1=%d y2=%d", y, y0_1, y1_1, y2_1);

    zebra_symbol_type_t edge = ZEBRA_NONE;
    /* 2nd zero-crossing is 1st local min/max - could be edge */
    if((!y2_1 ||
        ((y2_1 > 0) ? y2_2 < 0 : y2_2 > 0)) &&
       (calc_thresh(scn) < abs(y1_1)))
    {
        /* check for 1st sign change */
        if((scn->y1_sign > 0) ? y1_1 < 0 : y1_1 > 0)
            /* intensity change reversal - finalize previous edge */
            edge = process_edge(scn, y1_1);
        else
            dprintf("\n");

        /* update current edge */
        int d = y2_1 - y2_2;
        scn->cur_edge = 1 << ZEBRA_FIXED;
        if(!d)
            scn->cur_edge >>= 1;
        else if(y2_1)
            /* interpolate zero crossing */
            scn->cur_edge -= ((y2_1 << ZEBRA_FIXED) + 1) / d;
        scn->cur_edge += scn->x << ZEBRA_FIXED;
    }
    else
        dprintf("\n");
    /* FIXME add fall-thru pass to decoder after heuristic "idle" period
       (eg, 6-8 * last width) */
    scn->x++;
    return(edge);
}

/* undocumented API for drawing cutesy debug graphics */
void zebra_scanner_get_state (const zebra_scanner_t *scn,
                              unsigned *x,
                              unsigned *cur_edge,
                              unsigned *last_edge,
                              int *y0,
                              int *y1,
                              int *y2,
                              int *y1_thresh)
{
    register int y0_0 = scn->y0[scn->x & 0x3];
    register int y0_1 = scn->y0[(scn->x - 1) & 3];
    register int y0_2 = scn->y0[(scn->x - 2) & 3];
    if(x) *x = scn->x - 1;
    if(cur_edge) *cur_edge = scn->cur_edge;
    if(last_edge) *last_edge = scn->last_edge;
    if(y0) *y0 = y0_1;
    if(y1) *y1 = y0_0 - y0_2;
    if(y2) *y2 = y0_0 - (y0_1 * 2) + y0_2;
    if(y1_thresh) *y1_thresh = calc_thresh((zebra_scanner_t*)scn);
}
