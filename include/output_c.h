/*
 * Translating parsed Datalog into C code.
 * Nik Sultana, April 2018.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

void emit_c_program(struct TymProgram * ParsedInputFileContents, struct TymProgram * ParsedQuery);
