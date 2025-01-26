        /*
        fv7 = fr0 * fr4 +
              fr1 * fr5 +
              fr2 * fr6 +
              fr3 * fr7;
        */

        /*
        a in r4
        b in r5
        */

        .global _fipr
_fipr:
        fmov.s @r4+,fr0
        fmov.s @r4+,fr1
        fmov.s @r4+,fr2
        fmov.s @r4+,fr3

        fmov.s @r5+,fr4
        fmov.s @r5+,fr5
        fmov.s @r5+,fr6
        fmov.s @r5+,fr7

        fipr    fv0,fv4

        rts
        fmov fr7,fr0

_load:
        /* x - 1, y - 1 = - 1 - 640 */
        /* x    , y - 1 = + 0 - 640 */
        /* x + 1, y - 1 = + 1 - 640 */

        /* x - 1, y   1 = - 1 + 0   */
        /* x    , y   1 = + 0 + 0   */
        /* x + 1, y   1 = + 1 + 0   */

        /* x - 1, y + 1 = - 1 + 640 */
        /* x    , y + 1 = + 0 + 640 */
        /* x + 1, y + 1 = + 1 + 640 */
