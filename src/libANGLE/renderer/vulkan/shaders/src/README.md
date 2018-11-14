# Shader Variations

To build multiple variations of a shader, add a file named X.json corresponding to shader file X.

There are multiple possible fields in the json file:

- "description": This contains the license and other comments, which will be ignored.
- "output": A suffix for the output file name.
- "flags": this is a list of flags.  Each flag FLAG defines a shader variation with or without the
  define FLAG=1.  Each flag has a corresponding string which will be used to construct the name of the output file.
- other: any other field is a similar list to flags, except that each entry in this enumeration is a variation.  Similar
  to "flags", every variation VARIATION results in a VARIATION=1 define, and the corresponding string used to construct
  the name of the output file.
