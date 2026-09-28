/*
 * Transaction helpers for the opcodes of fm_${{ values.module }}.
 *
 * BRM has no nested transactions. An opcode opens one only if its caller has
 * not, and only the opcode that opened it commits or aborts it.
 */
#ifndef ${{ values.moduleUpper }}_TRANS_H
#define ${{ values.moduleUpper }}_TRANS_H

#include "pcm.h"

/*
 * Opens a transaction on the database of the input POID, unless one is
 * already open. Returns LOCAL_TRANS_OPEN_SUCCESS if it opened one.
 */
int32 ${{ values.module }}_trans_open(pcm_context_t *ctxp, int32 flags, pin_flist_t *i_flistp, pin_errbuf_t *ebufp);

/*
 * Commits the transaction if ${{ values.module }}_trans_open opened it, or aborts
 * it if ebufp holds an error. Does nothing otherwise.
 */
void ${{ values.module }}_trans_close(pcm_context_t *ctxp, int32 local, pin_errbuf_t *ebufp);

#endif /* ${{ values.moduleUpper }}_TRANS_H */
