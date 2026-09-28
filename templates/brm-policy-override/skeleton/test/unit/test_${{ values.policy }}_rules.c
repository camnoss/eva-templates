/*
 * Unit tests for src/logic/${{ values.policy }}_rules.c.
 *
 * They use the libportal flist API directly, so they need the BRM SDK but not
 * a Connection Manager. Run them with `make test`.
 */
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <cmocka.h>

#include "pcm.h"
#include "pin_errs.h"

#include "logic/${{ values.policy }}_rules.h"

static void
test_before_accepts_input_with_poid(void **state)
{
	pin_errbuf_t ebuf;
	pin_flist_t *i_flistp = NULL;

	PIN_ERRBUF_CLEAR(&ebuf);
	i_flistp = PIN_FLIST_CREATE(&ebuf);
	PIN_FLIST_FLD_PUT(i_flistp, PIN_FLD_POID, PIN_POID_CREATE(1, "/account", 42, &ebuf), &ebuf);

	${{ values.policy }}_rules_before(i_flistp, &ebuf);

	assert_false(PIN_ERR_IS_ERR(&ebuf));
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
}

static void
test_before_rejects_input_without_poid(void **state)
{
	pin_errbuf_t ebuf;
	pin_flist_t *i_flistp = NULL;

	PIN_ERRBUF_CLEAR(&ebuf);
	i_flistp = PIN_FLIST_CREATE(&ebuf);

	${{ values.policy }}_rules_before(i_flistp, &ebuf);

	assert_int_equal(ebuf.pin_err, PIN_ERR_MISSING_ARG);
	assert_int_equal(ebuf.field, PIN_FLD_POID);
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
}

static void
test_after_keeps_the_stock_output(void **state)
{
	pin_errbuf_t ebuf;
	pin_flist_t *i_flistp = NULL;
	pin_flist_t *o_flistp = NULL;

	PIN_ERRBUF_CLEAR(&ebuf);
	i_flistp = PIN_FLIST_CREATE(&ebuf);
	o_flistp = PIN_FLIST_CREATE(&ebuf);
	PIN_FLIST_FLD_PUT(o_flistp, PIN_FLD_POID, PIN_POID_CREATE(1, "/account", 42, &ebuf), &ebuf);

	${{ values.policy }}_rules_after(i_flistp, o_flistp, &ebuf);

	assert_false(PIN_ERR_IS_ERR(&ebuf));
	assert_int_equal(PIN_FLIST_COUNT(o_flistp, &ebuf), 1);
	PIN_FLIST_DESTROY_EX(&o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
}

int
main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_before_accepts_input_with_poid),
		cmocka_unit_test(test_before_rejects_input_without_poid),
		cmocka_unit_test(test_after_keeps_the_stock_output),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
