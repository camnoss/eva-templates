/*
 * Unit tests for src/logic/${{ values.module }}_${{ values.opcodeLower }}.c.
 *
 * They use the libportal flist API directly, so they need the BRM SDK but not
 * a Connection Manager. Run them with `make test`.
 */
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <cmocka.h>

#include "pcm.h"
#include "pin_cust.h"
#include "pin_errs.h"

#include "logic/${{ values.module }}_${{ values.opcodeLower }}.h"

static pin_flist_t *
poid_flist(const char *type, pin_errbuf_t *ebufp)
{
	pin_flist_t *flistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, PIN_POID_CREATE(1, (char *)type, 42, ebufp), ebufp);
	return flistp;
}

static void
test_validate_accepts_account(void **state)
{
	pin_errbuf_t ebuf;
	pin_flist_t *i_flistp = NULL;

	PIN_ERRBUF_CLEAR(&ebuf);
	i_flistp = poid_flist("/account", &ebuf);

	${{ values.module }}_${{ values.opcodeLower }}_validate(i_flistp, &ebuf);

	assert_false(PIN_ERR_IS_ERR(&ebuf));
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
}

static void
test_validate_requires_poid(void **state)
{
	pin_errbuf_t ebuf;
	pin_flist_t *i_flistp = NULL;

	PIN_ERRBUF_CLEAR(&ebuf);
	i_flistp = PIN_FLIST_CREATE(&ebuf);

	${{ values.module }}_${{ values.opcodeLower }}_validate(i_flistp, &ebuf);

	assert_int_equal(ebuf.pin_err, PIN_ERR_MISSING_ARG);
	assert_int_equal(ebuf.field, PIN_FLD_POID);
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
}

static void
test_validate_rejects_other_poid_types(void **state)
{
	pin_errbuf_t ebuf;
	pin_flist_t *i_flistp = NULL;

	PIN_ERRBUF_CLEAR(&ebuf);
	i_flistp = poid_flist("/service", &ebuf);

	${{ values.module }}_${{ values.opcodeLower }}_validate(i_flistp, &ebuf);

	assert_int_equal(ebuf.pin_err, PIN_ERR_BAD_POID_TYPE);
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
}

static void
test_read_flist_asks_for_the_output_fields(void **state)
{
	pin_errbuf_t ebuf;
	pin_flist_t *i_flistp = NULL;
	pin_flist_t *r_flistp = NULL;
	poid_t *i_pdp = NULL;
	poid_t *r_pdp = NULL;

	PIN_ERRBUF_CLEAR(&ebuf);
	i_flistp = poid_flist("/account", &ebuf);

	r_flistp = ${{ values.module }}_${{ values.opcodeLower }}_read_flist(i_flistp, &ebuf);

	assert_false(PIN_ERR_IS_ERR(&ebuf));
	assert_int_equal(PIN_FLIST_COUNT(r_flistp, &ebuf), 3);
	i_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, &ebuf);
	r_pdp = (poid_t *)PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_POID, 0, &ebuf);
	assert_int_equal(PIN_POID_COMPARE(r_pdp, i_pdp, 0, &ebuf), 0);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
}

static void
test_output_copies_the_account_fields(void **state)
{
	pin_errbuf_t ebuf;
	pin_flist_t *read_flistp = NULL;
	pin_flist_t *o_flistp = NULL;
	int32 status = PIN_STATUS_ACTIVE;

	PIN_ERRBUF_CLEAR(&ebuf);
	read_flistp = poid_flist("/account", &ebuf);
	PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_ACCOUNT_NO, (void *)"0.0.0.1-42", &ebuf);
	PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_STATUS, &status, &ebuf);

	o_flistp = ${{ values.module }}_${{ values.opcodeLower }}_output(read_flistp, &ebuf);

	assert_false(PIN_ERR_IS_ERR(&ebuf));
	assert_string_equal(PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_ACCOUNT_NO, 0, &ebuf), "0.0.0.1-42");
	assert_int_equal(*(int32 *)PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_STATUS, 0, &ebuf),
		PIN_STATUS_ACTIVE);
	PIN_FLIST_DESTROY_EX(&o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
}

static void
test_functions_do_nothing_after_an_error(void **state)
{
	pin_errbuf_t ebuf;
	pin_flist_t *i_flistp = NULL;

	PIN_ERRBUF_CLEAR(&ebuf);
	i_flistp = poid_flist("/account", &ebuf);
	pin_set_err(&ebuf, PIN_ERRLOC_FM, PIN_ERRCLASS_APPLICATION, PIN_ERR_BAD_VALUE, 0, 0, 0);

	assert_null(${{ values.module }}_${{ values.opcodeLower }}_read_flist(i_flistp, &ebuf));
	assert_null(${{ values.module }}_${{ values.opcodeLower }}_output(i_flistp, &ebuf));
	assert_int_equal(ebuf.pin_err, PIN_ERR_BAD_VALUE);
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
}

int
main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_validate_accepts_account),
		cmocka_unit_test(test_validate_requires_poid),
		cmocka_unit_test(test_validate_rejects_other_poid_types),
		cmocka_unit_test(test_read_flist_asks_for_the_output_fields),
		cmocka_unit_test(test_output_copies_the_account_fields),
		cmocka_unit_test(test_functions_do_nothing_after_an_error),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
