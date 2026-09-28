/*
 * Registers the opcodes of fm_${{ values.module }} with the Connection Manager.
 * conf/pin.conf.d/fm_${{ values.module }}.conf points the CM at this table.
 *
 * To add an opcode: define its number in include/${{ values.module }}_ops.h,
 * add its handler under src/ops/ and its logic under src/logic/, then add a
 * line to this table.
 */
#include <stdio.h>

#include "pcm.h"
#include "cm_fm.h"

#include "${{ values.module }}_ops.h"

/* clang-format off */
struct cm_fm_config fm_${{ values.module }}_config[] = {
	/* opcode, handler function name (the CM looks it up by name) */
	{ ${{ values.opcodeMacro }},	"op_${{ values.module }}_${{ values.opcodeLower }}" },
	{ 0,	(char *)0 }
};
/* clang-format on */

void *
fm_${{ values.module }}_config_func(void)
{
	return ((void *)(fm_${{ values.module }}_config));
}
