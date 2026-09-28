/*
 * Custom ${{ values.policyOpcode }}.
 *
 * Wraps Oracle's stock op_${{ values.policy }}() instead of replacing it: our
 * rules run before and after it, and the stock behavior stays in place. The
 * build points the policy library's config table at this function (see
 * overrides.conf), so the CM calls it instead of the stock one.
 *
 * Keep the shape of this wrapper: rules go in src/logic/, where they can be
 * unit tested.
 */
#include <stddef.h>

#include "pcm.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
/* Opcode numbers of the policy library, generated at build time. */
#include "policy_ops.h"

#include "logic/${{ values.policy }}_rules.h"

/* Stock implementation, compiled into the same library from Oracle's source. */
extern void op_${{ values.policy }}(cm_nap_connection_t *connp, int32 opcode, int32 flags,
	pin_flist_t *i_flistp, pin_flist_t **o_flistpp, pin_errbuf_t *ebufp);

void
nerv_op_${{ values.policy }}(cm_nap_connection_t *connp, int32 opcode, int32 flags,
	pin_flist_t *i_flistp, pin_flist_t **o_flistpp, pin_errbuf_t *ebufp)
{
	*o_flistpp = NULL;
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	if (opcode != ${{ values.policyOpcode }}) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_OPCODE, 0,
			0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "nerv_op_${{ values.policy }}: bad opcode", ebufp);
		return;
	}

	/* Flists can hold customer data: log them at debug level only. */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "nerv_op_${{ values.policy }} input flist", i_flistp);

	${{ values.policy }}_rules_before(i_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		goto cleanup;
	}

	op_${{ values.policy }}(connp, opcode, flags, i_flistp, o_flistpp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		goto cleanup;
	}

	${{ values.policy }}_rules_after(i_flistp, *o_flistpp, ebufp);

cleanup:
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "nerv_op_${{ values.policy }} error", ebufp);
		PIN_FLIST_DESTROY_EX(o_flistpp, NULL);
	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "nerv_op_${{ values.policy }} output flist", *o_flistpp);
	}
}
