/* native_client/src/trusted/validator/x86/decoder/gen/ncop_expr_node_flag_impl.h
 * THIS FILE IS AUTO_GENERATED DO NOT EDIT.
 *
 * This file was auto-generated by enum_gen.py
 * from file ncop_expr_node_flag.enum
 */

/* Define the corresponding names of NaClExpFlag. */
static const char* const g_NaClExpFlagName[NaClExpFlagEnumSize + 1] = {
  "ExprSet",
  "ExprUsed",
  "ExprAddress",
  "ExprDest",
  "ExprSize8",
  "ExprSize16",
  "ExprSize32",
  "ExprSize64",
  "ExprUnsignedHex",
  "ExprSignedHex",
  "ExprUnsignedInt",
  "ExprSignedInt",
  "ExprImplicit",
  "ExprJumpTarget",
  "ExprDSrCase",
  "ExprESrCase",
  "NaClExpFlagEnumSize"
};

const char* NaClExpFlagName(NaClExpFlag name) {
  return name <= NaClExpFlagEnumSize
    ? g_NaClExpFlagName[name]
    : "NaClExpFlag???";
}
