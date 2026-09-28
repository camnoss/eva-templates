/*
 * Our rules for ${{ values.policyOpcode }}.
 *
 * No PCM calls here, so the unit tests run them without a CM. Every function
 * returns right away if ebufp already holds an error.
 */
#ifndef ${{ values.policyUpper }}_RULES_H
#define ${{ values.policyUpper }}_RULES_H

#include "pcm.h"

/* Runs before the stock policy. Sets ebufp to reject the input. */
void ${{ values.policy }}_rules_before(pin_flist_t *i_flistp, pin_errbuf_t *ebufp);

/* Runs after the stock policy, and may change its output flist. */
void ${{ values.policy }}_rules_after(pin_flist_t *i_flistp, pin_flist_t *o_flistp, pin_errbuf_t *ebufp);

#endif /* ${{ values.policyUpper }}_RULES_H */
