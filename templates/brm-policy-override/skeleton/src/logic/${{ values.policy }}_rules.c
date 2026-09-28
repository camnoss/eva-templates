#include <stddef.h>

#include "pcm.h"
#include "pin_errs.h"

#include "logic/${{ values.policy }}_rules.h"

void
${{ values.policy }}_rules_before(pin_flist_t *i_flistp, pin_errbuf_t *ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}

	/*
	 * Example rule: the input must carry a POID. Replace it with yours, and
	 * report a rejected input with PIN_ERRCLASS_APPLICATION and the field at
	 * fault, so callers can tell a bad request from a system error.
	 */
	if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp) == NULL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_APPLICATION, PIN_ERR_MISSING_ARG,
			PIN_FLD_POID, 0, 0);
	}
}

void
${{ values.policy }}_rules_after(pin_flist_t *i_flistp, pin_flist_t *o_flistp, pin_errbuf_t *ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}

	/* Nothing yet: the stock output is returned as is. */
}
