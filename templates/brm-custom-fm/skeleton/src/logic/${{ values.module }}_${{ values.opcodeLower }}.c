#include <string.h>

#include "pcm.h"
#include "pin_errs.h"

#include "logic/${{ values.module }}_${{ values.opcodeLower }}.h"

void
${{ values.module }}_${{ values.opcodeLower }}_validate(pin_flist_t *i_flistp, pin_errbuf_t *ebufp)
{
	poid_t *a_pdp = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}

	/* Optional GET, so a missing field is reported as a missing argument. */
	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	if (a_pdp == NULL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_APPLICATION, PIN_ERR_MISSING_ARG,
			PIN_FLD_POID, 0, 0);
		return;
	}

	if (strcmp(PIN_POID_GET_TYPE(a_pdp), "/account") != 0) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_APPLICATION, PIN_ERR_BAD_POID_TYPE,
			PIN_FLD_POID, 0, 0);
	}
}

pin_flist_t *
${{ values.module }}_${{ values.opcodeLower }}_read_flist(pin_flist_t *i_flistp, pin_errbuf_t *ebufp)
{
	pin_flist_t *r_flistp = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return NULL;
	}

	r_flistp = PIN_FLIST_CREATE(ebufp);
	/* COPY, not GET and PUT: the input flist still owns its POID. */
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
	/* NULL values ask READ_FLDS for these fields. */
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, NULL, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	}
	return r_flistp;
}

pin_flist_t *
${{ values.module }}_${{ values.opcodeLower }}_output(pin_flist_t *read_flistp, pin_errbuf_t *ebufp)
{
	pin_flist_t *o_flistp = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return NULL;
	}

	/* Build the output explicitly, so it only changes when this code does. */
	o_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(read_flistp, PIN_FLD_POID, o_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(read_flistp, PIN_FLD_ACCOUNT_NO, o_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
	PIN_FLIST_FLD_COPY(read_flistp, PIN_FLD_STATUS, o_flistp, PIN_FLD_STATUS, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_FLIST_DESTROY_EX(&o_flistp, NULL);
	}
	return o_flistp;
}
