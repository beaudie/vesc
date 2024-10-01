// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TODO: For now, disable unused warnings because nothing is exported
#![allow(dead_code)]

mod ast;
mod builder;
mod debug;
mod instruction;
mod ir;
mod output;
mod transform;
mod traverser;
mod util;
mod validator;
use std::collections::{HashMap, HashSet};
use std::fmt::Write;

#[cfg(test)]
mod tests {
    use super::ir::*;
    use super::*;
    use std::mem;

    fn gen_sample_vs_ir() -> IR {
        let mut builder = builder::Builder::new(ShaderType::Vertex);

        // Emulate creating an IR from the following GLSL
        //
        //     #extension GL_EXT_shader_io_blocks : require
        //     #extension GL_EXT_shader_non_constant_global_initializers : require
        //
        //     struct S {
        //         mat2x4 m[2];
        //         mediump sampler2D s;
        //     };
        //     uniform S u[3];
        //
        //     out Output {
        //         invariant vec4 v;
        //     };
        //
        //     vec4 global = u[0].m[1].y > 0.0 ? u[1].m[0] + u[2].m[0] : u[0].m[1];
        //     float globalY = u[0].m[1].z;
        //     lowp float global_one = 1.0;
        //
        //     vec4 f(mat2x4 m, out vec4 vOut, uint l) {
        //         vOut = m[1] + float(l);
        //         return m[0];
        //     }
        //
        //     void main() {
        //         gl_Position = f(u[2].m[0], v, u.length());
        //         gl_Position += global;
        //
        //         for (float a = 1.0; !(a < 0.0 || a > global_one); a /= 2.0) {
        //             if (v.x > 0.0) {
        //                 vec4 g = vec4(global.zw * 2.0, global.xzy, globalY);
        //                 gl_Position.zy += g.xw;
        //
        //                 bool should_break = v.y < 1.0;
        //                 if (!!should_break) {
        //                     break;
        //                 }
        //             } else {
        //                 gl_Position /= globalY;
        //                 break;
        //             }
        //         }
        //
        //         float b = 2.0;
        //         do {
        //            vec4 v2 = u[2].m[1] * v;
        //            b -= v2.x;
        //            if (vec2(global.y, global.z), b > 5.0 && b < 6.0) {
        //                return;
        //            }
        //         } while (b > 1.0);
        //
        //         uint i = uint(b);
        //
        //         switch (i) {
        //             case 10:
        //             case 11:
        //                 gl_Position.z = 0.0;
        //                 // passthrough
        //             default:
        //                 gl_Position.z /= gl_Position.w;
        //                 break;
        //              case 1:
        //                 v.y = -1.0;
        //         }
        //     }
        //

        //     struct S {
        //         mat2x4 m[2];
        //         mediump sampler2D s;
        //     };
        let bool_type = TYPE_ID_BOOL;
        let float_type = TYPE_ID_FLOAT;
        let vec4_type = builder.ir().meta.get_vector_type_id(BasicType::Float, 4);
        let mat2x4_type = builder.ir().meta.get_matrix_type_id(2, 4);
        let sampler2d_type = builder.ir().meta.get_image_type_id(
            ImageBasicType::Float,
            ImageType {
                dimension: ImageDimension::D2,
                is_sampled: true,
                is_array: false,
                is_ms: false,
                is_shadow: false,
            },
        );
        let mat2x4_array2_type = builder.ir().meta.get_array_type_id(mat2x4_type, 2);
        let s_type = builder.ir().meta.get_struct_type_id(
            Name::new_interface("S"),
            vec![
                Field::new(
                    Name::new_interface("m"),
                    mat2x4_array2_type,
                    Precision::High,
                    Decorations::new_none(),
                ),
                Field::new(
                    Name::new_interface("s"),
                    sampler2d_type,
                    Precision::Medium,
                    Decorations::new_none(),
                ),
            ],
            StructSpecialization::Struct,
        );

        //     uniform S u[3];
        let s_array3_type = builder.ir().meta.get_array_type_id(s_type, 3);
        let u_var = builder.declare_interface_variable(
            "u",
            s_array3_type,
            Precision::NotApplicable,
            Decorations::new(vec![Decoration::Uniform]),
        );

        //     out Output {
        //         invariant vec4 v;
        //     };
        let output_type = builder.ir().meta.get_struct_type_id(
            Name::new_interface("Output"),
            vec![Field::new(
                Name::new_interface("v"),
                vec4_type,
                Precision::High,
                Decorations::new(vec![Decoration::Invariant]),
            )],
            StructSpecialization::InterfaceBlock,
        );
        let output_var = builder.declare_interface_variable(
            "",
            output_type,
            Precision::NotApplicable,
            Decorations::new(vec![Decoration::Output]),
        );

        //     vec4 global = u[0].m[1].y > 0.0 ? u[1].m[0] + u[2].m[0] : u[0].m[1];
        let global_var = builder.declare_temp_variable(
            "global",
            vec4_type,
            Precision::High,
            Decorations::new_none(),
        );

        // u[0].m[1].y > 0.0
        builder.push_variable(u_var);
        builder.push_constant_uint(0);
        builder.index();
        builder.struct_field(0);
        builder.push_constant_uint(1);
        builder.index();
        builder.vector_component(1);

        builder.push_constant_float(0.0);
        builder.greater_than();

        // u[1].m[0] + u[2].m[0]
        builder.begin_ternary_true_expression();
        builder.push_variable(u_var);
        builder.push_constant_uint(1);
        builder.index();
        builder.struct_field(0);
        builder.push_constant_uint(0);
        builder.index();

        builder.push_variable(u_var);
        builder.push_constant_uint(2);
        builder.index();
        builder.struct_field(0);
        builder.push_constant_uint(0);
        builder.index();

        builder.add();
        builder.end_ternary_true_expression();

        // u[0].m[1]
        builder.begin_ternary_false_expression();
        builder.push_variable(u_var);
        builder.push_constant_uint(0);
        builder.index();
        builder.struct_field(0);
        builder.push_constant_uint(1);
        builder.index();
        builder.end_ternary_false_expression();
        builder.end_ternary();

        builder.initialize(global_var);

        //     float globalY = u[0].m[1].z;
        let globaly_var = builder.declare_temp_variable(
            "globalY",
            float_type,
            Precision::High,
            Decorations::new_none(),
        );

        builder.push_variable(u_var);
        builder.push_constant_uint(0);
        builder.index();
        builder.struct_field(0);
        builder.push_constant_uint(1);
        builder.index();
        builder.vector_component(2);

        builder.initialize(globaly_var);

        //     lowp float global_one = 1.0;
        let global_one_var = builder.declare_temp_variable(
            "global_one",
            float_type,
            Precision::Low,
            Decorations::new_none(),
        );

        builder.push_constant_float(1.0);
        builder.initialize(global_one_var);

        //     vec4 f(mat2x4 m, out vec4 vOut, uint l) {
        let f_func = builder.new_function("f", vec4_type, Precision::High);
        let uint_type = TYPE_ID_UINT;

        let f_params = vec![
            builder.declare_function_param(
                "m",
                mat2x4_type,
                Precision::High,
                FunctionParamDirection::Input,
            ),
            builder.declare_function_param(
                "vOut",
                vec4_type,
                Precision::High,
                FunctionParamDirection::Output,
            ),
            builder.declare_function_param(
                "l",
                uint_type,
                Precision::High,
                FunctionParamDirection::Input,
            ),
        ];
        let f_m_var = f_params[0].variable_id;
        let f_vout_var = f_params[1].variable_id;
        let f_l_var = f_params[2].variable_id;
        builder.begin_function(f_func, f_params);

        //         vOut = m[1] + float(l);
        builder.push_variable(f_vout_var);
        builder.push_variable(f_m_var);
        builder.push_constant_uint(1);
        builder.index();

        builder.push_variable(f_l_var);
        builder.construct(float_type, 1);
        builder.add();

        builder.store();
        builder.end_statement_with_value();

        //         return m[0];
        builder.push_variable(f_m_var);
        builder.push_constant_uint(0);
        builder.index();
        builder.branch_return_value();

        //     }
        builder.end_function();

        //     void main() {
        let void_type = TYPE_ID_VOID;
        let main_func = builder.new_function("main", void_type, Precision::NotApplicable);

        builder.begin_function(main_func, vec![]);

        //         gl_Position = f(u[2].m[0], v, u.length());
        let gl_position_var = builder.declare_builtin_variable(
            BuiltIn::Position,
            vec4_type,
            Precision::High,
            Decorations::new_none(),
        );

        builder.push_variable(gl_position_var);

        // u[2].m[0], v, u.length()
        builder.push_variable(u_var);
        builder.push_constant_uint(2);
        builder.index();
        builder.struct_field(0);
        builder.push_constant_uint(0);
        builder.index();

        builder.push_variable(output_var);
        builder.struct_field(0);

        builder.push_variable(u_var);
        builder.array_length();

        // gl_Position = f(...)
        builder.call_function(f_func);
        builder.store();
        builder.end_statement_with_value();

        //         gl_Position += global;
        builder.push_variable(gl_position_var);
        builder.push_variable(global_var);
        builder.add_assign();
        builder.end_statement_with_value();

        //         for (float a = 1.0; !(a < 0.0 || a > global_one); a /= 2.0) {

        // float a = 1.0
        let a_var = builder.declare_temp_variable(
            "a",
            float_type,
            Precision::High,
            Decorations::new_none(),
        );

        builder.push_constant_float(1.0);
        builder.initialize(a_var);

        // !(a < 0.0 || a > global_one)
        builder.begin_loop_condition();

        builder.push_variable(a_var);
        builder.push_constant_float(0.0);
        builder.less_than();

        builder.begin_short_circuit_or();
        builder.push_variable(a_var);
        builder.push_variable(global_one_var);
        builder.greater_than();
        builder.end_short_circuit_or();

        builder.logical_not();
        builder.end_loop_condition();

        // a /= 2.0
        builder.push_variable(a_var);
        builder.push_constant_float(2.0);
        builder.div_assign();
        builder.end_statement_with_value();
        builder.end_loop_continue();

        //             if (v.x > 0.0) {
        builder.push_variable(output_var);
        builder.struct_field(0);
        builder.vector_component(0);
        builder.push_constant_float(0.0);
        builder.greater_than();

        builder.begin_if_true_block();

        //                 vec4 g = vec4(global.zw * 2.0, global.xzy, globalY);
        let g_var =
            builder.declare_temp_variable("g", vec4_type, Precision::High, Decorations::new_none());

        builder.push_variable(global_var);
        builder.vector_component_multi(vec![2, 3]);
        builder.push_constant_float(2.0);
        builder.mul();

        builder.push_variable(global_var);
        builder.vector_component_multi(vec![0, 2, 1]);

        builder.push_variable(globaly_var);

        builder.construct(vec4_type, 3);
        builder.initialize(g_var);

        //                 gl_Position.zy += g.xw;
        builder.push_variable(gl_position_var);
        builder.vector_component_multi(vec![2, 1]);
        builder.push_variable(g_var);
        builder.vector_component_multi(vec![0, 3]);
        builder.add_assign();
        builder.end_statement_with_value();

        //                 bool should_break = v.y < 1.0;
        let should_break_var = builder.declare_temp_variable(
            "should_break",
            bool_type,
            Precision::NotApplicable,
            Decorations::new_none(),
        );

        builder.push_variable(output_var);
        builder.struct_field(0);
        builder.vector_component(1);
        builder.push_constant_float(1.0);
        builder.less_than();
        builder.initialize(should_break_var);

        //                 if (!!should_break) {
        builder.push_variable(should_break_var);
        builder.logical_not();
        builder.logical_not();
        builder.begin_if_true_block();

        //                     break;
        builder.branch_break();
        builder.end_if_true_block();

        //                 }
        builder.end_if();

        //             } else {
        builder.end_if_true_block();
        builder.begin_if_false_block();

        //                 gl_Position /= globalY;
        builder.push_variable(gl_position_var);
        builder.push_variable(globaly_var);
        builder.div_assign();
        builder.end_statement_with_value();

        //                 break;
        builder.branch_break();

        //             }
        builder.end_if_false_block();
        builder.end_if();

        //         }
        builder.end_loop();

        //         float b = 2.0;
        let b_var = builder.declare_temp_variable(
            "b",
            float_type,
            Precision::High,
            Decorations::new_none(),
        );

        builder.push_constant_float(2.0);
        builder.initialize(b_var);

        //         do {
        builder.begin_do_loop();

        //            vec4 v2 = u[2].m[1] * v;
        let v2_var = builder.declare_temp_variable(
            "v2",
            vec4_type,
            Precision::High,
            Decorations::new_none(),
        );

        builder.push_variable(u_var);
        builder.push_constant_uint(2);
        builder.index();
        builder.struct_field(0);
        builder.push_constant_uint(1);
        builder.index();

        builder.push_variable(output_var);
        builder.struct_field(0);

        builder.matrix_times_vector();
        builder.initialize(v2_var);

        //            b -= v2.x;
        builder.push_variable(b_var);
        builder.push_variable(v2_var);
        builder.vector_component(0);
        builder.sub_assign();
        builder.end_statement_with_value();

        //            if (vec2(global.y, global.z), b > 5.0 && b < 6.0) {
        let vec2_type = builder.ir().meta.get_vector_type_id(BasicType::Float, 2);

        builder.push_variable(global_var);
        builder.vector_component(1);
        builder.push_variable(global_var);
        builder.vector_component(2);
        builder.construct(vec2_type, 2);
        builder.end_statement_with_value();

        builder.push_variable(b_var);
        builder.push_constant_float(5.0);
        builder.greater_than();

        builder.begin_short_circuit_and();
        builder.push_variable(b_var);
        builder.push_constant_float(6.0);
        builder.less_than();
        builder.end_short_circuit_and();

        builder.begin_if_true_block();

        //                return;
        builder.branch_return();
        builder.end_if_true_block();

        //            }
        builder.end_if();

        //         } while (b > 1.0);
        builder.begin_do_loop_condition();
        builder.push_variable(b_var);
        builder.push_constant_float(1.0);
        builder.greater_than();
        builder.end_do_loop();

        //         uint i = uint(b);
        let i_var =
            builder.declare_temp_variable("i", uint_type, Precision::High, Decorations::new_none());

        builder.push_variable(b_var);
        builder.construct(uint_type, 1);
        builder.initialize(i_var);

        //         switch (i) {
        builder.push_variable(i_var);
        builder.begin_switch();

        //             case 10:
        builder.push_constant_uint(10);
        builder.begin_case();
        builder.end_case();

        //             case 11:
        builder.push_constant_uint(11);
        builder.begin_case();

        //                 gl_Position.z = 0.0;
        builder.push_variable(gl_position_var);
        builder.vector_component(2);
        builder.push_constant_float(0.0);
        builder.store();
        builder.end_statement_with_value();

        //                 // passthrough
        builder.end_case();

        //             default:
        builder.begin_default();

        //                 gl_Position.z /= gl_Position.w;
        builder.push_variable(gl_position_var);
        builder.vector_component(2);
        builder.push_variable(gl_position_var);
        builder.vector_component(3);
        builder.div_assign();
        builder.end_statement_with_value();

        //                 break;
        builder.branch_break();
        builder.end_case();

        //              case 1:
        builder.push_constant_uint(1);
        builder.begin_case();

        //                 v.y = -1.0;
        builder.push_variable(output_var);
        builder.struct_field(0);
        builder.vector_component(1);
        builder.push_constant_float(-1.0);
        builder.store();
        builder.end_statement_with_value();
        builder.end_case();

        //         }
        builder.end_switch();

        //     }
        builder.end_function();

        builder.finish();

        builder.take_ir()
    }

    #[test]
    fn dump() {
        let ir = gen_sample_vs_ir();
        debug::dump(&ir);
    }

    struct TransformState<'a> {
        ir_meta: &'a mut IRMeta,

        // The ID of structs that have a sampler.
        structs_with_sampler: HashSet<TypeId>,
        // A map of uniform struct variables + chain of fields that lead to a sampler to the
        // sampler declaration that replaces it.
        replacement_samplers: HashMap<(VariableId, Vec<u32>), VariableId>,
    }

    fn transform_add(
        state: &mut TransformState,
        lhs: TypedId,
        rhs: TypedId,
        result: TypedRegisterId,
    ) -> Vec<traverser::Transform> {
        let mut transforms = vec![];

        // Replace lhs+rhs with lhs+(-rhs)
        let negated_rhs = traverser::add_typed_instruction(
            &mut transforms,
            instruction::make!(negate, state.ir_meta, rhs),
        );
        traverser::add_typed_instruction(
            &mut transforms,
            instruction::make_with_result_id!(add, state.ir_meta, result, lhs, negated_rhs),
        );

        transforms
    }

    fn transform_float_div_by_constant(
        state: &mut TransformState,
        lhs: TypedId,
        rhs: TypedId,
        result: TypedRegisterId,
    ) -> Vec<traverser::Transform> {
        if state.ir_meta.get_scalar_type(lhs.type_id) != TYPE_ID_FLOAT {
            return vec![];
        }

        match rhs.id {
            Id::Constant(_) => {
                // Calculate 1/rhs which is a constant (so it will be constant folded).
                let one = state.ir_meta.get_constant_float(1.0);
                let one = TypedId::from_constant_id(one, TYPE_ID_FLOAT);

                let mut transforms = vec![];
                let rhs_inversed = traverser::add_typed_instruction(
                    &mut transforms,
                    instruction::make!(div, state.ir_meta, one, rhs),
                );
                traverser::add_typed_instruction(
                    &mut transforms,
                    instruction::make_with_result_id!(
                        mul,
                        state.ir_meta,
                        result,
                        lhs,
                        rhs_inversed
                    ),
                );
                transforms
            }
            _ => vec![],
        }
    }

    fn transform_assign_to_gl_position(
        state: &mut TransformState,
        pointer: TypedId,
        _pointee: TypedId,
    ) -> Vec<traverser::Transform> {
        match pointer.id {
            Id::Variable(variable_id) => {
                let variable = state.ir_meta.get_variable(variable_id);
                if let Some(built_in) = variable.built_in {
                    if built_in == BuiltIn::Position {
                        let zero = state.ir_meta.get_constant_float(0.0);
                        let zero =
                            state.ir_meta.get_constant_composite(TYPE_ID_VEC4, vec![zero; 4]);
                        let zero = TypedId::from_constant_id(zero, TYPE_ID_VEC4);

                        // Append `gl_Position += vec4(0);` after every write to `gl_Position`
                        let mut transforms = vec![traverser::Transform::Keep];
                        let position = traverser::add_typed_instruction(
                            &mut transforms,
                            instruction::make!(load, state.ir_meta, pointer),
                        );
                        let sum = traverser::add_typed_instruction(
                            &mut transforms,
                            instruction::make!(add, state.ir_meta, position, zero),
                        );
                        traverser::add_void_instruction(
                            &mut transforms,
                            instruction::make!(store, state.ir_meta, pointer, sum),
                        );
                        transforms
                    } else {
                        vec![]
                    }
                } else {
                    vec![]
                }
            }
            _ => vec![],
        }
    }

    fn transform_logical_not(
        state: &mut TransformState,
        operand: TypedId,
        result: TypedRegisterId,
    ) -> Vec<traverser::Transform> {
        let operand_instruction = state.ir_meta.get_instruction(operand.id.get_register());
        match operand_instruction.op {
            OpCode::Binary(BinaryOpCode::Equal, lhs, rhs) => traverser::single_typed_instruction(
                instruction::make_with_result_id!(not_equal, state.ir_meta, result, lhs, rhs),
            ),
            OpCode::Binary(BinaryOpCode::NotEqual, lhs, rhs) => {
                traverser::single_typed_instruction(instruction::make_with_result_id!(
                    equal,
                    state.ir_meta,
                    result,
                    lhs,
                    rhs
                ))
            }
            OpCode::Binary(BinaryOpCode::LessThan, lhs, rhs) => {
                traverser::single_typed_instruction(instruction::make_with_result_id!(
                    greater_than_equal,
                    state.ir_meta,
                    result,
                    lhs,
                    rhs
                ))
            }
            OpCode::Binary(BinaryOpCode::GreaterThan, lhs, rhs) => {
                traverser::single_typed_instruction(instruction::make_with_result_id!(
                    less_than_equal,
                    state.ir_meta,
                    result,
                    lhs,
                    rhs
                ))
            }
            OpCode::Binary(BinaryOpCode::LessThanEqual, lhs, rhs) => {
                traverser::single_typed_instruction(instruction::make_with_result_id!(
                    greater_than,
                    state.ir_meta,
                    result,
                    lhs,
                    rhs
                ))
            }
            OpCode::Binary(BinaryOpCode::GreaterThanEqual, lhs, rhs) => {
                traverser::single_typed_instruction(instruction::make_with_result_id!(
                    less_than,
                    state.ir_meta,
                    result,
                    lhs,
                    rhs
                ))
            }
            OpCode::Unary(UnaryOpCode::LogicalNot, operand) => traverser::single_typed_instruction(
                instruction::make_with_result_id!(alias, state.ir_meta, result, operand),
            ),
            _ => vec![],
        }
    }

    fn transform_instruction(
        state: &mut TransformState,
        instruction: &BlockInstruction,
    ) -> Vec<traverser::Transform> {
        let (opcode, result) = instruction.get_op_and_result(state.ir_meta);

        match opcode {
            &OpCode::Binary(BinaryOpCode::Add, lhs, rhs) => {
                transform_add(state, lhs, rhs, result.unwrap())
            }
            &OpCode::Binary(BinaryOpCode::Div, lhs, rhs) => {
                transform_float_div_by_constant(state, lhs, rhs, result.unwrap())
            }
            &OpCode::Store(pointer, pointee) => {
                transform_assign_to_gl_position(state, pointer, pointee)
            }
            &OpCode::Unary(UnaryOpCode::LogicalNot, operand) => {
                transform_logical_not(state, operand, result.unwrap())
            }
            _ => vec![],
        }
    }

    fn is_block_entirely_merge(block: &Block, expect: ConstantId) -> bool {
        if block.instructions.len() != 1 {
            return false;
        }

        if let BlockInstruction::Void(OpCode::Merge(Some(value))) =
            block.instructions.first().unwrap()
        {
            value.id.get_constant().map(|value| value == expect).unwrap_or(false)
        } else {
            false
        }
    }

    fn apply_logical_not_to_branch_instruction<MakeInst>(
        ir_meta: &mut IRMeta,
        block: &mut Block,
        value: TypedId,
        make_inst: MakeInst,
    ) where
        MakeInst: FnOnce(TypedId) -> OpCode,
    {
        block.instructions.pop();
        let not_value = instruction::make!(logical_not, ir_meta, value);

        match not_value {
            instruction::Result::Constant(constant_id, type_id) => {
                block.add_void_instruction(make_inst(TypedId::from_constant_id(
                    constant_id,
                    type_id,
                )));
            }
            instruction::Result::Register(id) => {
                block.add_typed_instruction(id.id);
                block.add_void_instruction(make_inst(TypedId::from_register_id(id)));
            }
            _ => panic!("Should have created value"),
        }
    }

    fn apply_logical_not_to_merge_instruction(ir_meta: &mut IRMeta, block: &mut Block) {
        if let &BlockInstruction::Void(OpCode::Merge(Some(value))) =
            block.instructions.last().unwrap()
        {
            apply_logical_not_to_branch_instruction(ir_meta, block, value, |id| {
                OpCode::Merge(Some(id))
            });
        } else {
            panic!("Should have been merge");
        }
    }

    fn apply_logical_not_to_if_instruction(ir_meta: &mut IRMeta, block: &mut Block) {
        if let &BlockInstruction::Void(OpCode::If(value)) = block.instructions.last().unwrap() {
            apply_logical_not_to_branch_instruction(ir_meta, block, value, OpCode::If);
        } else {
            panic!("Should have been if");
        }
    }

    fn check_and_apply_logical_not_to_short_circuit_op<'block>(
        _transformer: &mut traverser::Transformer,
        state: &mut TransformState,
        block: &'block mut Block,
    ) -> &'block mut Block {
        // Look ahead for the following:
        //
        // - The merge block has a merge input
        // - The true block has a single Merge(Some(CONSTANT_ID_TRUE)) or the false block has a
        //   single Merge(Some(CONSTANT_ID_FALSE))
        // - The first instruction of the merge block is a LogicalNot of the merge input.
        //
        // This is detecting `!(a || b)` and `!(a && b)`.  Transform this effectively to `!a && !b`
        // and `!a || !b` respectively by changing the short-circuit loop.
        if block.merge_block.is_none() {
            return block;
        }
        let merge_block = block.merge_block.as_mut().unwrap();

        if merge_block.input.is_none() {
            return block;
        }
        let input_id = merge_block.input.unwrap();
        let input = TypedId::from_register_id(input_id);

        // Check if first instruction of merge block is !input
        let (op, result) =
            merge_block.instructions.first().unwrap().get_op_and_result(state.ir_meta);
        if let OpCode::Unary(UnaryOpCode::LogicalNot, operand) = op {
            if operand.id != input.id {
                return block;
            }
        } else {
            return block;
        }

        // Remove the not operation, and replace it with an Alias of the input block id with the
        // result of the logical not.
        let alias_id =
            state.ir_meta.new_register(OpCode::Alias(input), input.type_id, input.precision);
        state.ir_meta.replace_instruction(result.unwrap().id, alias_id.id);
        merge_block.instructions[0] = BlockInstruction::new_typed(result.unwrap().id);

        // Check if true block is just true, or false block is just false.
        let is_true_block_true = block
            .block1
            .as_ref()
            .map(|block| is_block_entirely_merge(block, CONSTANT_ID_TRUE))
            .unwrap_or(false);
        let is_false_block_false = block
            .block2
            .as_ref()
            .map(|block| is_block_entirely_merge(block, CONSTANT_ID_FALSE))
            .unwrap_or(false);

        // Move the logical not to inside the blocks
        block
            .block1
            .as_mut()
            .map(|block| apply_logical_not_to_merge_instruction(state.ir_meta, block));
        block
            .block2
            .as_mut()
            .map(|block| apply_logical_not_to_merge_instruction(state.ir_meta, block));

        // If either block is a constant, swap the blocks and negate the if condition.
        if is_true_block_true || is_false_block_false {
            mem::swap(&mut block.block1, &mut block.block2);
            apply_logical_not_to_if_instruction(state.ir_meta, block);
        }

        block
    }

    fn apply_logical_not_to_short_circuit_ops(
        transformer: &mut traverser::Transformer,
        state: &mut TransformState,
        block: &mut Block,
    ) {
        transformer.for_each_block(
            state,
            block,
            &check_and_apply_logical_not_to_short_circuit_op,
            &|_, _, block| block,
        );
    }

    fn do_transforms(ir: &mut IR) {
        let mut transformer = traverser::Transformer::new();

        let mut first_params = Vec::new();
        ir.meta.all_functions_mut().iter_mut().for_each(|function| {
            function.params.first().inspect(|param| first_params.push(param.variable_id));
        });
        for id in first_params {
            let var = ir.meta.get_variable_mut(id);
            if var.precision == Precision::High {
                var.precision = Precision::Medium;
            }
        }

        let mut state = TransformState {
            ir_meta: &mut ir.meta,
            structs_with_sampler: HashSet::new(),
            replacement_samplers: HashMap::new(),
        };

        // In a first pass, move ! on short-circuiting instructions to the operands.
        transformer.for_each_function(
            &mut state,
            &mut ir.function_entries,
            &|transformer, state, block| {
                apply_logical_not_to_short_circuit_ops(transformer, state, block)
            },
        );

        // Bogus transformation that does the following:
        //
        // - Change the first parameter of every function to `mediump`.
        // - Change a-b to a+(-b)
        // - Change float division by constant C to multiply with constant 1/C.
        // - Follow every assignment to gl_Position with a `gl_Position += vec4(0);`
        // - Change `!(a || b)` to `!a && !b`. At the same time, change `!(a < b)` with `a >= b`
        //   etc.
        transformer.for_each_instruction(
            &mut state,
            &mut ir.function_entries,
            &|state, instruction| transform_instruction(state, instruction),
        );
    }

    #[test]
    fn transform() {
        let mut ir = gen_sample_vs_ir();
        do_transforms(&mut ir);
        debug::dump(&ir);
    }

    #[test]
    fn to_ast() {
        let mut ir = gen_sample_vs_ir();
        //do_transforms(&mut ir);

        transform::dealias::run(&mut ir);
        transform::astify::run(&mut ir);
        debug::dump(&ir);

        let mut glsl_gen = output::glsl::Generator::new();
        let mut generator = ast::Generator::new(&ir.meta);
        generator.generate(&ir.function_entries, &mut glsl_gen);
    }
}
