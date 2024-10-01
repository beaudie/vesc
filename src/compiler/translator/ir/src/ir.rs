// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// The IR itself, consisting of a number of enums and structs.
// TODO: write a doc with an overview of the IR.

use super::instruction;
use std::collections::HashMap;

// Strong types for ids that refer to constants, registers, variables, types etc.  They are used to
// look information up in different tables.  In all cases, 0 means no applicable ID.
#[derive(Eq, PartialEq, Hash, Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub struct RegisterId {
    pub id: u32,
}

#[derive(Eq, PartialEq, Hash, Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub struct ConstantId {
    pub id: u32,
}

#[derive(Eq, PartialEq, Hash, Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub struct VariableId {
    pub id: u32,
}

#[derive(Eq, PartialEq, Hash, Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub struct FunctionId {
    pub id: u32,
}

#[derive(Eq, PartialEq, Hash, Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub struct TypeId {
    pub id: u32,
}

// Fixed enums for faster type lookup
pub const TYPE_ID_VOID: TypeId = TypeId { id: 0 };
pub const TYPE_ID_FLOAT: TypeId = TypeId { id: 1 };
pub const TYPE_ID_INT: TypeId = TypeId { id: 2 };
pub const TYPE_ID_UINT: TypeId = TypeId { id: 3 };
pub const TYPE_ID_BOOL: TypeId = TypeId { id: 4 };
pub const TYPE_ID_ATOMIC_COUNTER: TypeId = TypeId { id: 5 };
pub const TYPE_ID_YUV_CSC_STANDARD: TypeId = TypeId { id: 6 };
// Note: vector type ids must be consecutive
pub const TYPE_ID_VEC2: TypeId = TypeId { id: 7 };
pub const TYPE_ID_VEC3: TypeId = TypeId { id: 8 };
pub const TYPE_ID_VEC4: TypeId = TypeId { id: 9 };
pub const TYPE_ID_IVEC2: TypeId = TypeId { id: 10 };
pub const TYPE_ID_IVEC3: TypeId = TypeId { id: 11 };
pub const TYPE_ID_IVEC4: TypeId = TypeId { id: 12 };
pub const TYPE_ID_UVEC2: TypeId = TypeId { id: 13 };
pub const TYPE_ID_UVEC3: TypeId = TypeId { id: 14 };
pub const TYPE_ID_UVEC4: TypeId = TypeId { id: 15 };
pub const TYPE_ID_BVEC2: TypeId = TypeId { id: 16 };
pub const TYPE_ID_BVEC3: TypeId = TypeId { id: 17 };
pub const TYPE_ID_BVEC4: TypeId = TypeId { id: 18 };
// Note: matrix type ids must be consecutive in rows, then columns
pub const TYPE_ID_MAT2: TypeId = TypeId { id: 19 };
pub const TYPE_ID_MAT2X3: TypeId = TypeId { id: 20 };
pub const TYPE_ID_MAT2X4: TypeId = TypeId { id: 21 };
pub const TYPE_ID_MAT3X2: TypeId = TypeId { id: 22 };
pub const TYPE_ID_MAT3: TypeId = TypeId { id: 23 };
pub const TYPE_ID_MAT3X4: TypeId = TypeId { id: 24 };
pub const TYPE_ID_MAT4X2: TypeId = TypeId { id: 25 };
pub const TYPE_ID_MAT4X3: TypeId = TypeId { id: 26 };
pub const TYPE_ID_MAT4: TypeId = TypeId { id: 27 };

// Fixed enums for bool constants to avoid tracking whether they are defined or not, plus other
// constants for convenience.
pub const CONSTANT_ID_FALSE: ConstantId = ConstantId { id: 0 };
pub const CONSTANT_ID_TRUE: ConstantId = ConstantId { id: 1 };
pub const CONSTANT_ID_FLOAT_ZERO: ConstantId = ConstantId { id: 2 };
pub const CONSTANT_ID_FLOAT_ONE: ConstantId = ConstantId { id: 3 };
pub const CONSTANT_ID_INT_ZERO: ConstantId = ConstantId { id: 4 };
pub const CONSTANT_ID_INT_ONE: ConstantId = ConstantId { id: 5 };
pub const CONSTANT_ID_UINT_ZERO: ConstantId = ConstantId { id: 6 };
pub const CONSTANT_ID_UINT_ONE: ConstantId = ConstantId { id: 7 };
pub const CONSTANT_ID_YUV_CSC_ITU601: ConstantId = ConstantId { id: 8 };
pub const CONSTANT_ID_YUV_CSC_ITU601_FULL_RANGE: ConstantId = ConstantId { id: 9 };
pub const CONSTANT_ID_YUV_CSC_ITU709: ConstantId = ConstantId { id: 10 };

// Prefixes used for symbols
pub const USER_SYMBOL_PREFIX: &str = "_u";
pub const TEMP_VARIABLE_PREFIX: &str = "t";
pub const TEMP_FUNCTION_PREFIX: &str = "f";
pub const TEMP_IMPOSSIBLE_PREFIX: &str = "<internal error>";

// An ID that can be referred to by an operand of an instruction.
#[derive(Copy, Clone, PartialEq)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum Id {
    Register(RegisterId),
    Constant(ConstantId),
    Variable(VariableId),
}

impl Id {
    pub fn new_register(id: RegisterId) -> Id {
        Id::Register(id)
    }
    pub fn new_constant(id: ConstantId) -> Id {
        Id::Constant(id)
    }
    pub fn new_variable(id: VariableId) -> Id {
        Id::Variable(id)
    }

    pub fn get_register(&self) -> RegisterId {
        match self {
            &Id::Register(id) => id,
            _ => {
                panic!("Internal error: unexpected non-register id");
            }
        }
    }

    pub fn get_constant(&self) -> Option<ConstantId> {
        match self {
            &Id::Constant(id) => Some(id),
            _ => None,
        }
    }
}

#[derive(Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub struct TypedId {
    pub id: Id,
    pub type_id: TypeId,
    pub precision: Precision,
}

impl TypedId {
    pub fn new(id: Id, type_id: TypeId, precision: Precision) -> TypedId {
        TypedId { id, type_id, precision }
    }

    pub fn from_constant_id(id: ConstantId, type_id: TypeId) -> TypedId {
        TypedId { id: Id::new_constant(id), type_id, precision: Precision::NotApplicable }
    }

    pub fn from_register_id(register_id: TypedRegisterId) -> TypedId {
        TypedId {
            id: Id::new_register(register_id.id),
            type_id: register_id.type_id,
            precision: register_id.precision,
        }
    }

    pub fn to_register_id(&self) -> TypedRegisterId {
        TypedRegisterId {
            id: self.id.get_register(),
            type_id: self.type_id,
            precision: self.precision,
        }
    }

    pub fn from_bool_variable_id(id: VariableId) -> TypedId {
        Self::new(Id::new_variable(id), TYPE_ID_BOOL, Precision::NotApplicable)
    }

    pub fn from_variable_id(ir_meta: &IRMeta, id: VariableId) -> TypedId {
        let variable = ir_meta.get_variable(id);
        Self::new(Id::new_variable(id), variable.type_id, variable.precision)
    }

    // Replace id with whatever it is aliasing (if any).  The precision is retained, and the type
    // should match the alias already.
    pub fn dealias(&mut self, alias_map: &HashMap<RegisterId, Id>) {
        if let Id::Register(register_id) = self.id {
            alias_map.get(&register_id).inspect(|&mapped| {
                self.id = *mapped;
            });
        }
    }
}

#[derive(Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum UnaryOpCode {
    // Get the array length of a pointer.  The result is an `int` per GLSL.
    //   %result = ArrayLength %ptr
    ArrayLength,

    // Calculate -operand.
    //   %result = Negate %operand
    Negate,

    // Calculate !operand.
    //   %result = LogicalNot %operand
    LogicalNot,

    // Calculate ~operand
    //   %result = BitwiseNot %operand
    BitwiseNot,
}

#[derive(Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum BinaryOpCode {
    // Calculate operand1 + operand2.
    //   %result = Add %operand1 %operand2
    Add,
    // Calculate operand1 - operand2.
    //   %result = Sub %operand1 %operand2
    Sub,
    // Calculate operand1 * operand2.
    //   %result = Mul %operand1 %operand2
    Mul,
    // Calculate operand1 * operand2, where operand1 is a float vector and operand2 is a scalar.
    //   %result = VectorTimesScalar %operand1 %operand2
    VectorTimesScalar,
    // Calculate operand1 * operand2, where operand1 is a matrix and operand2 is a scalar.
    //   %result = MatrixTimesScalar %operand1 %operand2
    MatrixTimesScalar,
    // Calculate operand1 * operand2, where operand1 is a vector and operand2 is a matrix.
    //   %result = VectorTimesMatrix %operand1 %operand2
    VectorTimesMatrix,
    // Calculate operand1 * operand2, where operand1 is a matrix and operand2 is a vector.
    //   %result = MatrixTimesVector %operand1 %operand2
    MatrixTimesVector,
    // Calculate operand1 * operand2, where operand1 is a matrix and operand2 is a matrix.
    //   %result = MatrixTimesMatrix %operand1 %operand2
    MatrixTimesMatrix,

    // Calculate operand1 / operand2.
    //   %result = Div %operand1 %operand2
    Div,
    // Calculate operand1 % operand2.
    //   %result = IMod %operand1 %operand2
    IMod,

    // Calculate operand1 ^^ operand2.
    //   %result = LogicalXor %operand1 %operand2
    LogicalXor,

    // Calculate operand1 == operand2.
    //   %result = Equal %operand1 %operand2
    Equal,
    // Calculate operand1 != operand2.
    //   %result = NotEqual %operand1 %operand2
    NotEqual,
    // Calculate operand1 < operand2.
    //   %result = LessThan %operand1 %operand2
    LessThan,
    // Calculate operand1 > operand2.
    //   %result = GreaterThan %operand1 %operand2
    GreaterThan,
    // Calculate operand1 <= operand2.
    //   %result = LessThanEqual %operand1 %operand2
    LessThanEqual,
    // Calculate operand1 >= operand2.
    //   %result = GreaterThanEqual %operand1 %operand2
    GreaterThanEqual,

    // Calculate operand1 << operand2.
    //   %result = BitShiftLeft %operand1 %operand2
    BitShiftLeft,
    // Calculate operand1 >> operand2.
    //   %result = BitShiftRight %operand1 %operand2
    BitShiftRight,
    // Calculate operand1 | operand2.
    //   %result = BitwiseOr %operand1 %operand2
    BitwiseOr,
    // Calculate operand1 ^ operand2.
    //   %result = BitwiseXor %operand1 %operand2
    BitwiseXor,
    // Calculate operand1 & operand2.
    //   %result = BitwiseAnd %operand1 %operand2
    BitwiseAnd,
}

// TODO: TextureOpCode for the various texture* call variants?

// The opcode of an instruction together with its possible operands.  The `result`, if applicable,
// is always specified in the `Instruction` struct.
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum OpCode {
    // A special op to indicate that a register is an input to a merge block.  Not to be used
    // directly in blocks, this instruction is a marker for the global list of instructions in
    // `IRMeta::instructions`.
    MergeInput,

    // Discard the fragment (fragment shader only).
    //   Discard
    Discard,
    // Return from the function, optionally with one value.
    //   Return [%value]
    Return(Option<TypedId>),
    // Break from a loop or switch.
    //   Break
    Break,
    // Continue a loop.
    //   Continue
    Continue,
    // Passthrough from one case label to the next.
    //   Passthrough
    Passthrough,
    // Jump to next block, effectively concatenating it with the current block.
    //   NextBlock
    NextBlock,
    // Jump to merge block, optionally with one value.
    //   Merge [%value]
    Merge(Option<TypedId>),
    // Jump to a branch of if/else.
    //   If %condition
    If(TypedId),
    // Marking the beginning of a loop, jumps to the block that evaluates the condition of the
    // loop (which should terminate in a `LoopIf`).  If the condition block's `LoopIf` condition
    // is true, that block jumps to the body of loop or the merge block otherwise.  For and
    // while loops can be distinguished by the presence of a "continue" block in Block.  The
    // body of the loop itself must terminate with `Continue` (if not otherwise terminated
    // with `Break`, `Return` etc).  The continue block itself, if any, should also terminate
    // with `Continue`, though that's immaterial (as there cannot be any other terminator).
    //   Loop
    Loop,
    // Similarly to `Loop`, marks the beginning of a do-loop.  Unlike `Loop`, the initial jump is
    // to the body of the do-while loop.  The body and condition blocks are similar to `Loop`.
    //   DoLoop
    DoLoop,
    // Jump to beginning of the body of the loop if the condition is true.
    //   LoopIf %condition
    LoopIf(TypedId),
    // Switch based on a condition and jump to any number of blocks based on matching values.  The
    // None value indicates the default block.
    //   Switch %condition %case_values...
    Switch(TypedId, Vec<Option<ConstantId>>),

    // Extract a component of a vector into a scalar.
    //   %result = ExtractVectorComponent %vector %index
    ExtractVectorComponent(TypedId, u32),
    // Extract multiple components of a vector into another vector.
    //   %result = ExtractVectorComponentMulti %vector %indices...
    ExtractVectorComponentMulti(TypedId, Vec<u32>),
    // Extract a component of a vector dynamically.
    //   %result = ExtractVectorComponentDynamic %vector %index
    ExtractVectorComponentDynamic(TypedId, TypedId),
    // Extract a column of a matrix into a vector.
    //   %result = ExtractMatrixColumn %matrix %column
    ExtractMatrixColumn(TypedId, TypedId),
    // Extract a field of a struct.
    //   %result = ExtractStructField %struct %field_index
    ExtractStructField(TypedId, u32),
    // Extract an element of an array.
    //   %result = ExtractArrayElement %array %element_index
    ExtractArrayElement(TypedId, TypedId),

    // Given a vector pointer, create an access to a scalar component.
    //   %result = AccessVectorComponent %vector_ptr %index
    AccessVectorComponent(TypedId, u32),
    // Access multiple components of a vector into another vector.
    //   %result = AccessVectorComponentMulti %vector_ptr %indices...
    AccessVectorComponentMulti(TypedId, Vec<u32>),
    // Access a component of a vector dynamically.
    //   %result = AccessVectorComponentDynamic %vector_ptr %index
    AccessVectorComponentDynamic(TypedId, TypedId),
    // Access a column of a matrix into a vector.
    //   %result = AccessMatrixColumn %matrix_ptr %column
    AccessMatrixColumn(TypedId, TypedId),
    // Access a field of a struct.
    //   %result = AccessStructField %struct_ptr %field_index
    AccessStructField(TypedId, u32),
    // Access an element of an array.
    //   %result = AccessArrayElement %array_ptr %element_index
    AccessArrayElement(TypedId, TypedId),

    // Given a scalar, construct a scalar with a different type (effectively a cast).
    //   %result = ConstructScalarFromScalar %value
    ConstructScalarFromScalar(TypedId),
    // Given a scalar, construct a vector by replicating it.  The basic type is cast if
    // necesseray.   %result = ConstructVectorFromScalar %value
    ConstructVectorFromScalar(TypedId),
    // Given a scalar, construct a matrix where every element is 0 except on the diagonal, which
    // is the given scalar.  The basic type is cast if necessary.
    //   %result = ConstructMatrixFromScalar %value
    ConstructMatrixFromScalar(TypedId),
    // Given a matrix of size AxB, construct a matrix of size NxM as such:
    //
    // * Take C to be the minimum of A and N
    // * Take R to be ghe minimum of B and M
    // * The resulting matrix is the identity matrix of size NxM with the CxR submatrix of the
    //   input matrix superimposed.
    //
    // In effect, this means that if the result matrix is smaller than the input matrix, the
    // result is a submatrix of the input.  If it is larger, it is padded with 1s on the
    // diagonal and 0s elsewhere.
    //   %result = ConstructMatrixFromMatrix %value
    ConstructMatrixFromMatrix(TypedId),
    // Given multiple values, construct a vector/matrix by piecing together components from these
    // values.  The total number of components in the values must match the number of components
    // in the resulting vector.  The basic type is cast if necesseray.
    //   %result = ConstructVectorFromMultiple %values...
    //   %result = ConstructMatrixFromMultiple %values...
    ConstructVectorFromMultiple(Vec<TypedId>),
    ConstructMatrixFromMultiple(Vec<TypedId>),
    // Given values for each field of a struct, construct a value of that struct type.
    //   %result = ConstructStruct %fields...
    ConstructStruct(Vec<TypedId>),
    // Given values for each element of an array, construct a value of that array type.
    //   %result = ConstructArray %fields...
    ConstructArray(Vec<TypedId>),

    // Load from a pointer.
    //   %result = Load %ptr
    Load(TypedId),
    // Assign to a pointer.
    //   Store %ptr %value
    Store(TypedId, TypedId),
    // Create a register alias for another id.  Produced by transformations, makes it possible to
    // make an existing register id use a different value (such as a constant, or another
    // pre-existing register id).
    //   Alias %new %old
    Alias(TypedId),

    // Call a function, optionaly returning a result.
    //   [%result =] Call %function %operands...
    Call(FunctionId, Vec<TypedId>),

    // TODO: maybe one op for each internal function call?  Won't be needed for SPIR-V, but maybe
    // for others.  Ideally, they should all be converted to something that can be represented in
    // the IR instead, but basically they could be added here as some form of built-in.

    // Common arithmetic, logical and bitwise operations as well as most built-ins are bundled
    // under Unary and Binary operations.  This makes it easier to insoect the ids of an
    // instruction by reducing the need for numerous `match` arms.
    Unary(UnaryOpCode, TypedId),
    Binary(BinaryOpCode, TypedId, TypedId),
    // TODO something for textures + other built-ins.  See if Ternary makes sense, or Nary.  Maybe
    // just a BuiltIn(BuiltInOp, Vec<TypeId>) and be done with it.

    // TODO: autogen the built-ins?  Kind of stable at this point, so might as well just
    // hand-write them once.  Unimplemented for now.
}

impl OpCode {
    pub fn is_branch(&self) -> bool {
        matches!(
            *self,
            OpCode::Discard
                | OpCode::Return(_)
                | OpCode::Break
                | OpCode::Continue
                | OpCode::Passthrough
                | OpCode::NextBlock
                | OpCode::Merge(_)
                | OpCode::If(_)
                | OpCode::Loop
                | OpCode::DoLoop
                | OpCode::LoopIf(_)
                | OpCode::Switch(_, _)
        )
    }

    pub fn get_if_condition(&self) -> TypedId {
        match self {
            &OpCode::If(id) => id,
            _ => panic!("Internal error: Expected if"),
        }
    }
    pub fn get_loop_condition(&self) -> TypedId {
        match self {
            &OpCode::LoopIf(id) => id,
            _ => panic!("Internal error: Expected loop condition"),
        }
    }
    pub fn get_merge_parameter(&self) -> Option<TypedId> {
        match self {
            &OpCode::Merge(id) => id,
            _ => panic!("Internal error: Expected merge"),
        }
    }
    pub fn get_switch_expression_and_cases(&self) -> (TypedId, &Vec<Option<ConstantId>>) {
        match self {
            OpCode::Switch(id, cases) => (*id, cases),
            _ => panic!("Internal error: Expected switch"),
        }
    }
    pub fn add_switch_case(&mut self, value: Option<ConstantId>) {
        match self {
            OpCode::Switch(_, values) => values.push(value),
            _ => panic!("Internal error: Expected switch"),
        };
    }
}

#[derive(Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub struct TypedRegisterId {
    pub id: RegisterId,
    pub type_id: TypeId,
    pub precision: Precision,
}

impl TypedRegisterId {
    pub fn new(id: RegisterId, type_id: TypeId, precision: Precision) -> TypedRegisterId {
        TypedRegisterId { id, type_id, precision }
    }
}

// Representation of an IR instruction.  For example:
//
//     %result = Operation %operand1 ...
//
// Most instructions have a result, but some don't (notably Store, void function calls and
// control-flow instructions).  The instructions that don't have a result are directly stored in
// the blocks, but for convenience during vists and transformations, the instructions that have a
// result are indirectly referenced by the blocks, referenced by their result ID.
//
// The `Instruction` type associates a result ID with the instruction (that is otherwise just the
// `OpCode`).
//
// Associated with each result ID is a set of information, such as its type and precision.
#[cfg_attr(debug_assertions, derive(Debug))]
pub struct Instruction {
    // The instruction opcode itself.
    pub op: OpCode,
    // The `result` of the operation.  This is always a temporary (register) value.
    pub result: TypedRegisterId,
}

impl Instruction {
    pub fn new(op: OpCode, result: TypedRegisterId) -> Instruction {
        Instruction { op, result }
    }
}

// The entities in a block of the IR.  They are either directly the opcodes themselves (if they
// don't have a result), or an indirect reference to `IRMeta::instructions` by the ID of the result
// they produce.
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum BlockInstruction {
    Void(OpCode),
    Register(RegisterId),
}

impl BlockInstruction {
    pub fn new_void(op: OpCode) -> BlockInstruction {
        BlockInstruction::Void(op)
    }

    pub fn new_control_flow(op: OpCode) -> BlockInstruction {
        BlockInstruction::Void(op)
    }

    pub fn new_typed(id: RegisterId) -> BlockInstruction {
        BlockInstruction::Register(id)
    }

    pub fn is_branch(&self) -> bool {
        match self {
            BlockInstruction::Void(op) => op.is_branch(),
            _ => false,
        }
    }

    // Convenience function to get the OpCode of the instruction no matter if Void or Register.
    // The result (if available) is simultaneously returned.
    pub fn get_op_and_result<'block, 'ir: 'block>(
        &'block self,
        ir: &'ir IRMeta,
    ) -> (&'block OpCode, Option<TypedRegisterId>) {
        match self {
            BlockInstruction::Void(op) => (op, None),
            &BlockInstruction::Register(id) => {
                let instruction = ir.get_instruction(id);
                (&instruction.op, Some(instruction.result))
            }
        }
    }
}

// Representation of a block of instructions.  There are no branch instructions inside a block, and
// every block ends with a branch instruction (if, for, return, etc).
//
// Every block may have an input
#[cfg_attr(debug_assertions, derive(Debug))]
pub struct Block {
    // Some compilers do a bad job of register allocation, so the ANGLE IR supports scoping
    // variables to blocks.  This information is solely used during code generation for the purpose
    // of declaring variables in the right scope.
    pub variables: Vec<VariableId>,

    // The merge blocks may have an input, which is a more modern way of keeping the IR in SSA
    // form compared with Phi instructions (and is easier to generate text with).  ANGLE's IR uses
    // variables (like SPIR-V) and only ever really uses one parameter to implement SSA form.  This
    // could be avoided with temp variables, but having this support makes it easier to translate
    // ?: or short-circuiting && or ||.  ANGLE never produces blocks with more than one input.
    pub input: Option<TypedRegisterId>,

    // The instructions inside the block, the last one is always a branch instruction.
    pub instructions: Vec<BlockInstruction>,

    // The IR has the following branch instructions:
    //
    // - Discard: Terminates the current invocation of the fragment shader.
    // - Return: Returns execution to the caller of this function, or terminates the shader if in
    //   `main()`.
    // - Break: Break out of a loop or a switch case.
    //   * Branch is done to the `merge_block` of the innermost loop or switch block.
    // - Continue: Continue to the beginning of the loop.
    //   * If the loop has a continue block, branch is done to `block2` of the innermost loop
    //     block.
    //   * If this is the continue block itself, or if there is no continue block in a loop, branch
    //     is done to the parent of the innermost loop block.  That will always be the block that
    //     evaluates the loop condition.
    //   * If this is part of a do-loop, branch is done to the innermost do block.
    // - Passthrough: Pass through from one switch case to another.
    //   * If this is `case_blocks[i]` in the innermost switch block, branch to `case_blocks[i+1]`
    //     of that block.
    // - NextBlock: Continue to the next block of execution.  This is useful to represent blocks
    //   that are split for various reasons; for example the condition block of a for loop that is
    //   separated from the instructions before it.
    //   * Branch is done to the `merge_block` of this block.
    // - Merge: Branch to the merge block of the innermost control flow structure (if, loop, etc).
    // - If: Based on condition:
    //   * Branch to `block1` if the condition is true.
    //   * Branch to `block2` if the condition is false.
    // - Loop:
    //   * Branch to `loop_condition` first, which terminates in a `LoopIf`.
    //   * Branch to `block1` if the condition is true.
    //   * Branch to `merge_block` if the condition is false.
    //   * If there is a continue block, it is found in `block2`.
    // - DoLoop: Identical to `Loop`, except branches to `block1` instead of `loop_condition`.
    // - Switch: Given case values in `case_blocks`, where `None` denotes the `default` case, based
    //   on given expression:
    //   * Branch to `merge_block` if no matching cases and no default case exists.
    //   * Branch to `case_blocks[i]` if no matching cases and case expression `i` is `None`.
    //   * Branch to `case_blocks[i]` if case expression `i` is equal to the switch expression.
    pub merge_block: Option<Box<Block>>,
    // `loop_condition` is the block that evaluates the loop condition.
    pub loop_condition: Option<Box<Block>>,
    // `block1` is used for true block of `if` and body of loops.
    pub block1: Option<Box<Block>>,
    // `block2` is used for false block of `if` and continue block of `forpub `.
    pub block2: Option<Box<Block>>,
    // `case_blocks` contain the case blocks of `switch`.
    pub case_blocks: Vec<Block>,
}

impl Block {
    pub fn new() -> Block {
        Block {
            variables: Vec::new(),
            input: None,
            instructions: Vec::with_capacity(8),
            merge_block: None,
            loop_condition: None,
            block1: None,
            block2: None,
            case_blocks: Vec::new(),
        }
    }

    // Helpers used to create a function, one block at a time.  Moves the intermediary block into a
    // child block.
    pub fn set_merge_block(&mut self, block: Block) {
        debug_assert!(self.merge_block.is_none());
        self.merge_block = Some(Box::new(block));
    }

    pub fn set_sub_block1(&mut self, block: Block) {
        debug_assert!(self.block1.is_none());
        self.block1 = Some(Box::new(block));
    }

    pub fn set_sub_block2(&mut self, block: Block) {
        debug_assert!(self.block2.is_none());
        self.block2 = Some(Box::new(block));
    }

    pub fn set_if_true_block(&mut self, block: Block) {
        self.set_sub_block1(block);
    }

    pub fn set_if_false_block(&mut self, block: Block) {
        self.set_sub_block2(block);
    }

    pub fn set_loop_condition_block(&mut self, block: Block) {
        debug_assert!(self.loop_condition.is_none());
        self.loop_condition = Some(Box::new(block));
    }

    pub fn set_loop_body_block(&mut self, block: Block) {
        self.set_sub_block1(block);
    }

    pub fn set_loop_continue_block(&mut self, block: Block) {
        self.set_sub_block2(block);
    }

    pub fn set_switch_case_blocks(&mut self, blocks: Vec<Block>) {
        debug_assert!(self.case_blocks.is_empty());
        self.case_blocks = blocks;
    }

    pub fn add_switch_case_block(&mut self, block: Block) {
        self.case_blocks.push(block);
    }

    pub fn set_sub_blocks(
        &mut self,
        loop_condition_block: Option<Block>,
        block1: Option<Block>,
        block2: Option<Block>,
        case_blocks: Vec<Block>,
    ) {
        if let Some(block) = loop_condition_block {
            self.set_loop_condition_block(block);
        }
        if let Some(block) = block1 {
            self.set_sub_block1(block);
        }
        if let Some(block) = block2 {
            self.set_sub_block2(block);
        }
        self.set_switch_case_blocks(case_blocks);
    }

    // Takes a chain of blocks, and sets them as the merge chain of the current block.  If the
    // current block has a merge block of its own, it is set as the merge block of the given
    // chain.  Effectively, this inserts the given block right after this one.
    //
    // It returns the end of the _input_ merge chain after it is chained to the current block,
    // letting the caller continue to the rest of the original merge chain (which can be found in
    // the `merge_block` of the return value).
    pub fn insert_into_merge_chain(&mut self, block: Block) -> &mut Block {
        let original_merge_block = self.merge_block.take();
        self.set_merge_block(block);

        let result = self.get_merge_chain_last_block_mut();
        result.merge_block = original_merge_block;
        result
    }

    pub fn add_variable_declaration(&mut self, variable_id: VariableId) {
        self.variables.push(variable_id);
    }

    pub fn add_void_instruction(&mut self, op: OpCode) {
        debug_assert!(!self.is_terminated());
        self.instructions.push(BlockInstruction::new_void(op));
    }
    pub fn add_typed_instruction(&mut self, id: RegisterId) {
        debug_assert!(!self.is_terminated());
        self.instructions.push(BlockInstruction::new_typed(id));
    }
    pub fn add_instruction(&mut self, inst: instruction::Result) {
        // Helper to add instruction created with the `instruction` helpers.
        match inst {
            instruction::Result::Void(op) => self.instructions.push(BlockInstruction::new_void(op)),
            instruction::Result::Register(id) => {
                self.instructions.push(BlockInstruction::new_typed(id.id))
            }
            _ => panic!("Internal error: Unexpected constant or no-op instruction"),
        }
    }
    pub fn prepend_instruction(&mut self, inst: instruction::Result) {
        // Special case of prepending an instruction to the list of instructions.  Used in rare
        // occasions, in particular to replace the merge block input with a variable load
        // instruction for AST-ification.
        self.add_instruction(inst);
        self.instructions.rotate_right(1);
    }

    // Whether the block is already terminated.  This is used for assertions, but also ensures that
    // dead code after return/break/continue/etc is dropped automatically.
    pub fn is_terminated(&self) -> bool {
        self.instructions
            .last()
            .map(|instruction| match instruction {
                BlockInstruction::Void(op) => op.is_branch(),
                _ => false,
            })
            .unwrap_or(false)
    }

    pub fn terminate(&mut self, op: OpCode) {
        self.add_void_instruction(op);
    }
    pub fn unterminate(&mut self) {
        debug_assert!(self.is_terminated());
        self.instructions.pop();
    }

    // Return the branch op found at the end of this block.
    pub fn get_terminating_op(&self) -> &OpCode {
        match self.instructions.last().unwrap() {
            BlockInstruction::Void(op) => op,
            _ => panic!("Internal error: Expected terminated block"),
        }
    }
    pub fn get_terminating_op_mut(&mut self) -> &mut OpCode {
        match self.instructions.last_mut().unwrap() {
            BlockInstruction::Void(op) => op,
            _ => panic!("Internal error: Expected terminated block"),
        }
    }

    pub fn get_merge_chain_last_block(&self) -> &Block {
        let mut last_block = self;
        while let Some(ref merge_block) = last_block.merge_block {
            last_block = merge_block;
        }
        last_block
    }

    pub fn get_merge_chain_last_block_mut(&mut self) -> &mut Block {
        let mut last_block = self;
        while let Some(ref mut merge_block) = last_block.merge_block {
            last_block = merge_block;
        }
        last_block
    }

    // Return the branch op found at the end of the merge chain of this block.  Useful for sub
    // blocks of another block, for example to retrieve the terminating op of the loop condition.
    pub fn get_merge_chain_terminating_op(&self) -> &OpCode {
        self.get_merge_chain_last_block().get_terminating_op()
    }
}

#[derive(Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum YuvCscStandard {
    Itu601,
    Itu601FullRange,
    Itu709,
}

// A constant is either a single value (float, int, uint or bool) or is a composite of other
// constant values.  The type of the composite is specified in `Constant::type_id`.
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum ConstantValue {
    Float(f32),
    Int(i32),
    Uint(u32),
    Bool(bool),
    YuvCsc(YuvCscStandard),
    Composite(Vec<ConstantId>),
}

impl ConstantValue {
    pub fn get_index(&self) -> u32 {
        match self {
            &ConstantValue::Int(i) => i as u32,
            &ConstantValue::Uint(u) => u,
            _ => panic!("Internal error: Expected an index constant"),
        }
    }

    pub fn get_composite_elements(&self) -> &Vec<ConstantId> {
        match self {
            ConstantValue::Composite(ids) => ids,
            _ => panic!("Internal error: Attempt to query elements of a non-composite type"),
        }
    }
}

// A constant value.  Constant values are of a given type, and can composite accordingly.
#[cfg_attr(debug_assertions, derive(Debug))]
pub struct Constant {
    pub type_id: TypeId,
    pub value: ConstantValue,
}

impl Constant {
    pub fn new_bool(value: bool) -> Constant {
        Constant { type_id: TYPE_ID_BOOL, value: ConstantValue::Bool(value) }
    }
    pub fn new_int(value: i32) -> Constant {
        Constant { type_id: TYPE_ID_INT, value: ConstantValue::Int(value) }
    }
    pub fn new_uint(value: u32) -> Constant {
        Constant { type_id: TYPE_ID_UINT, value: ConstantValue::Uint(value) }
    }
    pub fn new_float(value: f32) -> Constant {
        Constant { type_id: TYPE_ID_FLOAT, value: ConstantValue::Float(value) }
    }
    pub fn new_yuv_csc(value: YuvCscStandard) -> Constant {
        Constant { type_id: TYPE_ID_YUV_CSC_STANDARD, value: ConstantValue::YuvCsc(value) }
    }
    pub fn new_composite(type_id: TypeId, params: Vec<ConstantId>) -> Constant {
        Constant { type_id, value: ConstantValue::Composite(params) }
    }
}

// Where a name came from.  This affects how it is output.
#[derive(Copy, Clone, PartialEq)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum NameSource {
    // A name in the shader itself, which corresponds to an interface variable (input, output,
    // etc).  The backend may rely on this name to be output with a certain prefix.
    ShaderInterface,
    // A name that must be output exactly, because:
    //
    // - It's an ANGLE-internal name, which should not collide with a shader name.
    // - It's `main`.
    AngleInternal,
    // A name that can freely be changed because it's internal to the shader.  The output
    // generator may try its best to retain it as-is, but without guarantees.
    Temporary,
}

// A name associated with a variable, struct, struct field etc.
#[derive(Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub struct Name {
    // This is a slice into the shader source, a static name, or otherwise an empty string.  Either
    // way (including when it's received as a string from FFI), it lives for the duration of the
    // compilation, and so is considered 'static.
    // TODO: use [u8] instead of str, Rust should never really access this, and having Unicode in
    // the mix can only be a source of annoyance.
    pub name: &'static str,
    // Whether the name has any significance other than being debug info.  For example, it may be a
    // name that needs to be output exactly in text because the backend/driver looks for it.
    pub source: NameSource,
}

impl Name {
    // Create a temp name, it is not important if the name is exactly preserved in the output (for
    // example, it's a temporary helper variable etc).  Duplicate names are allowed, and will be
    // made distinguishable if generating text.  Temp names are optional, and might as well be "".
    pub fn new_temp(name: &'static str) -> Name {
        Name { name, source: NameSource::Temporary }
    }
    // A name that must be preserved in some predictable form in the output.  This is useful for
    // backends that reference this name directly, such as with OpenGL.
    pub fn new_interface(name: &'static str) -> Name {
        Name { name, source: NameSource::ShaderInterface }
    }
    // A name that must be preserved exactly in the output, for example `main`, or ANGLE internal
    // interface variables.
    pub fn new_exact(name: &'static str) -> Name {
        Name { name, source: NameSource::AngleInternal }
    }
}

// Information surrounding a variable.
#[cfg_attr(debug_assertions, derive(Debug))]
pub struct Variable {
    pub name: Name,
    pub type_id: TypeId,
    pub precision: Precision,
    pub decorations: Decorations,
    pub built_in: Option<BuiltIn>,
    pub initializer: Option<ConstantId>,
}

impl Variable {
    pub fn new(
        name: Name,
        type_id: TypeId,
        precision: Precision,
        decorations: Decorations,
        built_in: Option<BuiltIn>,
        initializer: Option<ConstantId>,
    ) -> Variable {
        Variable { name, type_id, precision, decorations, built_in, initializer }
    }
}

#[derive(Copy, Clone, PartialEq)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum BuiltIn {
    // The BuiltIn enum value X corresponds to gl_X in GLSL.
    InstaceID,
    VertexID,
    Position,
    PointSize,
    DrawID,
    FragCoord,
    FrontFacing,
    PointCoored,
    HelperInvocation,
    FragColor,
    FragData,
    FragDepth,
    SecondaryFragColorEXT,
    SecondaryFragDataEXT,
    ViewIDOVR,
    ViewportIndex,
    ClipDistance,
    CullDistance,
    LastFragColor,
    LastFragData,
    SampleID,
    SamplePosition,
    SampleMaskIn,
    SampleMask,
    NumSamples,
    NumWorkGroups,
    WorkGroupSize,
    WorkGroupID,
    LocalInvaocationID,
    GlobalInvocationID,
    LocalInvocationIndex,
    PerVertexIn,
    PerVertexOut,
    PrimitiveIDIn,
    InvocationID,
    PrimitiveID,
    // gl_Layer as GS output
    LayerOut,
    // gl_Layer as FS input
    LayerIn,
    BoundingBoxOES,
    PixelLocalEXT,
}

// Whether a function parameter is `in`, `out` or `inout`.
#[derive(Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum FunctionParamDirection {
    Input,
    Output,
    InputOutput,
}

#[cfg_attr(debug_assertions, derive(Debug))]
pub struct FunctionParam {
    pub variable_id: VariableId,
    pub direction: FunctionParamDirection,
}

impl FunctionParam {
    pub fn new(variable_id: VariableId, direction: FunctionParamDirection) -> FunctionParam {
        FunctionParam { variable_id, direction }
    }
}

// A function is defined by its name, parameters, return type and a graph of blocks.  The blocks
// are specified separately in `IR`.
#[cfg_attr(debug_assertions, derive(Debug))]
pub struct Function {
    pub name: Name,
    pub params: Vec<FunctionParam>,
    pub return_type_id: TypeId,
    pub return_precision: Precision,
}

impl Function {
    pub fn new(
        name: &'static str,
        return_type_id: TypeId,
        return_precision: Precision,
    ) -> Function {
        Function {
            // Keep the exact name for main() only
            name: if name == "main" { Name::new_exact(name) } else { Name::new_temp(name) },
            params: Vec::new(),
            return_type_id,
            return_precision,
        }
    }

    pub fn add_param(&mut self, variable_id: VariableId, direction: FunctionParamDirection) {
        self.params.push(FunctionParam::new(variable_id, direction));
    }
}

#[derive(Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum ShaderType {
    Vertex,
    TessellationControl,
    TessellationEvaluation,
    Geometry,
    Fragment,
    Compute,
}

#[derive(Copy, Clone, PartialEq)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum Precision {
    NotApplicable,
    Low,
    Medium,
    High,
}

#[derive(Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum BlockStorage {
    Shared,
    Packed,
    Std140,
    Std430,
}

#[derive(Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum MatrixPacking {
    ColumnMajor,
    RowMajor,
}

#[derive(Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum Depth {
    Any,
    Greater,
    Less,
    Unchanged,
}

#[derive(Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum ImageInternalFormat {
    RGBA32F,
    RGBA16F,
    R32F,
    RGBA32UI,
    RGBA16UI,
    RGBA8UI,
    R32UI,
    RGBA32I,
    RGBA16I,
    RGBA8I,
    R32I,
    RGBA8,
    RGBA8SNORM,
}

#[derive(Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum GeometryPrimitive {
    Undefined,
    Points,
    Lines,
    LinesAdjacency,
    Triangles,
    TrianglesAdjacency,
    LineStrip,
    TriangleStrip,
}

#[derive(Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum TessellationPrimitive {
    Undefined,
    Triangles,
    Quads,
    Isolines,
}

#[derive(Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum TessellationSpacing {
    Undefined,
    EqualSpacing,
    FractionalEvenSpacing,
    FractionalOddSpacing,
}

#[derive(Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum TessellationOrdering {
    Undefined,
    Cw,
    Ccw,
}

#[derive(Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum Decoration {
    // Corresponding to GLSL qualifiers with the same name
    Invariant,
    Precise,
    Interpolant,
    Smooth,
    Flat,
    NoPerspective,
    Centroid,
    Sample,
    Patch,
    Shared,
    ReadOnly,
    WriteOnly,
    Coherent,
    Restrict,
    Volatile,
    Uniform,
    Buffer,
    NonCoherent,
    YUV,
    // Indicates that a variable (excluding built-ins) is an input to the shader
    Input,
    // Indicates that a variable (excluding built-ins) is an output of the shader
    Output,
    // Indicates that a variable (excluding built-ins) is both an input to and output of the
    // shader, used in EXT_shader_framebuffer_fetch.
    InputOutput,
    // The location qualifier of an input or output
    Location(u32),
    // The index qualifier per EXT_blend_func_extended
    Index(u32),
    // The input attachment qualifier for EXT_shader_framebuffer_fetch support
    InputAttachmentIndex(u32),
    // Specialization constant index
    SpecConst(u32),
    // The storage of UBO and SSBO blocks.
    Block(BlockStorage),
    // The binding qualifier
    Binding(u32),
    // The offset qualifier
    Offset(u32),
    // The matrix packing of a block or struct field
    MatrixPacking(MatrixPacking),
    // The depth qualifier
    Depth(Depth),
    // Internal format declared on storage images
    ImageInternalFormat(ImageInternalFormat),
    // Number of views in OVR_multiview
    NumViews(u32),
    // Used internally to implement ANGLE_pixel_local_storage, indicates a D3D 11.3 Rasterizer
    // Order Views (ROV)
    RasterOrdered,
}

// A set of decorations that only affect variables.  They are placed in a vector that's expected to
// always be very short.
#[derive(Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub struct Decorations {
    pub decorations: Vec<Decoration>,
}

impl Decorations {
    pub fn new_none() -> Decorations {
        Decorations { decorations: Vec::new() }
    }
    pub fn new(decorations: Vec<Decoration>) -> Decorations {
        Decorations { decorations }
    }
}

#[derive(Eq, PartialEq, Hash, Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum BasicType {
    Void,
    Float,
    Int,
    Uint,
    Bool,
    AtomicCounter,
    // To support EXT_YUV_target
    YuvCscStandard,
}

#[derive(Eq, PartialEq, Hash, Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum ImageBasicType {
    Float,
    Int,
    Uint,
}

#[derive(Eq, PartialEq, Hash, Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum ImageDimension {
    // Note: D2 is 2D, but works around identifier starting with number
    D2,
    D3,
    Cube,
    Rect,
    Buffer,
    // For OES_EGL_image_external
    External,
    // For GL_EXT_YUV_target
    ExternalY2Y,
    // For WebGL_video_texture
    Video,
    // For ANGLE_shader_pixel_local_storage
    PixelLocal,
    // For subpass inputs
    Subpass,
}

#[derive(Eq, PartialEq, Hash, Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub struct ImageType {
    pub dimension: ImageDimension,
    pub is_sampled: bool,
    pub is_array: bool,
    pub is_ms: bool,
    pub is_shadow: bool,
}

// A field of a struct or interface block.
#[cfg_attr(debug_assertions, derive(Debug))]
pub struct Field {
    pub name: Name,
    pub type_id: TypeId,
    pub precision: Precision,
    pub decorations: Decorations,
}

impl Field {
    pub fn new(
        name: Name,
        type_id: TypeId,
        precision: Precision,
        decorations: Decorations,
    ) -> Field {
        Field { name, type_id, precision, decorations }
    }
}

#[derive(PartialEq, Copy, Clone)]
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum StructSpecialization {
    // TODO: add an entry for structs that have been dead-code-eliminated so they aren't forward
    // declared.
    // TODO: Not enough to look at variable declarations / function returns.  Add a test
    //       that does this, S should still be alive (unless constant folded?):
    //
    //       struct S { ... };
    //       void main() { ... = S{non, constant, values}.member; }
    Struct,
    InterfaceBlock,
}

// The type of an expression.  Types can be recursive, for example a vector, matrix or array of
// another type.
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum Type {
    // Representing Void, Float, Int, Uint, Bool, YuvCscStandard and AtomicCounter
    Scalar(BasicType),
    // Representing FloatImage, IntImage and UintImage
    Image(ImageBasicType, ImageType),
    // Representing Struct, may represent an interface block as well.
    Struct(Name, Vec<Field>, StructSpecialization),
    // A vector of other Scalar types
    Vector(TypeId, u32),
    // A matrix of other Vector types (which are columns)
    Matrix(TypeId, u32),
    // An array of other types
    Array(TypeId, u32),
    UnsizedArray(TypeId),
    // A pointer to a type, includes variables, access chains etc
    Pointer(TypeId),
}

impl Type {
    pub fn new_void() -> Type {
        Type::Scalar(BasicType::Void)
    }
    pub fn new_scalar(basic_type: BasicType) -> Type {
        Type::Scalar(basic_type)
    }
    pub fn new_image(basic_type: ImageBasicType, image_type: ImageType) -> Type {
        Type::Image(basic_type, image_type)
    }
    pub fn new_struct(
        name: Name,
        fields: Vec<Field>,
        specialization: StructSpecialization,
    ) -> Type {
        Type::Struct(name, fields, specialization)
    }
    pub fn new_vector(type_id: TypeId, count: u32) -> Type {
        Type::Vector(type_id, count)
    }
    pub fn new_matrix(type_id: TypeId, count: u32) -> Type {
        Type::Matrix(type_id, count)
    }
    pub fn new_array(type_id: TypeId, count: u32) -> Type {
        Type::Array(type_id, count)
    }
    pub fn new_unsized_array(type_id: TypeId) -> Type {
        Type::UnsizedArray(type_id)
    }
    pub fn new_pointer(type_id: TypeId) -> Type {
        Type::Pointer(type_id)
    }

    pub fn is_scalar(&self) -> bool {
        matches!(self, Type::Scalar(_))
    }

    pub fn is_unsized_array(&self) -> bool {
        matches!(self, Type::UnsizedArray(_))
    }

    pub fn is_pointer(&self) -> bool {
        matches!(self, Type::Pointer(_))
    }

    pub fn get_scalar_basic_type(&self) -> BasicType {
        match self {
            &Type::Scalar(basic_type) => basic_type,
            _ => panic!("Internal error: Expected scalar type"),
        }
    }

    pub fn get_vector_size(&self) -> Option<u32> {
        match self {
            &Type::Vector(_, count) => Some(count),
            _ => None,
        }
    }

    pub fn get_matrix_size(&self) -> Option<u32> {
        match self {
            &Type::Matrix(_, count) => Some(count),
            _ => None,
        }
    }

    pub fn get_element_type_id(&self) -> Option<TypeId> {
        match self {
            &Type::Vector(element_id, _) => Some(element_id),
            &Type::Matrix(element_id, _) => Some(element_id),
            &Type::Array(element_id, _) => Some(element_id),
            &Type::UnsizedArray(element_id) => Some(element_id),
            &Type::Pointer(element_id) => Some(element_id),
            _ => None,
        }
    }

    pub fn get_struct_field(&self, field: u32) -> &Field {
        match self {
            Type::Struct(_, fields, _) => &fields[field as usize],
            _ => panic!("Internal error: Expected struct type"),
        }
    }

    pub fn get_total_component_count(&self, ir_meta: &IRMeta) -> u32 {
        let mut total_components = 1;

        if let &Type::Matrix(vector_type_id, count) = self {
            let vector_type_info = ir_meta.get_type(vector_type_id);
            total_components = count * vector_type_info.get_vector_size().unwrap();
        } else if let &Type::Vector(_, count) = self {
            total_components = count;
        }

        total_components
    }
}

#[cfg_attr(debug_assertions, derive(Debug))]
pub struct AdvancedBlendEquations {
    pub multiply: bool,
    pub screen: bool,
    pub overlay: bool,
    pub darken: bool,
    pub lighten: bool,
    pub colordodge: bool,
    pub colorburn: bool,
    pub hardlight: bool,
    pub softlight: bool,
    pub difference: bool,
    pub exclusion: bool,
    pub hsl_hue: bool,
    pub hsl_saturation: bool,
    pub hsl_color: bool,
    pub hsl_luminosity: bool,
}

impl AdvancedBlendEquations {
    pub fn new() -> AdvancedBlendEquations {
        AdvancedBlendEquations {
            multiply: false,
            screen: false,
            overlay: false,
            darken: false,
            lighten: false,
            colordodge: false,
            colorburn: false,
            hardlight: false,
            softlight: false,
            difference: false,
            exclusion: false,
            hsl_hue: false,
            hsl_saturation: false,
            hsl_color: false,
            hsl_luminosity: false,
        }
    }

    pub fn set_all(&mut self) {
        self.multiply = true;
        self.screen = true;
        self.overlay = true;
        self.darken = true;
        self.lighten = true;
        self.colordodge = true;
        self.colorburn = true;
        self.hardlight = true;
        self.softlight = true;
        self.difference = true;
        self.exclusion = true;
        self.hsl_hue = true;
        self.hsl_saturation = true;
        self.hsl_color = true;
        self.hsl_luminosity = true;
    }

    pub fn all(&self) -> bool {
        self.multiply
            && self.screen
            && self.overlay
            && self.darken
            && self.lighten
            && self.colordodge
            && self.colorburn
            && self.hardlight
            && self.softlight
            && self.difference
            && self.exclusion
            && self.hsl_hue
            && self.hsl_saturation
            && self.hsl_color
            && self.hsl_luminosity
    }
}

// The entire IR.  At a high level, the IR is:
//
// - A set of types
// - A set of constants
// - A set of variables
// - A set of Functions
//
// Each of the above is implemented as a map of the respective ID to data.  Additionally, the IR
// carries other global data, such as shader type, enabled advanced blend equations, geometry or
// tessellation parameters etc.
//
// Transformations traverse functions and mutate them.  The rest of the IR may be accessed and
// modified during these transformations too.  Since the many layers of helper functions and
// closures makes it hard for Rust to verify things, everything in the IR except the function
// blocks are split into an `IRMeta` struct (allowing both the `.functions` and `.meta` to be
// borrowed as &mut).
#[cfg_attr(debug_assertions, derive(Debug))]
pub struct IRMeta {
    types: Vec<Type>,
    constants: Vec<Constant>,
    variables: Vec<Variable>,
    functions: Vec<Function>,

    // List of instructions indexed by register ID.  Stored in a global array, this allows
    // easy look up of instructions that calculate an id, such as the parameter of another
    // instruction.
    instructions: Vec<Instruction>,

    // List of global variables.
    global_variables: Vec<VariableId>,

    // Id of main().
    main_function_id: Option<FunctionId>,

    // Maps to look up the id of existing types.  This ensures that the type ids are unique and
    // there are no duplicate types.
    image_type_map: HashMap<(ImageBasicType, ImageType), TypeId>,
    array_type_map: HashMap<(TypeId, u32), TypeId>,
    pointer_type_map: HashMap<TypeId, TypeId>,

    // Maps to look up the id of existing constants to avoid duplicates even though they are
    // harmless.  Note that float constants are hashed by their bit pattern as integers because
    // Rust does not implement Eq and Hash for f32.
    float_constant_map: HashMap<u32, ConstantId>,
    int_constant_map: HashMap<i32, ConstantId>,
    uint_constant_map: HashMap<u32, ConstantId>,
    composite_constant_map: HashMap<Vec<ConstantId>, ConstantId>,

    // Data that globally affects the shader:
    shader_type: ShaderType,

    // Affecting fragment shaders:
    early_fragment_tests: bool,
    advanced_blend_equations: AdvancedBlendEquations,

    // Affecting tessellation constrol shaders:
    tcs_vertices: u32,

    // Affecting tessellation evaluation shaders:
    tes_primitive: TessellationPrimitive,
    tes_vertex_spacing: TessellationSpacing,
    tes_ordering: TessellationOrdering,
    tes_point_mode: bool,

    // Affecting geometry shaders:
    gs_primitive_in: GeometryPrimitive,
    gs_primitive_out: GeometryPrimitive,
    gs_invocations: u32,
    gs_max_vertices: u32,
    // TODO: invariant can be globally set? There are some other qualifiers that can be globally
    // set, find them.
}

impl IRMeta {
    pub fn new(shader_type: ShaderType) -> IRMeta {
        let mut types = vec![
            // Corresponding to TYPE_ID_VOID
            Type::new_void(),
            // Corresponding to TYPE_ID_FLOAT
            Type::new_scalar(BasicType::Float),
            // Corresponding to TYPE_ID_INT
            Type::new_scalar(BasicType::Int),
            // Corresponding to TYPE_ID_UINT
            Type::new_scalar(BasicType::Uint),
            // Corresponding to TYPE_ID_BOOL
            Type::new_scalar(BasicType::Bool),
            // Corresponding to TYPE_ID_ATOMIC_COUNTER
            Type::new_scalar(BasicType::AtomicCounter),
            // Corresponding to TYPE_ID_YUV_CSC_STANDARD
            Type::new_scalar(BasicType::YuvCscStandard),
            // Corresponding to TYPE_ID_VEC2
            Type::new_vector(TYPE_ID_FLOAT, 2),
            // Corresponding to TYPE_ID_VEC3
            Type::new_vector(TYPE_ID_FLOAT, 3),
            // Corresponding to TYPE_ID_VEC4
            Type::new_vector(TYPE_ID_FLOAT, 4),
            // Corresponding to TYPE_ID_IVEC2
            Type::new_vector(TYPE_ID_INT, 2),
            // Corresponding to TYPE_ID_IVEC3
            Type::new_vector(TYPE_ID_INT, 3),
            // Corresponding to TYPE_ID_IVEC4
            Type::new_vector(TYPE_ID_INT, 4),
            // Corresponding to TYPE_ID_UVEC2
            Type::new_vector(TYPE_ID_UINT, 2),
            // Corresponding to TYPE_ID_UVEC3
            Type::new_vector(TYPE_ID_UINT, 3),
            // Corresponding to TYPE_ID_UVEC4
            Type::new_vector(TYPE_ID_UINT, 4),
            // Corresponding to TYPE_ID_BVEC2
            Type::new_vector(TYPE_ID_BOOL, 2),
            // Corresponding to TYPE_ID_BVEC3
            Type::new_vector(TYPE_ID_BOOL, 3),
            // Corresponding to TYPE_ID_BVEC4
            Type::new_vector(TYPE_ID_BOOL, 4),
            // Corresponding to TYPE_ID_MAT2
            Type::new_matrix(TYPE_ID_VEC2, 2),
            // Corresponding to TYPE_ID_MAT2X3
            Type::new_matrix(TYPE_ID_VEC3, 2),
            // Corresponding to TYPE_ID_MAT2X4
            Type::new_matrix(TYPE_ID_VEC4, 2),
            // Corresponding to TYPE_ID_MAT3X2
            Type::new_matrix(TYPE_ID_VEC2, 3),
            // Corresponding to TYPE_ID_MAT3
            Type::new_matrix(TYPE_ID_VEC3, 3),
            // Corresponding to TYPE_ID_MAT3X4
            Type::new_matrix(TYPE_ID_VEC4, 3),
            // Corresponding to TYPE_ID_MAT4X2
            Type::new_matrix(TYPE_ID_VEC2, 4),
            // Corresponding to TYPE_ID_MAT4X3
            Type::new_matrix(TYPE_ID_VEC3, 4),
            // Corresponding to TYPE_ID_MAT4
            Type::new_matrix(TYPE_ID_VEC4, 4),
        ];

        let mut constants = vec![
            // Corresponding to CONSTANT_ID_FALSE
            Constant::new_bool(false),
            // Corresponding to CONSTANT_ID_TRUE
            Constant::new_bool(true),
            // Corresponding to CONSTANT_ID_FLOAT_ZERO
            Constant::new_float(0.0),
            // Corresponding to CONSTANT_ID_FLOAT_ONE
            Constant::new_float(1.0),
            // Corresponding to CONSTANT_ID_INT_ZERO
            Constant::new_int(0),
            // Corresponding to CONSTANT_ID_INT_ONE
            Constant::new_int(1),
            // Corresponding to CONSTANT_ID_UINT_ZERO
            Constant::new_uint(0),
            // Corresponding to CONSTANT_ID_UINT_ONE
            Constant::new_uint(1),
            // Corresponding to CONSTANT_ID_YUV_CSC_ITU601
            Constant::new_yuv_csc(YuvCscStandard::Itu601),
            // Corresponding to CONSTANT_ID_YUV_CSC_ITU601_FULL_RANGE
            Constant::new_yuv_csc(YuvCscStandard::Itu601FullRange),
            // Corresponding to CONSTANT_ID_YUV_CSC_ITU709
            Constant::new_yuv_csc(YuvCscStandard::Itu709),
        ];

        let mut variables = Vec::new();
        let mut functions = Vec::new();
        let mut instructions = Vec::new();

        // Reserve sensible counts upfront.
        // Fr types, typically there may be a few structs and blocks and some samplers.
        types.reserve(20);
        // For constants, there can be many of them.
        constants.reserve(200);
        // For variables, there can be many temporary ones.
        variables.reserve(200);
        // For functions, there are typically no more than a handful.
        functions.reserve(20);
        // For instructions (corresponding to register ids), hundreds in a shader are common.
        instructions.reserve(200);

        IRMeta {
            types,
            constants,
            variables,
            functions,
            instructions,
            global_variables: Vec::new(),
            main_function_id: None,
            image_type_map: HashMap::new(),
            array_type_map: HashMap::new(),
            pointer_type_map: HashMap::new(),
            float_constant_map: HashMap::new(),
            int_constant_map: HashMap::new(),
            uint_constant_map: HashMap::new(),
            composite_constant_map: HashMap::new(),
            shader_type,
            early_fragment_tests: false,
            advanced_blend_equations: AdvancedBlendEquations::new(),
            tcs_vertices: 0,
            tes_primitive: TessellationPrimitive::Undefined,
            tes_vertex_spacing: TessellationSpacing::Undefined,
            tes_ordering: TessellationOrdering::Undefined,
            tes_point_mode: false,
            gs_primitive_in: GeometryPrimitive::Undefined,
            gs_primitive_out: GeometryPrimitive::Undefined,
            gs_invocations: 0,
            gs_max_vertices: 0,
        }
    }

    pub fn all_types(&self) -> &Vec<Type> {
        &self.types
    }
    pub fn all_constants(&self) -> &Vec<Constant> {
        &self.constants
    }
    pub fn all_variables(&self) -> &Vec<Variable> {
        &self.variables
    }
    pub fn all_functions(&self) -> &Vec<Function> {
        &self.functions
    }
    pub fn all_functions_mut(&mut self) -> &mut Vec<Function> {
        &mut self.functions
    }
    pub fn all_global_variables(&self) -> &Vec<VariableId> {
        &self.global_variables
    }

    pub fn get_main_function_id(&self) -> Option<FunctionId> {
        self.main_function_id
    }
    pub fn set_main_function_id(&mut self, main_id: FunctionId) {
        debug_assert!(self.main_function_id.is_none());
        self.main_function_id = Some(main_id);
    }

    pub fn get_shader_type(&self) -> ShaderType {
        self.shader_type
    }
    pub fn get_early_fragment_tests(&self) -> bool {
        self.early_fragment_tests
    }
    pub fn get_advanced_blend_equations(&self) -> &AdvancedBlendEquations {
        &self.advanced_blend_equations
    }
    pub fn get_tcs_vertices(&self) -> u32 {
        self.tcs_vertices
    }
    pub fn get_tes_primitive(&self) -> TessellationPrimitive {
        self.tes_primitive
    }
    pub fn get_tes_vertex_spacing(&self) -> TessellationSpacing {
        self.tes_vertex_spacing
    }
    pub fn get_tes_ordering(&self) -> TessellationOrdering {
        self.tes_ordering
    }
    pub fn get_tes_point_mode(&self) -> bool {
        self.tes_point_mode
    }
    pub fn get_gs_primitive_in(&self) -> GeometryPrimitive {
        self.gs_primitive_in
    }
    pub fn get_gs_primitive_out(&self) -> GeometryPrimitive {
        self.gs_primitive_out
    }
    pub fn get_gs_invocations(&self) -> u32 {
        self.gs_invocations
    }
    pub fn get_gs_max_vertices(&self) -> u32 {
        self.gs_max_vertices
    }

    fn add_item_and_get_id<T>(items: &mut Vec<T>, new_item: T) -> u32 {
        let item_id = items.len() as u32;
        items.push(new_item);
        item_id
    }

    fn add_type_and_get_id(types: &mut Vec<Type>, type_desc: Type) -> TypeId {
        TypeId { id: Self::add_item_and_get_id(types, type_desc) }
    }

    pub fn add_constant_and_get_id(
        constants: &mut Vec<Constant>,
        constant: Constant,
    ) -> ConstantId {
        ConstantId { id: Self::add_item_and_get_id(constants, constant) }
    }

    // Returns a predefined type id, see TYPE_ID_* constants.
    pub fn get_basic_type_id(&self, basic_type: BasicType) -> TypeId {
        match basic_type {
            BasicType::Void => TYPE_ID_VOID,
            BasicType::Float => TYPE_ID_FLOAT,
            BasicType::Int => TYPE_ID_INT,
            BasicType::Uint => TYPE_ID_UINT,
            BasicType::Bool => TYPE_ID_BOOL,
            BasicType::AtomicCounter => TYPE_ID_ATOMIC_COUNTER,
            BasicType::YuvCscStandard => TYPE_ID_YUV_CSC_STANDARD,
        }
    }

    // Returns a predefined type id for vectors, see TYPE_ID_* constants.
    pub fn get_vector_type_id(&self, basic_type: BasicType, vector_size: u32) -> TypeId {
        debug_assert!(vector_size >= 2 && vector_size <= 4);

        let offset = vector_size - 2;

        match basic_type {
            BasicType::Float => TypeId { id: TYPE_ID_VEC2.id + offset },
            BasicType::Int => TypeId { id: TYPE_ID_IVEC2.id + offset },
            BasicType::Uint => TypeId { id: TYPE_ID_UVEC2.id + offset },
            BasicType::Bool => TypeId { id: TYPE_ID_BVEC2.id + offset },
            _ => panic!("Internal error: Not a vector-able type"),
        }
    }

    // Returns a vectors type id based on the given element.
    pub fn get_vector_type_id_from_element_id(
        &self,
        element_type_id: TypeId,
        vector_size: u32,
    ) -> TypeId {
        match element_type_id {
            TYPE_ID_FLOAT => self.get_vector_type_id(BasicType::Float, vector_size),
            TYPE_ID_INT => self.get_vector_type_id(BasicType::Int, vector_size),
            TYPE_ID_UINT => self.get_vector_type_id(BasicType::Uint, vector_size),
            TYPE_ID_BOOL => self.get_vector_type_id(BasicType::Bool, vector_size),
            _ => panic!("Internal error: Not a vector-able type"),
        }
    }

    // Returns a predefined type id for matrices, see TYPE_ID_* constants.
    pub fn get_matrix_type_id(&self, column_count: u32, row_count: u32) -> TypeId {
        debug_assert!(column_count >= 2 && column_count <= 4);
        debug_assert!(row_count >= 2 && row_count <= 4);

        let offset = (column_count - 2) * 3 + (row_count - 2);
        TypeId { id: TYPE_ID_MAT2.id + offset }
    }

    // TODO: get_image_type_id, get_array_type_id, and get_constant_* are very similar.  Maybe they
    // can be templatized + some lambdas
    pub fn get_image_type_id(
        &mut self,
        basic_type: ImageBasicType,
        image_type: ImageType,
    ) -> TypeId {
        // Look up the image type; if one doesn't exist, create it.
        *self.image_type_map.entry((basic_type, image_type)).or_insert_with(|| {
            Self::add_type_and_get_id(&mut self.types, Type::new_image(basic_type, image_type))
        })
    }

    pub fn get_struct_type_id(
        &mut self,
        name: Name,
        fields: Vec<Field>,
        specialization: StructSpecialization,
    ) -> TypeId {
        // Note: this should only make new types, not used as a way to look up a struct, that ID
        // should already be known.
        Self::add_type_and_get_id(&mut self.types, Type::new_struct(name, fields, specialization))
    }

    pub fn get_array_type_id(&mut self, element_type_id: TypeId, array_size: u32) -> TypeId {
        // Look up the array type; if one doesn't exist, create it.
        *self.array_type_map.entry((element_type_id, array_size)).or_insert_with(|| {
            Self::add_type_and_get_id(&mut self.types, Type::new_array(element_type_id, array_size))
        })
    }

    pub fn get_pointer_type_id(&mut self, pointee_type_id: TypeId) -> TypeId {
        // Look up the pointer type; if one doesn't exist, create it.
        *self.pointer_type_map.entry(pointee_type_id).or_insert_with(|| {
            Self::add_type_and_get_id(&mut self.types, Type::new_pointer(pointee_type_id))
        })
    }

    pub fn get_constant_float(&mut self, value: f32) -> ConstantId {
        // Look up the float constant; if one doesn't exist, create it.
        *self.float_constant_map.entry(value.to_bits()).or_insert_with(|| {
            Self::add_constant_and_get_id(&mut self.constants, Constant::new_float(value))
        })
    }

    pub fn get_constant_int(&mut self, value: i32) -> ConstantId {
        // Look up the int constant; if one doesn't exist, create it.
        *self.int_constant_map.entry(value).or_insert_with(|| {
            Self::add_constant_and_get_id(&mut self.constants, Constant::new_int(value))
        })
    }

    pub fn get_constant_uint(&mut self, value: u32) -> ConstantId {
        // Look up the int constant; if one doesn't exist, create it.
        *self.uint_constant_map.entry(value).or_insert_with(|| {
            Self::add_constant_and_get_id(&mut self.constants, Constant::new_uint(value))
        })
    }

    pub fn get_constant_yuv_csc_standard(&mut self, value: YuvCscStandard) -> ConstantId {
        match value {
            YuvCscStandard::Itu601 => CONSTANT_ID_YUV_CSC_ITU601,
            YuvCscStandard::Itu601FullRange => CONSTANT_ID_YUV_CSC_ITU601_FULL_RANGE,
            YuvCscStandard::Itu709 => CONSTANT_ID_YUV_CSC_ITU709,
        }
    }

    pub fn get_constant_bool(&mut self, value: bool) -> ConstantId {
        // Bool constants are predefined
        if value { CONSTANT_ID_TRUE } else { CONSTANT_ID_FALSE }
    }

    pub fn get_constant_composite(
        &mut self,
        type_id: TypeId,
        components: Vec<ConstantId>,
    ) -> ConstantId {
        // Look up the composite constant; if one doesn't exist, create it.
        *self.composite_constant_map.entry(components.clone()).or_insert_with(|| {
            Self::add_constant_and_get_id(
                &mut self.constants,
                Constant::new_composite(type_id, components),
            )
        })
    }

    pub fn get_const_float_value(&self, constant_id: ConstantId) -> f32 {
        match self.get_constant(constant_id).value {
            ConstantValue::Float(f) => f,
            _ => panic!("Internal error: Expected float value is not a float value"),
        }
    }
    pub fn get_const_bool_value(&self, constant_id: ConstantId) -> bool {
        match self.get_constant(constant_id).value {
            ConstantValue::Bool(b) => b,
            _ => panic!("Internal error: Expected bool value is not a bool value"),
        }
    }

    pub fn add_variable(&mut self, variable: Variable) -> VariableId {
        VariableId { id: Self::add_item_and_get_id(&mut self.variables, variable) }
    }
    pub fn declare_variable(
        &mut self,
        name: Name,
        type_id: TypeId,
        precision: Precision,
        decorations: Decorations,
        built_in: Option<BuiltIn>,
        initializer: Option<ConstantId>,
    ) -> VariableId {
        // Automatically turn the type into a pointer
        debug_assert!(!matches!(self.get_type(type_id), Type::Pointer(_)));
        let type_id = self.get_pointer_type_id(type_id);
        let var = Variable::new(name, type_id, precision, decorations, built_in, initializer);
        self.add_variable(var)
    }
    pub fn add_to_global_variables(&mut self, id: VariableId) {
        self.global_variables.push(id);
    }

    pub fn set_variable_initializer(&mut self, id: VariableId, constant_id: ConstantId) {
        debug_assert!(self.variables[id.id as usize].initializer.is_none());
        self.variables[id.id as usize].initializer = Some(constant_id);
    }

    pub fn add_function(&mut self, function: Function) -> FunctionId {
        FunctionId { id: IRMeta::add_item_and_get_id(&mut self.functions, function) }
    }
    pub fn set_function_params(&mut self, id: FunctionId, params: Vec<FunctionParam>) {
        debug_assert!(self.functions[id.id as usize].params.is_empty());
        self.functions[id.id as usize].params = params;
    }

    pub fn get_type(&self, type_id: TypeId) -> &Type {
        // TODO: this may be in a hot loop, do unsafe lookup + debug_assert when outside Chrome?
        // Same with other getters.  Need to measure if it impacts performnace.
        &self.types[type_id.id as usize]
    }
    pub fn get_type_mut(&mut self, type_id: TypeId) -> &mut Type {
        &mut self.types[type_id.id as usize]
    }
    pub fn get_constant(&self, constant_id: ConstantId) -> &Constant {
        &self.constants[constant_id.id as usize]
    }
    pub fn get_constant_mut(&mut self, constant_id: ConstantId) -> &mut Constant {
        &mut self.constants[constant_id.id as usize]
    }
    pub fn get_variable(&self, variable_id: VariableId) -> &Variable {
        &self.variables[variable_id.id as usize]
    }
    pub fn get_variable_mut(&mut self, variable_id: VariableId) -> &mut Variable {
        &mut self.variables[variable_id.id as usize]
    }
    pub fn get_function(&self, function_id: FunctionId) -> &Function {
        &self.functions[function_id.id as usize]
    }
    pub fn get_function_mut(&mut self, function_id: FunctionId) -> &mut Function {
        &mut self.functions[function_id.id as usize]
    }
    pub fn get_instruction(&self, register_id: RegisterId) -> &Instruction {
        &self.instructions[register_id.id as usize]
    }
    pub fn get_instruction_mut(&mut self, register_id: RegisterId) -> &mut Instruction {
        &mut self.instructions[register_id.id as usize]
    }
    pub fn get_aliased_id(&self, register_id: RegisterId) -> TypedId {
        // Follow `Alias`es of the instruction to get to the origin of the id.
        // Useful when analyzing / transforming the IR.
        let instruction = &self.instructions[register_id.id as usize];
        if let OpCode::Alias(alias_id) = instruction.op {
            match alias_id.id {
                Id::Register(alias_id) => self.get_aliased_id(alias_id),
                _ => alias_id,
            }
        } else {
            TypedId::from_register_id(instruction.result)
        }
    }
    pub fn replace_instruction(&mut self, to_replace: RegisterId, replace_by: RegisterId) {
        // Takes an instruction with ID `replace_by` and makes the instruction `to_replace` contain
        // that instead.  This is used by transformations to change an instruction without
        // affecting its result ID, and therefore no need to replicate an ID change throughout the
        // IR.
        self.instructions.swap(to_replace.id as usize, replace_by.id as usize);
        // Make sure the replacing instruction has the id of the one being replaced.  The other is
        // not worth correcting as it is a dead ID.
        self.instructions[to_replace.id as usize].result.id = to_replace;

        // The `replace_by` ID should no longer be used, it refers to an instruction that is
        // technically thrown away.  If possible, actually throw it away.
        if replace_by.id as usize + 1 == self.instructions.len() {
            self.instructions.pop();
        }
    }
    pub fn assign_new_register_to_instruction(&mut self, to_replace: RegisterId) -> RegisterId {
        // Used to change the ID of an instruction with a new one.  This is used by transformations
        // to make the instruction produce a new ID, let that be post-processed and eventually
        // have another instruction produce the original ID (using `replace_instruction` above).
        // In this way, the rest of the IR does not need to be modified to take the transformations
        // into account.
        let new_id = RegisterId { id: self.instructions.len() as u32 };

        // Take the instruction out and place a bogus instruction in its place.
        let mut instruction = std::mem::replace(
            &mut self.instructions[to_replace.id as usize],
            Instruction::new(
                OpCode::NextBlock,
                TypedRegisterId::new(to_replace, TYPE_ID_VOID, Precision::NotApplicable),
            ),
        );

        // Change the result of the instruction and place it in the instruction list.
        instruction.result.id = new_id;
        self.instructions.push(instruction);
        new_id
    }

    pub fn new_register(
        &mut self,
        op: OpCode,
        type_id: TypeId,
        precision: Precision,
    ) -> TypedRegisterId {
        let new_id = RegisterId { id: self.instructions.len() as u32 };
        let typed_id = TypedRegisterId::new(new_id, type_id, precision);
        self.instructions.push(Instruction::new(op, typed_id));
        typed_id
    }

    // Given a matrix, vector or scalar type id, retrieves the scalar type of their components.
    pub fn get_scalar_type(&self, type_id: TypeId) -> TypeId {
        let type_info = self.get_type(type_id);
        let element_type_id = type_info.get_element_type_id().unwrap_or(type_id);
        let element_type_info = self.get_type(element_type_id);
        element_type_info.get_element_type_id().unwrap_or(element_type_id)
    }
}

#[cfg_attr(debug_assertions, derive(Debug))]
pub struct IR {
    pub meta: IRMeta,
    // The first block of the function for each function id.  This is the root of the block graph.
    // If this block doesn't exist, the function has been dead-code-eliminated.
    //
    // This is separate from IRMeta so that can be mutated while the function blocks are traversed.
    pub function_entries: Vec<Option<Block>>,
}

impl IR {
    pub fn new(shader_type: ShaderType) -> IR {
        IR { meta: IRMeta::new(shader_type), function_entries: Vec::with_capacity(20) }
    }

    pub fn add_function(&mut self, function: Function) -> FunctionId {
        let new_id = self.meta.add_function(function);
        debug_assert!(new_id.id as usize == self.function_entries.len());
        self.function_entries.push(None);
        new_id
    }

    pub fn set_function_entry(&mut self, id: FunctionId, entry: Block) {
        debug_assert!(self.function_entries[id.id as usize].is_none());
        self.function_entries[id.id as usize] = Some(entry);
    }

    pub fn prepend_to_main(&mut self, mut new_entry: Block) {
        let main_function_id = self.meta.get_main_function_id().unwrap();
        let main_entry = &mut self.function_entries[main_function_id.id as usize];
        let main_entry_block = main_entry.take().unwrap();

        // Terminate the last block of `new_entry` with a `NextBlock`, and set its merge block to
        // the current main block.
        let last_block = new_entry.get_merge_chain_last_block_mut();
        debug_assert!(matches!(last_block.get_terminating_op(), OpCode::NextBlock));
        last_block.set_merge_block(main_entry_block);

        *main_entry = Some(new_entry);
    }
}
