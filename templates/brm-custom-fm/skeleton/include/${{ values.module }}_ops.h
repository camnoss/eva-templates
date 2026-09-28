/*
 * Opcodes of fm_${{ values.module }}.
 *
 * Clients call opcodes by number, so this file is the public contract of the
 * module. Numbers must be unique across every custom FM in BRM: before adding
 * one, search the MAGI catalog for the nerv.io/brm-opcodes annotation, and add
 * the new number to catalog-info.yaml.
 */
#ifndef ${{ values.moduleUpper }}_OPS_H
#define ${{ values.moduleUpper }}_OPS_H

#define ${{ values.opcodeMacro }} ${{ values.opcodeNumber }}

#endif /* ${{ values.moduleUpper }}_OPS_H */
