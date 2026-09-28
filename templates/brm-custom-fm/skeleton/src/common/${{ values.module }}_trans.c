#include <stddef.h>

#include "pcm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"

#include "common/${{ values.module }}_trans.h"

int32
${{ values.module }}_trans_open(pcm_context_t *ctxp, int32 flags, pin_flist_t *i_flistp, pin_errbuf_t *ebufp)
{
	poid_t *pdp = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return LOCAL_TRANS_OPEN_FAIL;
	}

	pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		return LOCAL_TRANS_OPEN_FAIL;
	}
	return fm_utils_trans_open(ctxp, flags, pdp, ebufp);
}

void
${{ values.module }}_trans_close(pcm_context_t *ctxp, int32 local, pin_errbuf_t *ebufp)
{
	pin_errbuf_t abort_ebuf;

	if (local != LOCAL_TRANS_OPEN_SUCCESS) {
		return;
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		/* Own errbuf, so the caller still sees the error that caused the abort. */
		PIN_ERRBUF_CLEAR(&abort_ebuf);
		fm_utils_trans_abort(ctxp, &abort_ebuf);
		if (PIN_ERR_IS_ERR(&abort_ebuf)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "${{ values.module }}_trans_close: abort failed", &abort_ebuf);
		}
		return;
	}

	fm_utils_trans_commit(ctxp, ebufp);
}
