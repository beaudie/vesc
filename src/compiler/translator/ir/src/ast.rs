// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// A helper to turn the IR into a GLSL AST.  For the GLSL generator, this is used to generate the
// shader itself.  While the translator is not yet fully converted to use the IR, this is used to
// drop back to the legacy AST.
use crate::ir::*;
use crate::*;

pub trait Target {
    type BlockResult;

    fn begin(&mut self);
    fn end(&mut self);

    // Upfront, create objects that would later represent types, constants, variables and
    // functions.  Later, the generator references these objects by ID and the implementation
    // can look these objects up based on that.
    fn new_type(&mut self, id: TypeId, type_info: &Type);
    fn new_constant(&mut self, id: ConstantId, constant: &Constant);
    fn new_variable(&mut self, id: VariableId, variable: &Variable);
    fn new_function(&mut self, id: FunctionId, function: &Function);

    // Set up everything that needs to be set up in the global scope.  That includes global
    // variables, geometry/tessellation info, etc.
    fn global_scope(&mut self, ir_meta: &IRMeta);

    fn begin_block(&mut self, variables: &Vec<VariableId>) -> Self::BlockResult;
    fn merge_blocks(&mut self, blocks: Vec<Self::BlockResult>) -> Self::BlockResult;

    // Instructions
    fn swizzle_single(
        &mut self,
        block: &mut Self::BlockResult,
        result: RegisterId,
        id: TypedId,
        index: u32,
    );
    fn swizzle_multi(
        &mut self,
        block: &mut Self::BlockResult,
        result: RegisterId,
        id: TypedId,
        indices: &Vec<u32>,
    );
    fn index(
        &mut self,
        block: &mut Self::BlockResult,
        result: RegisterId,
        id: TypedId,
        index: TypedId,
    );
    fn select_field(
        &mut self,
        block: &mut Self::BlockResult,
        result: RegisterId,
        id: TypedId,
        index: u32,
        field: &Field,
    );
    fn construct_single(
        &mut self,
        block: &mut Self::BlockResult,
        result: RegisterId,
        type_id: TypeId,
        id: TypedId,
    );
    fn construct_multi(
        &mut self,
        block: &mut Self::BlockResult,
        result: RegisterId,
        type_id: TypeId,
        ids: &Vec<TypedId>,
    );
    fn load(&mut self, block: &mut Self::BlockResult, result: RegisterId, pointer: TypedId);
    fn store(&mut self, block: &mut Self::BlockResult, pointer: TypedId, value: TypedId);
    fn call(
        &mut self,
        block: &mut Self::BlockResult,
        result: Option<RegisterId>,
        function_id: FunctionId,
        params: &Vec<TypedId>,
    );
    fn unary(
        &mut self,
        block: &mut Self::BlockResult,
        result: RegisterId,
        unary_op: UnaryOpCode,
        id: TypedId,
    );
    fn binary(
        &mut self,
        block: &mut Self::BlockResult,
        result: RegisterId,
        binary_op: BinaryOpCode,
        lhs: TypedId,
        rhs: TypedId,
    );

    // Control flow
    fn branch_discard(&mut self, block: &mut Self::BlockResult);
    fn branch_return(&mut self, block: &mut Self::BlockResult, value: Option<TypedId>);
    fn branch_break(&mut self, block: &mut Self::BlockResult);
    fn branch_continue(&mut self, block: &mut Self::BlockResult);
    fn branch_if(
        &mut self,
        block: &mut Self::BlockResult,
        condition: TypedId,
        true_block: Option<Self::BlockResult>,
        false_block: Option<Self::BlockResult>,
    );
    fn branch_loop(
        &mut self,
        block: &mut Self::BlockResult,
        loop_condition_block: Option<Self::BlockResult>,
        body_block: Option<Self::BlockResult>,
    );
    fn branch_do_loop(
        &mut self,
        block: &mut Self::BlockResult,
        body_block: Option<Self::BlockResult>,
    );
    fn branch_loop_if(&mut self, block: &mut Self::BlockResult, condition: TypedId);
    fn branch_switch(
        &mut self,
        block: &mut Self::BlockResult,
        value: TypedId,
        case_ids: &Vec<Option<ConstantId>>,
        case_blocks: Vec<Self::BlockResult>,
    );

    // Take the current AST and place it as the body of the given function.
    fn end_function(&mut self, block: Self::BlockResult, id: FunctionId);
}

pub struct Generator<'ir> {
    ir_meta: &'ir IRMeta,
}

impl<'ir> Generator<'ir> {
    pub fn new(ir_meta: &'ir IRMeta) -> Generator {
        Generator { ir_meta }
    }

    // Note: call transform::dealias::run() beforehands, as well as transform::astify::run().
    pub fn generate<T: Target>(&mut self, function_entries: &Vec<Option<Block>>, target: &mut T) {
        // Declare the base types, variables and functions up-front so they can be referred to
        // by ids when generating the AST itself.
        self.declare_base_types(target);
        self.declare_constants(target);
        self.declare_variables(target);
        self.declare_functions(target);

        self.generate_ast(function_entries, target);

        // TODO: have the generator know what to do with types, text gen doesn't do much, but
        //       TType gen has to create new TTypes based on other decorations.  Then
        //       intermediate nodes automatically derive their TTypes and I don't have to
        //       (precision still needs to be communicated).
    }

    fn declare_base_types<T: Target>(&mut self, target: &mut T) {
        self.ir_meta
            .all_types()
            .iter()
            .enumerate()
            .for_each(|(id, type_info)| target.new_type(TypeId { id: id as u32 }, type_info));
    }

    fn declare_constants<T: Target>(&mut self, target: &mut T) {
        self.ir_meta
            .all_constants()
            .iter()
            .enumerate()
            .for_each(|(id, constant)| target.new_constant(ConstantId { id: id as u32 }, constant));
    }

    fn declare_variables<T: Target>(&mut self, target: &mut T) {
        self.ir_meta
            .all_variables()
            .iter()
            .enumerate()
            .for_each(|(id, variable)| target.new_variable(VariableId { id: id as u32 }, variable));
    }

    fn declare_functions<T: Target>(&mut self, target: &mut T) {
        self.ir_meta
            .all_functions()
            .iter()
            .enumerate()
            .for_each(|(id, function)| target.new_function(FunctionId { id: id as u32 }, function));
    }

    fn generate_ast<T: Target>(&mut self, function_entries: &Vec<Option<Block>>, target: &mut T) {
        target.begin();

        // Prepare the global scope
        target.global_scope(self.ir_meta);

        // Visit the functions in DAG-sorted order, so that forward declarations are
        // unnecessary.
        let function_decl_order =
            util::calculate_function_decl_order(self.ir_meta, function_entries);

        for function_id in function_decl_order {
            let entry = &function_entries[function_id.id as usize].as_ref().unwrap();

            let result_body = self.generate_block(entry, target);
            target.end_function(result_body, function_id);
        }

        target.end();
    }

    fn generate_block<T: Target>(&mut self, block: &Block, target: &mut T) -> T::BlockResult {
        let traverser = traverser::Visitor::new();
        traverser.visit_block_instructions(
            &mut (self, target),
            block,
            &|(_, target), block: &Block| {
                // transform::astify::run() should have gotten rid of merge block inputs.
                debug_assert!(block.input.is_none());
                target.begin_block(&block.variables)
            },
            &|(generator, target), block_result, instructions| {
                generator.generate_instructions(block_result, instructions, *target);
            },
            &|(generator, target),
              block_result,
              branch_opcode,
              loop_condition_block_result,
              block1_result,
              block2_result,
              case_block_results| {
                generator.generate_branch_instruction(
                    block_result,
                    branch_opcode,
                    loop_condition_block_result,
                    block1_result,
                    block2_result,
                    case_block_results,
                    *target,
                );
            },
            &|(_, target), block_result_chain| target.merge_blocks(block_result_chain),
        )
    }

    fn generate_instructions<T: Target>(
        &mut self,
        block_result: &mut T::BlockResult,
        instructions: &[BlockInstruction],
        target: &mut T,
    ) {
        // Generate nodes for all instructions except the terminating branch instruction.
        instructions.iter().for_each(|instruction| {
            let (op, result) = instruction.get_op_and_result(self.ir_meta);
            match op {
                &OpCode::ExtractVectorComponent(id, index)
                | &OpCode::AccessVectorComponent(id, index) => {
                    target.swizzle_single(block_result, result.unwrap().id, id, index)
                }
                &OpCode::ExtractVectorComponentMulti(id, ref indices)
                | &OpCode::AccessVectorComponentMulti(id, ref indices) => {
                    target.swizzle_multi(block_result, result.unwrap().id, id, indices)
                }
                &OpCode::ExtractVectorComponentDynamic(id, index)
                | &OpCode::AccessVectorComponentDynamic(id, index)
                | &OpCode::ExtractMatrixColumn(id, index)
                | &OpCode::AccessMatrixColumn(id, index)
                | &OpCode::ExtractArrayElement(id, index)
                | &OpCode::AccessArrayElement(id, index) => {
                    target.index(block_result, result.unwrap().id, id, index)
                }
                &OpCode::ExtractStructField(id, index) => target.select_field(
                    block_result,
                    result.unwrap().id,
                    id,
                    index,
                    self.ir_meta.get_type(id.type_id).get_struct_field(index),
                ),
                &OpCode::AccessStructField(id, index) => {
                    let struct_type_id =
                        self.ir_meta.get_type(id.type_id).get_element_type_id().unwrap();
                    target.select_field(
                        block_result,
                        result.unwrap().id,
                        id,
                        index,
                        self.ir_meta.get_type(struct_type_id).get_struct_field(index),
                    )
                }

                &OpCode::ConstructScalarFromScalar(id)
                | &OpCode::ConstructVectorFromScalar(id)
                | &OpCode::ConstructMatrixFromScalar(id)
                | &OpCode::ConstructMatrixFromMatrix(id) => {
                    let result = result.unwrap();
                    target.construct_single(block_result, result.id, result.type_id, id);
                }
                &OpCode::ConstructVectorFromMultiple(ref ids)
                | &OpCode::ConstructMatrixFromMultiple(ref ids)
                | &OpCode::ConstructStruct(ref ids)
                | &OpCode::ConstructArray(ref ids) => {
                    let result = result.unwrap();
                    target.construct_multi(block_result, result.id, result.type_id, ids)
                }

                &OpCode::Load(pointer) => target.load(block_result, result.unwrap().id, pointer),
                &OpCode::Store(pointer, value) => {
                    debug_assert!(result.is_none());
                    target.store(block_result, pointer, value);
                }

                &OpCode::Call(function_id, ref params) => {
                    target.call(block_result, result.map(|id| id.id), function_id, params)
                }

                &OpCode::Unary(unary_op, id) => {
                    target.unary(block_result, result.unwrap().id, unary_op, id)
                }
                &OpCode::Binary(binary_op, lhs, rhs) => {
                    target.binary(block_result, result.unwrap().id, binary_op, lhs, rhs)
                }
                _ => panic!("Internal error: unexpected op when generating AST"),
            }
        });
    }

    fn generate_branch_instruction<T: Target>(
        &mut self,
        block_result: &mut T::BlockResult,
        op: &OpCode,
        loop_condition_block_result: Option<T::BlockResult>,
        block1_result: Option<T::BlockResult>,
        block2_result: Option<T::BlockResult>,
        case_block_results: Vec<T::BlockResult>,
        target: &mut T,
    ) {
        match op {
            &OpCode::Discard => target.branch_discard(block_result),
            &OpCode::Return(id) => target.branch_return(block_result, id),
            &OpCode::Break => target.branch_break(block_result),
            &OpCode::Continue => target.branch_continue(block_result),
            &OpCode::If(id) => target.branch_if(block_result, id, block1_result, block2_result),
            &OpCode::Loop => {
                target.branch_loop(block_result, loop_condition_block_result, block1_result)
            }
            &OpCode::DoLoop => target.branch_do_loop(block_result, block1_result),
            &OpCode::LoopIf(id) => target.branch_loop_if(block_result, id),
            &OpCode::Switch(id, ref case_ids) => {
                target.branch_switch(block_result, id, case_ids, case_block_results)
            }
            // Passthrough, NextBlock and Merge translate to nothing in GLSL
            &OpCode::Passthrough | &OpCode::NextBlock | &OpCode::Merge(None) => (),
            // Merge with a value should have been eliminated by astify.
            &OpCode::Merge(Some(_)) => panic!(
                "Internal error: unexpected merge instruction with value when generating AST"
            ),
            _ => panic!("Internal error: unexpected branch op when generating AST"),
        }
    }
}
