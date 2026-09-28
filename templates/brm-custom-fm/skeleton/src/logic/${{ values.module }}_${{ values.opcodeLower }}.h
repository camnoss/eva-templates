/*
 * Flist logic of ${{ values.opcodeMacro }}.
 *
 * No PCM calls here, so the unit tests run it without a CM. The handler in
 * src/ops/ does the PCM work. Every function returns right away if ebufp
 * already holds an error, and the caller owns any flist it returns.
 */
#ifndef ${{ values.moduleUpper }}_${{ values.opcodeName }}_H
#define ${{ values.moduleUpper }}_${{ values.opcodeName }}_H

#include "pcm.h"

/* Sets ebufp if the input flist is not valid. */
void ${{ values.module }}_${{ values.opcodeLower }}_validate(pin_flist_t *i_flistp, pin_errbuf_t *ebufp);

/* Builds the PCM_OP_READ_FLDS input for the account. */
pin_flist_t *${{ values.module }}_${{ values.opcodeLower }}_read_flist(pin_flist_t *i_flistp, pin_errbuf_t *ebufp);

/* Builds the opcode output from the PCM_OP_READ_FLDS result. */
pin_flist_t *${{ values.module }}_${{ values.opcodeLower }}_output(pin_flist_t *read_flistp, pin_errbuf_t *ebufp);

#endif /* ${{ values.moduleUpper }}_${{ values.opcodeName }}_H */
