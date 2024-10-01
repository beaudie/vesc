// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// A helper to validate the rules of IR.  This is useful particularly to be run after
// transformations, to ensure they generate valid IR.
//
// TODO: to validate:
//
//   - Every ID must be present in the respective map.
//   - Every variable must be defined somewhere, either in global block or in a block.
//   - Every accessed variable must be declared in an accessible block.
//   - Branches must have the appropriate targets set (merge, trueblock for if, etc).
//   - No brances inside a block, every block ends in branch. (i.e. no dead code)
//   - For merge blocks that have an input, the branch instruction of blocks that jump to it have an
//     output.
//   - No identical types with different IDs.
//   - If there's a cached "has side effect", that it's correct.
//   - Validate that ImageType fields are valid in combination with ImageDimension
//   - No operations should have entirely constant arguments, that should be folded (and
//     transformations shouldn't retintroduce it)
//   - Catch misuse of builtin names.
//   - Precision is not applied to types that don't are not applicable.  It _is_ applied to types
//     that are applicable (including uniforms and samplers for example).  Needs to work to make
//     sure precision is always assigned.
//   - Case values are always ConstantId (int/uint only too?)
//   - Variables are Pointers
//   - Pointers only valid in the right arg of load/store/access/call
//   - Loop blocks ends in the appropriate instructions.
//   - Do blocks end in DoLoop (unless already terminated by something else, like Return)
//   - If condition is a bool.
//   - Maximum one default case for Switch instructions.
//   - No pointer->pointer type.
//   - Interface variables with NameSource::AngleInternal are unique.
//   - NameSource::AngleInternal names don't start with the user and temporary name prefixes (_u, t
//     and f respectively).
//   - Interface variables with NameSource::ShaderInterface are unique.
//   - NameSource::ShaderInterface and NameSource::AngleInternal are never found inside body
//   - blocks, those should always be Temporary.
//   - No identity swizzles.
//   - Type matches?
//   - Block inputs have MergeInput opcode, nothing else has that opcode.
//   - Block inputs are not pointers.  AST-ifier does not handle that.
//   - Whatever else is in the AST validation currently.
use crate::ir::*;
struct Validator {
    // TODO: stuff the validator needs to track; is there actually anything it needs to track?
}

impl Validator {
    fn new() -> Validator {
        Validator {
            // TODO: default state
        }
    }
    fn validate(&mut self, _ir: &IR) -> bool {
        // TODO
        true
    }
}
