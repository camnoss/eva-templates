/*
 * ${{ values.opcodeMacro }}
 *
 * Input flist:
 *   0 PIN_FLD_POID          POID [0] /account
 * Output flist:
 *   0 PIN_FLD_POID          POID [0] /account
 *   0 PIN_FLD_ACCOUNT_NO     STR [0]
 *   0 PIN_FLD_STATUS        ENUM [0]
 *
 * Starts as a read-only account lookup. Replace the logic in src/logic/ and
 * keep the shape of this handler: check the opcode, validate, open a
 * transaction only if the caller has not, check the errbuf after every call,
 * and free every flist on every path.
 */
#include <stddef.h>

#include "pcm.h"
#include "ops/base.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"

#include "${{ values.module }}_ops.h"
#include "common/${{ values.module }}_trans.h"
#include "logic/${{ values.module }}_${{ values.opcodeLower }}.h"

void
op_${{ values.module }}_${{ values.opcodeLower }}(cm_nap_connection_t *connp, int32 opcode, int32 flags,
	pin_flist_t *i_flistp, pin_flist_t **o_flistpp, pin_errbuf_t *ebufp)
{
	pcm_context_t *ctxp = connp->dm_ctx;
	pin_flist_t *read_flistp = NULL;
	pin_flist_t *read_res_flistp = NULL;
	int32 local = LOCAL_TRANS_OPEN_FAIL;

	*o_flistpp = NULL;
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	if (opcode != ${{ values.opcodeMacro }}) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_OPCODE, 0,
			0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "op_${{ values.module }}_${{ values.opcodeLower }}: bad opcode", ebufp);
		return;
	}

	/* Flists can hold customer data: log them at debug level only. */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_${{ values.module }}_${{ values.opcodeLower }} input flist", i_flistp);

	${{ values.module }}_${{ values.opcodeLower }}_validate(i_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		goto cleanup;
	}

	/*
	 * Read-only for now. Before the first write, switch to
	 * PCM_TRANS_OPEN_READWRITE and lock the account with PCM_OPFLG_LOCK_OBJ.
	 */
	local = ${{ values.module }}_trans_open(ctxp, PCM_TRANS_OPEN_READONLY, i_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		goto cleanup;
	}

	read_flistp = ${{ values.module }}_${{ values.opcodeLower }}_read_flist(i_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		goto cleanup;
	}

	/* READ_FLDS fetches only the fields asked for. Prefer it to READ_OBJ. */
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &read_res_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		goto cleanup;
	}

	*o_flistpp = ${{ values.module }}_${{ values.opcodeLower }}_output(read_res_flistp, ebufp);

cleanup:
	${{ values.module }}_trans_close(ctxp, local, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "op_${{ values.module }}_${{ values.opcodeLower }} error", ebufp);
		PIN_FLIST_DESTROY_EX(o_flistpp, NULL);
	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_${{ values.module }}_${{ values.opcodeLower }} output flist", *o_flistpp);
	}
	PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
}
