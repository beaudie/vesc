// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Helper functions to create instructions given parameters.  The result type and precision is
// automatically deduced, and const-folding is done if possible.

use crate::ir::*;
use crate::*;

// Helper functions that perform constant folding per instruction
mod const_fold {
    use crate::ir::*;

    fn apply_unary_componentwise<FloatOp, IntOp, UintOp, BoolOp>(
        ir_meta: &mut IRMeta,
        constant_id: ConstantId,
        result_type_id: TypeId,
        float_op: FloatOp,
        int_op: IntOp,
        uint_op: UintOp,
        bool_op: BoolOp,
    ) -> ConstantId
    where
        FloatOp: Fn(f32) -> f32 + Copy,
        IntOp: Fn(i32) -> i32 + Copy,
        UintOp: Fn(u32) -> u32 + Copy,
        BoolOp: Fn(bool) -> bool + Copy,
    {
        let constant = ir_meta.get_constant(constant_id);
        debug_assert!(constant.type_id == result_type_id);
        let type_id = constant.type_id;
        let element_type_id = ir_meta.get_type(type_id).get_element_type_id().unwrap_or(type_id);

        match &constant.value {
            &ConstantValue::Float(f) => ir_meta.get_constant_float(float_op(f)),
            &ConstantValue::Int(i) => ir_meta.get_constant_int(int_op(i)),
            &ConstantValue::Uint(u) => ir_meta.get_constant_uint(uint_op(u)),
            &ConstantValue::Bool(b) => ir_meta.get_constant_bool(bool_op(b)),
            ConstantValue::YuvCsc(_) => {
                panic!("Internal error: Ops not allowed on YUV CSC constants")
            }
            ConstantValue::Composite(components) => {
                let mapped = components
                    .clone()
                    .iter()
                    .map(|&component_id| {
                        apply_unary_componentwise(
                            ir_meta,
                            component_id,
                            element_type_id,
                            float_op,
                            int_op,
                            uint_op,
                            bool_op,
                        )
                    })
                    .collect();
                ir_meta.get_constant_composite(type_id, mapped)
            }
        }
    }

    fn apply_binary_to_scalars<FloatOp, IntOp, UintOp, BoolOp>(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        _result_type_id: TypeId,
        float_op: FloatOp,
        int_op: IntOp,
        uint_op: UintOp,
        bool_op: BoolOp,
    ) -> ConstantId
    where
        FloatOp: Fn(f32, f32) -> f32 + Copy,
        IntOp: Fn(i32, i32) -> i32 + Copy,
        UintOp: Fn(u32, u32) -> u32 + Copy,
        BoolOp: Fn(bool, bool) -> bool + Copy,
    {
        let lhs = ir_meta.get_constant(lhs_constant_id);
        let rhs = ir_meta.get_constant(rhs_constant_id);

        match (&lhs.value, &rhs.value) {
            (&ConstantValue::Float(f1), &ConstantValue::Float(f2)) => {
                ir_meta.get_constant_float(float_op(f1, f2))
            }
            (&ConstantValue::Int(i1), &ConstantValue::Int(i2)) => {
                ir_meta.get_constant_int(int_op(i1, i2))
            }
            (&ConstantValue::Uint(u1), &ConstantValue::Uint(u2)) => {
                ir_meta.get_constant_uint(uint_op(u1, u2))
            }
            (&ConstantValue::Bool(b1), &ConstantValue::Bool(b2)) => {
                ir_meta.get_constant_bool(bool_op(b1, b2))
            }
            (&ConstantValue::YuvCsc(_), &ConstantValue::YuvCsc(_)) => {
                panic!("Internal error: Ops not allowed on YUV CSC constants")
            }
            _ => panic!("Internal error: Expected scalars when constant folding a binary op"),
        }
    }

    fn apply_binary_componentwise_scalar_rhs<FloatOp, IntOp, UintOp, BoolOp>(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        result_type_id: TypeId,
        float_op: FloatOp,
        int_op: IntOp,
        uint_op: UintOp,
        bool_op: BoolOp,
    ) -> ConstantId
    where
        FloatOp: Fn(f32, f32) -> f32 + Copy,
        IntOp: Fn(i32, i32) -> i32 + Copy,
        UintOp: Fn(u32, u32) -> u32 + Copy,
        BoolOp: Fn(bool, bool) -> bool + Copy,
    {
        let lhs = ir_meta.get_constant(lhs_constant_id);

        if let ConstantValue::Composite(lhs_components) = &lhs.value {
            let result_element_type_id =
                ir_meta.get_type(result_type_id).get_element_type_id().unwrap_or(result_type_id);

            let components = lhs_components
                .clone()
                .iter()
                .map(|&lhs_component_id| {
                    apply_binary_componentwise_scalar_rhs(
                        ir_meta,
                        lhs_component_id,
                        rhs_constant_id,
                        result_element_type_id,
                        float_op,
                        int_op,
                        uint_op,
                        bool_op,
                    )
                })
                .collect();
            ir_meta.get_constant_composite(result_type_id, components)
        } else {
            apply_binary_to_scalars(
                ir_meta,
                lhs_constant_id,
                rhs_constant_id,
                result_type_id,
                float_op,
                int_op,
                uint_op,
                bool_op,
            )
        }
    }

    fn apply_binary_componentwise_scalar_lhs<FloatOp, IntOp, UintOp, BoolOp>(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        result_type_id: TypeId,
        float_op: FloatOp,
        int_op: IntOp,
        uint_op: UintOp,
        bool_op: BoolOp,
    ) -> ConstantId
    where
        FloatOp: Fn(f32, f32) -> f32 + Copy,
        IntOp: Fn(i32, i32) -> i32 + Copy,
        UintOp: Fn(u32, u32) -> u32 + Copy,
        BoolOp: Fn(bool, bool) -> bool + Copy,
    {
        let rhs = ir_meta.get_constant(rhs_constant_id);

        if let ConstantValue::Composite(rhs_components) = &rhs.value {
            let result_element_type_id =
                ir_meta.get_type(result_type_id).get_element_type_id().unwrap_or(result_type_id);

            let components = rhs_components
                .clone()
                .iter()
                .map(|&rhs_component_id| {
                    apply_binary_componentwise_scalar_lhs(
                        ir_meta,
                        lhs_constant_id,
                        rhs_component_id,
                        result_element_type_id,
                        float_op,
                        int_op,
                        uint_op,
                        bool_op,
                    )
                })
                .collect();
            ir_meta.get_constant_composite(result_type_id, components)
        } else {
            apply_binary_to_scalars(
                ir_meta,
                lhs_constant_id,
                rhs_constant_id,
                result_type_id,
                float_op,
                int_op,
                uint_op,
                bool_op,
            )
        }
    }

    fn apply_binary_componentwise_non_scalar<FloatOp, IntOp, UintOp, BoolOp>(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        result_type_id: TypeId,
        float_op: FloatOp,
        int_op: IntOp,
        uint_op: UintOp,
        bool_op: BoolOp,
    ) -> ConstantId
    where
        FloatOp: Fn(f32, f32) -> f32 + Copy,
        IntOp: Fn(i32, i32) -> i32 + Copy,
        UintOp: Fn(u32, u32) -> u32 + Copy,
        BoolOp: Fn(bool, bool) -> bool + Copy,
    {
        let lhs = ir_meta.get_constant(lhs_constant_id);
        let rhs = ir_meta.get_constant(rhs_constant_id);

        if let (
            ConstantValue::Composite(lhs_components),
            ConstantValue::Composite(rhs_components),
        ) = (&lhs.value, &rhs.value)
        {
            let result_element_type_id =
                ir_meta.get_type(result_type_id).get_element_type_id().unwrap_or(result_type_id);

            let components = lhs_components
                .clone()
                .iter()
                .zip(rhs_components.clone().iter())
                .map(|(&lhs_component_id, &rhs_component_id)| {
                    apply_binary_componentwise_non_scalar(
                        ir_meta,
                        lhs_component_id,
                        rhs_component_id,
                        result_element_type_id,
                        float_op,
                        int_op,
                        uint_op,
                        bool_op,
                    )
                })
                .collect();
            ir_meta.get_constant_composite(result_type_id, components)
        } else {
            apply_binary_to_scalars(
                ir_meta,
                lhs_constant_id,
                rhs_constant_id,
                result_type_id,
                float_op,
                int_op,
                uint_op,
                bool_op,
            )
        }
    }

    fn apply_binary_componentwise<FloatOp, IntOp, UintOp, BoolOp>(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        result_type_id: TypeId,
        float_op: FloatOp,
        int_op: IntOp,
        uint_op: UintOp,
        bool_op: BoolOp,
    ) -> ConstantId
    where
        FloatOp: Fn(f32, f32) -> f32 + Copy,
        IntOp: Fn(i32, i32) -> i32 + Copy,
        UintOp: Fn(u32, u32) -> u32 + Copy,
        BoolOp: Fn(bool, bool) -> bool + Copy,
    {
        let is_lhs_scalar =
            ir_meta.get_type(ir_meta.get_constant(lhs_constant_id).type_id).is_scalar();
        let is_rhs_scalar =
            ir_meta.get_type(ir_meta.get_constant(rhs_constant_id).type_id).is_scalar();

        if is_rhs_scalar {
            apply_binary_componentwise_scalar_rhs(
                ir_meta,
                lhs_constant_id,
                rhs_constant_id,
                result_type_id,
                float_op,
                int_op,
                uint_op,
                bool_op,
            )
        } else if is_lhs_scalar {
            apply_binary_componentwise_scalar_lhs(
                ir_meta,
                lhs_constant_id,
                rhs_constant_id,
                result_type_id,
                float_op,
                int_op,
                uint_op,
                bool_op,
            )
        } else {
            apply_binary_componentwise_non_scalar(
                ir_meta,
                lhs_constant_id,
                rhs_constant_id,
                result_type_id,
                float_op,
                int_op,
                uint_op,
                bool_op,
            )
        }
    }

    pub fn composite_element(
        ir_meta: &mut IRMeta,
        constant_id: ConstantId,
        index: u32,
        _result_type_id: TypeId,
    ) -> ConstantId {
        ir_meta.get_constant(constant_id).value.get_composite_elements()[index as usize]
    }

    pub fn vector_components(
        ir_meta: &mut IRMeta,
        constant_id: ConstantId,
        fields: &Vec<u32>,
        result_type_id: TypeId,
    ) -> ConstantId {
        let composite_elements = ir_meta.get_constant(constant_id).value.get_composite_elements();
        let new_elements = fields.iter().map(|&field| composite_elements[field as usize]).collect();
        ir_meta.get_constant_composite(result_type_id, new_elements)
    }

    pub fn index(
        ir_meta: &mut IRMeta,
        indexed_constant_id: ConstantId,
        index_constant_id: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        let index = ir_meta.get_constant(index_constant_id).value.get_index();
        composite_element(ir_meta, indexed_constant_id, index, result_type_id)
    }

    // Taking a list of constants, flattens them so every element is a basic type.  For example,
    // [v.xy, m2x3, ...] becomes [v.x, v.y, m[0][0], m[0][1], ...]
    fn flatten_constructor_args(ir_meta: &IRMeta, args: &Vec<ConstantId>) -> Vec<ConstantId> {
        let mut result = Vec::new();

        for &arg in args {
            let constant = ir_meta.get_constant(arg);

            if let ConstantValue::Composite(ids) = &constant.value {
                let flattened = flatten_constructor_args(ir_meta, ids);
                result.extend(flattened);
            } else {
                result.push(arg);
            }
        }

        result
    }

    // Taking a list of all-scalar constants, casts the values to the given basic_type.
    fn cast_constructor_args(
        ir_meta: &mut IRMeta,
        args: Vec<ConstantId>,
        basic_type: BasicType,
    ) -> Vec<ConstantId> {
        args.iter()
            .map(|&arg| {
                let constant = ir_meta.get_constant(arg);
                if let ConstantValue::Float(f) = constant.value {
                    match basic_type {
                        BasicType::Int => ir_meta.get_constant_int(f as i32),
                        BasicType::Uint => ir_meta.get_constant_uint(f as u32),
                        BasicType::Bool => ir_meta.get_constant_bool(f != 0.0),
                        _ => arg,
                    }
                } else if let ConstantValue::Int(i) = constant.value {
                    match basic_type {
                        BasicType::Float => ir_meta.get_constant_float(i as f32),
                        BasicType::Uint => ir_meta.get_constant_uint(i as u32),
                        BasicType::Bool => ir_meta.get_constant_bool(i != 0),
                        _ => arg,
                    }
                } else if let ConstantValue::Uint(u) = constant.value {
                    match basic_type {
                        BasicType::Float => ir_meta.get_constant_float(u as f32),
                        BasicType::Int => ir_meta.get_constant_int(u as i32),
                        BasicType::Bool => ir_meta.get_constant_bool(u != 0),
                        _ => arg,
                    }
                } else if let ConstantValue::Bool(b) = constant.value {
                    match basic_type {
                        BasicType::Float => ir_meta.get_constant_float(b.into()),
                        BasicType::Int => ir_meta.get_constant_int(b as i32),
                        BasicType::Uint => ir_meta.get_constant_uint(b as u32),
                        _ => arg,
                    }
                } else {
                    // This function is called on scalars, so `Composite` is impossible.
                    // Additionally, GLSL forbids type conversion to and from
                    // yuvCscStandardEXT.
                    arg
                }
            })
            .collect()
    }

    // Construct a vector from a scalar by replicating it.
    fn construct_vector_from_scalar(
        ir_meta: &mut IRMeta,
        arg: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        let type_info = ir_meta.get_type(result_type_id);
        let vec_size = type_info.get_vector_size().unwrap();
        let args = vec![arg; vec_size as usize];
        ir_meta.get_constant_composite(result_type_id, args)
    }

    // Construct a matrix from a scalar by setting the diagonal elements with that scalar while
    // everywhere else is filled with zeros.
    fn construct_matrix_from_scalar(
        ir_meta: &mut IRMeta,
        arg: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        let type_info = ir_meta.get_type(result_type_id);
        let &Type::Matrix(vector_type_id, column_count) = type_info else { unreachable!() };
        let &Type::Vector(_, row_count) = ir_meta.get_type(vector_type_id) else { unreachable!() };

        // Create columns where every component is 0 except the one at index = column_index.
        let columns = (0..column_count)
            .map(|column| {
                let column_components = (0..row_count)
                    .map(|row| if row == column { arg } else { CONSTANT_ID_FLOAT_ZERO })
                    .collect();
                ir_meta.get_constant_composite(vector_type_id, column_components)
            })
            .collect();
        ir_meta.get_constant_composite(result_type_id, columns)
    }

    // Construct a matrix from a matrix by starting with the identity matrix and overwriting it with
    // components from the argument.
    fn construct_matrix_from_matrix(
        ir_meta: &mut IRMeta,
        arg: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        let type_info = ir_meta.get_type(result_type_id);
        let &Type::Matrix(vector_type_id, column_count) = type_info else { unreachable!() };
        let &Type::Vector(_, row_count) = ir_meta.get_type(vector_type_id) else { unreachable!() };

        let input = ir_meta.get_constant(arg);
        let input_columns = input.value.get_composite_elements();
        let input_column_components: Vec<_> = input_columns
            .iter()
            .map(|&column_id| ir_meta.get_constant(column_id).value.get_composite_elements())
            .collect();
        let input_column_count = input_columns.len() as u32;
        let input_row_count = input_column_components[0].len() as u32;

        // Create columns where every component is taken from the input matrix, except if it's out
        // of bounds.  In that case, the component is 1 on the diagonal and 0 elsewhere.
        let columns: Vec<_> = (0..column_count)
            .map(|column| {
                (0..row_count)
                    .map(|row| {
                        if column < input_column_count && row < input_row_count {
                            input_column_components[column as usize][row as usize]
                        } else if row == column {
                            CONSTANT_ID_FLOAT_ONE
                        } else {
                            CONSTANT_ID_FLOAT_ZERO
                        }
                    })
                    .collect()
            })
            .collect();

        let columns = columns
            .into_iter()
            .map(|column_components| {
                ir_meta.get_constant_composite(vector_type_id, column_components)
            })
            .collect();
        ir_meta.get_constant_composite(result_type_id, columns)
    }

    // Construct a vector from multiple components.
    fn construct_vector_from_many(
        ir_meta: &mut IRMeta,
        args: Vec<ConstantId>,
        result_type_id: TypeId,
    ) -> ConstantId {
        ir_meta.get_constant_composite(result_type_id, args)
    }

    // Construct a matrix from multiple components.
    fn construct_matrix_from_many(
        ir_meta: &mut IRMeta,
        args: Vec<ConstantId>,
        result_type_id: TypeId,
    ) -> ConstantId {
        let type_info = ir_meta.get_type(result_type_id);
        let &Type::Matrix(vector_type_id, column_count) = type_info else { unreachable!() };
        let &Type::Vector(_, row_count) = ir_meta.get_type(vector_type_id) else { unreachable!() };

        let columns = (0..column_count)
            .map(|column| {
                let start = (column * row_count) as usize;
                let end = start + row_count as usize;
                ir_meta.get_constant_composite(vector_type_id, args[start..end].to_vec())
            })
            .collect();
        ir_meta.get_constant_composite(result_type_id, columns)
    }

    pub fn construct(
        ir_meta: &mut IRMeta,
        args: Vec<ConstantId>,
        result_type_id: TypeId,
    ) -> ConstantId {
        let type_info = ir_meta.get_type(result_type_id);

        // For arrays and struct, simply make the constant out of the arguments.
        if matches!(type_info, Type::Struct(..) | Type::Array(..)) {
            return ir_meta.get_constant_composite(result_type_id, args);
        }

        let is_vector = matches!(type_info, Type::Vector(..));
        let is_matrix = matches!(type_info, Type::Matrix(..));

        // For scalars, vectors and matrices, first flatten the components for simplicity.
        let args = flatten_constructor_args(ir_meta, &args);

        // Then cast the basic type to the type of the result, if needed.
        let basic_type =
            ir_meta.get_type(ir_meta.get_scalar_type(result_type_id)).get_scalar_basic_type();
        let args = cast_constructor_args(ir_meta, args, basic_type);

        let is_first_arg_matrix =
            matches!(ir_meta.get_type(ir_meta.get_constant(args[0]).type_id), Type::Matrix(..));

        if is_vector && args.len() == 1 {
            construct_vector_from_scalar(ir_meta, args[0], result_type_id)
        } else if is_matrix && is_first_arg_matrix {
            construct_matrix_from_matrix(ir_meta, args[0], result_type_id)
        } else if is_matrix && args.len() == 1 {
            construct_matrix_from_scalar(ir_meta, args[0], result_type_id)
        } else if is_vector {
            construct_vector_from_many(ir_meta, args, result_type_id)
        } else if is_matrix {
            construct_matrix_from_many(ir_meta, args, result_type_id)
        } else {
            // The type cast is enough to satisfy scalar constructors.
            args[0]
        }
    }

    pub fn negate(
        ir_meta: &mut IRMeta,
        constant_id: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        apply_unary_componentwise(
            ir_meta,
            constant_id,
            result_type_id,
            |f| -f,
            |i| i.wrapping_neg(),
            |u| u.wrapping_neg(),
            |_| panic!("Internal error: Cannot negate a bool"),
        )
    }
    pub fn add(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        apply_binary_componentwise(
            ir_meta,
            lhs_constant_id,
            rhs_constant_id,
            result_type_id,
            |f1, f2| f1 + f2,
            |i1, i2| i1.wrapping_add(i2),
            |u1, u2| u1.wrapping_add(u2),
            |_, _| panic!("Internal error: Cannot add bools"),
        )
    }
    pub fn sub(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        apply_binary_componentwise(
            ir_meta,
            lhs_constant_id,
            rhs_constant_id,
            result_type_id,
            |f1, f2| f1 - f2,
            |i1, i2| i1.wrapping_sub(i2),
            |u1, u2| u1.wrapping_sub(u2),
            |_, _| panic!("Internal error: Cannot subtract bools"),
        )
    }
    pub fn mul(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        apply_binary_componentwise(
            ir_meta,
            lhs_constant_id,
            rhs_constant_id,
            result_type_id,
            |f1, f2| f1 * f2,
            |i1, i2| i1.wrapping_mul(i2),
            |u1, u2| u1.wrapping_mul(u2),
            |_, _| panic!("Internal error: Cannot multiply bools"),
        )
    }
    pub fn div(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        apply_binary_componentwise(
            ir_meta,
            lhs_constant_id,
            rhs_constant_id,
            result_type_id,
            |f1, f2| f1 / f2,
            |i1, i2| i1.checked_div(i2).unwrap_or(i32::MAX),
            |u1, u2| u1.checked_div(u2).unwrap_or(u32::MAX),
            |_, _| panic!("Internal error: Cannot divide bools"),
        )
    }
    pub fn imod(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        apply_binary_componentwise(
            ir_meta,
            lhs_constant_id,
            rhs_constant_id,
            result_type_id,
            |_, _| panic!("Internal error: Cannot use % to calculate remainder of floats"),
            |i1, i2| i1.checked_rem(i2).unwrap_or(0),
            |u1, u2| u1.checked_rem(u2).unwrap_or(0),
            |_, _| panic!("Internal error: Cannot use % on bools"),
        )
    }
    pub fn dot(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        debug_assert!(result_type_id == TYPE_ID_FLOAT);

        let lhs = ir_meta.get_constant(lhs_constant_id);
        let rhs = ir_meta.get_constant(rhs_constant_id);

        let lhs_components = lhs.value.get_composite_elements();
        let rhs_components = rhs.value.get_composite_elements();

        let result = lhs_components
            .iter()
            .zip(rhs_components.iter())
            .map(|(&lhs_component_id, &rhs_component_id)| {
                ir_meta.get_const_float_value(lhs_component_id)
                    * ir_meta.get_const_float_value(rhs_component_id)
            })
            .sum();
        ir_meta.get_constant_float(result)
    }
    pub fn transpose(
        ir_meta: &mut IRMeta,
        constant_id: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        let constant = ir_meta.get_constant(constant_id);
        let columns = constant.value.get_composite_elements();
        let column_count = columns.len();
        let row_count = ir_meta.get_type(result_type_id).get_matrix_size().unwrap();

        let mut transposed_data = vec![Vec::new(); row_count as usize];

        // Create new columns out of rows of the matrix.
        columns.iter().for_each(|&column_id| {
            let column = ir_meta.get_constant(column_id).value.get_composite_elements();
            debug_assert!(column.len() == row_count as usize);
            column.iter().enumerate().for_each(|(row_index, &component)| {
                transposed_data[row_index as usize].push(component);
            });
        });

        // Create constants for these columns.
        let transposed_column_type_id =
            ir_meta.get_vector_type_id(BasicType::Float, column_count as u32);
        let transposed_column_ids = transposed_data
            .into_iter()
            .map(|data| ir_meta.get_constant_composite(transposed_column_type_id, data))
            .collect();

        // Finally, assemble the matrix itself.
        ir_meta.get_constant_composite(result_type_id, transposed_column_ids)
    }
    pub fn vector_times_matrix(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        let rhs = ir_meta.get_constant(rhs_constant_id);
        let rhs_columns = rhs.value.get_composite_elements();

        let result_column_ids = rhs_columns
            .clone()
            .iter()
            .map(|&column_id| dot(ir_meta, lhs_constant_id, column_id, TYPE_ID_FLOAT))
            .collect();
        ir_meta.get_constant_composite(result_type_id, result_column_ids)
    }
    pub fn matrix_times_vector(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        let lhs = ir_meta.get_constant(lhs_constant_id);
        let matrix_type = ir_meta.get_type(lhs.type_id);
        let column_count = matrix_type.get_matrix_size().unwrap();
        let row_count = ir_meta.get_type(result_type_id).get_vector_size().unwrap();

        // Transpose the matrix first, so `vector_times_matrix` can be reused.
        let transposed_type_id = ir_meta.get_matrix_type_id(row_count, column_count);
        let transposed = transpose(ir_meta, lhs_constant_id, transposed_type_id);
        vector_times_matrix(ir_meta, rhs_constant_id, transposed, result_type_id)
    }
    pub fn matrix_times_matrix(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        let lhs = ir_meta.get_constant(lhs_constant_id);
        let rhs = ir_meta.get_constant(rhs_constant_id);
        let lhs_type = ir_meta.get_type(lhs.type_id);
        let result_type = ir_meta.get_type(result_type_id);

        let lhs_columns = lhs.value.get_composite_elements();
        let rhs_columns = rhs.value.get_composite_elements().clone();

        let result_column_type_id = result_type.get_element_type_id().unwrap();

        // Say LHS has C1 columns of R1 components, and RHS has C2 columns of R2 components.  For
        // matrix times matrix to work, C1 == R2 must hold.  The result will have C2 columns of R1
        // components.
        let rhs_column_count = rhs_columns.len();
        let common_component_count = lhs_columns.len();
        let lhs_row_count = ir_meta.get_type(result_column_type_id).get_vector_size().unwrap();

        debug_assert!(
            ir_meta.get_type(lhs_type.get_element_type_id().unwrap()).get_vector_size().unwrap()
                == lhs_row_count
        );
        debug_assert!(result_type.get_matrix_size().unwrap() == rhs_column_count as u32);

        // The result at column c, row r, is the dot product of row r in lhs and column c in rhs.
        // Equivalently, it's the dot product of column r in transposed lhs and column c in rhs.
        // The lhs is thus transposed so the `dot` helper can be used.
        let lhs_transposed_type_id =
            ir_meta.get_matrix_type_id(lhs_row_count, common_component_count as u32);
        let lhs_transposed_id = transpose(ir_meta, lhs_constant_id, lhs_transposed_type_id);

        let lhs = ir_meta.get_constant(lhs_transposed_id);
        let lhs_columns = lhs.value.get_composite_elements().clone();

        let result_columns_ids = rhs_columns
            .iter()
            .map(|&rhs_column_id| {
                let result_column_ids = lhs_columns
                    .iter()
                    .map(|&lhs_column_id| dot(ir_meta, lhs_column_id, rhs_column_id, TYPE_ID_FLOAT))
                    .collect();
                ir_meta.get_constant_composite(result_column_type_id, result_column_ids)
            })
            .collect();
        ir_meta.get_constant_composite(result_type_id, result_columns_ids)
    }
    pub fn logical_not(
        _ir_meta: &mut IRMeta,
        constant_id: ConstantId,
        _result_type_id: TypeId,
    ) -> ConstantId {
        debug_assert!(constant_id == CONSTANT_ID_TRUE || constant_id == CONSTANT_ID_FALSE);
        if constant_id == CONSTANT_ID_FALSE { CONSTANT_ID_TRUE } else { CONSTANT_ID_FALSE }
    }
    pub fn logical_xor(
        _ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        _result_type_id: TypeId,
    ) -> ConstantId {
        debug_assert!(lhs_constant_id == CONSTANT_ID_TRUE || lhs_constant_id == CONSTANT_ID_FALSE);
        debug_assert!(rhs_constant_id == CONSTANT_ID_TRUE || rhs_constant_id == CONSTANT_ID_FALSE);

        if lhs_constant_id == rhs_constant_id { CONSTANT_ID_FALSE } else { CONSTANT_ID_TRUE }
    }
    pub fn equal(
        _ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        _result_type_id: TypeId,
    ) -> ConstantId {
        // Note: because the IR ensures constants are unique, no need for a recursive equality
        // check of the components.
        if lhs_constant_id == rhs_constant_id { CONSTANT_ID_TRUE } else { CONSTANT_ID_FALSE }
    }
    pub fn not_equal(
        _ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        _result_type_id: TypeId,
    ) -> ConstantId {
        if lhs_constant_id == rhs_constant_id { CONSTANT_ID_FALSE } else { CONSTANT_ID_TRUE }
    }
    fn compare_scalars<FloatCompare, IntCompare, UintCompare>(
        ir_meta: &IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        float_compare: FloatCompare,
        int_compare: IntCompare,
        uint_compare: UintCompare,
    ) -> ConstantId
    where
        FloatCompare: FnOnce(f32, f32) -> bool,
        IntCompare: FnOnce(i32, i32) -> bool,
        UintCompare: FnOnce(u32, u32) -> bool,
    {
        let lhs_constant = ir_meta.get_constant(lhs_constant_id);
        let rhs_constant = ir_meta.get_constant(rhs_constant_id);

        let result = match (&lhs_constant.value, &rhs_constant.value) {
            (&ConstantValue::Float(lhs_f), &ConstantValue::Float(rhs_f)) => {
                float_compare(lhs_f, rhs_f)
            }
            (&ConstantValue::Int(lhs_i), &ConstantValue::Int(rhs_i)) => int_compare(lhs_i, rhs_i),
            (&ConstantValue::Uint(lhs_u), &ConstantValue::Uint(rhs_u)) => {
                uint_compare(lhs_u, rhs_u)
            }
            _ => false,
        };

        if result { CONSTANT_ID_TRUE } else { CONSTANT_ID_FALSE }
    }
    pub fn less_than(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        _result_type_id: TypeId,
    ) -> ConstantId {
        compare_scalars(
            ir_meta,
            lhs_constant_id,
            rhs_constant_id,
            |f1, f2| f1 < f2,
            |i1, i2| i1 < i2,
            |u1, u2| u1 < u2,
        )
    }
    pub fn greater_than(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        _result_type_id: TypeId,
    ) -> ConstantId {
        compare_scalars(
            ir_meta,
            lhs_constant_id,
            rhs_constant_id,
            |f1, f2| f1 > f2,
            |i1, i2| i1 > i2,
            |u1, u2| u1 > u2,
        )
    }
    pub fn less_than_equal(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        _result_type_id: TypeId,
    ) -> ConstantId {
        compare_scalars(
            ir_meta,
            lhs_constant_id,
            rhs_constant_id,
            |f1, f2| f1 <= f2,
            |i1, i2| i1 <= i2,
            |u1, u2| u1 <= u2,
        )
    }
    pub fn greater_than_equal(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        _result_type_id: TypeId,
    ) -> ConstantId {
        compare_scalars(
            ir_meta,
            lhs_constant_id,
            rhs_constant_id,
            |f1, f2| f1 >= f2,
            |i1, i2| i1 >= i2,
            |u1, u2| u1 >= u2,
        )
    }
    pub fn bitwise_not(
        ir_meta: &mut IRMeta,
        constant_id: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        apply_unary_componentwise(
            ir_meta,
            constant_id,
            result_type_id,
            |_| panic!("Internal error: Cannot bitwise-not a float"),
            |i| !i,
            |u| !u,
            |_| panic!("Internal error: Cannot bitwise-not a bool"),
        )
    }
    pub fn bit_shift_left(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        // GLSL allows the operands of shift to differ in signedness.  Cast the rhs to the type of
        // lhs before calling the helper.  This can be conveniently done with `construct`, which
        // may turn a scalar rhs into a vector.  This is ok because the operation is component-wise
        // anyway.
        let lhs_type_id = ir_meta.get_constant(lhs_constant_id).type_id;
        let rhs_constant_id = construct(ir_meta, vec![rhs_constant_id], lhs_type_id);

        apply_binary_componentwise(
            ir_meta,
            lhs_constant_id,
            rhs_constant_id,
            result_type_id,
            |_, _| panic!("Internal error: Cannot use << on floats"),
            |i1, i2| i1.checked_shl(i2 as u32).unwrap_or(0),
            |u1, u2| u1.checked_shl(u2).unwrap_or(0),
            |_, _| panic!("Internal error: Cannot use << on bools"),
        )
    }
    pub fn bit_shift_right(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        // See bit_shift_left
        let lhs_type_id = ir_meta.get_constant(lhs_constant_id).type_id;
        let rhs_constant_id = construct(ir_meta, vec![rhs_constant_id], lhs_type_id);

        apply_binary_componentwise(
            ir_meta,
            lhs_constant_id,
            rhs_constant_id,
            result_type_id,
            |_, _| panic!("Internal error: Cannot use >> on floats"),
            |i1, i2| i1.checked_shr(i2 as u32).unwrap_or(0),
            |u1, u2| u1.checked_shr(u2).unwrap_or(0),
            |_, _| panic!("Internal error: Cannot use >> on bools"),
        )
    }
    pub fn bitwise_or(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        apply_binary_componentwise(
            ir_meta,
            lhs_constant_id,
            rhs_constant_id,
            result_type_id,
            |_, _| panic!("Internal error: Cannot use | on floats"),
            |i1, i2| i1 | i2,
            |u1, u2| u1 | u2,
            |_, _| panic!("Internal error: Cannot use | on bools"),
        )
    }
    pub fn bitwise_xor(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        apply_binary_componentwise(
            ir_meta,
            lhs_constant_id,
            rhs_constant_id,
            result_type_id,
            |_, _| panic!("Internal error: Cannot use ^ on floats"),
            |i1, i2| i1 ^ i2,
            |u1, u2| u1 ^ u2,
            |_, _| panic!("Internal error: Cannot use ^ on bools"),
        )
    }
    pub fn bitwise_and(
        ir_meta: &mut IRMeta,
        lhs_constant_id: ConstantId,
        rhs_constant_id: ConstantId,
        result_type_id: TypeId,
    ) -> ConstantId {
        apply_binary_componentwise(
            ir_meta,
            lhs_constant_id,
            rhs_constant_id,
            result_type_id,
            |_, _| panic!("Internal error: Cannot use & on floats"),
            |i1, i2| i1 & i2,
            |u1, u2| u1 & u2,
            |_, _| panic!("Internal error: Cannot use & on bools"),
        )
    }
}

// Helper functions that derive the result type of an operation.
mod promote {
    use crate::ir::*;

    // A helper to get the type id of an element being indexed.  The difference with
    // `Type::get_element_type_id` is that if the type is a `Pointer`, the result is also a
    // pointer.
    //
    // Additionally, to support swizzles, the element is optionally vectorized.
    fn get_indexed_type_id(
        ir_meta: &mut IRMeta,
        type_id: TypeId,
        vector_size: Option<u32>,
    ) -> TypeId {
        let type_info = ir_meta.get_type(type_id);
        let is_pointer = type_info.is_pointer();
        let mut element_type_id = type_info.get_element_type_id().unwrap();

        if is_pointer {
            element_type_id = ir_meta.get_type(element_type_id).get_element_type_id().unwrap();
        }

        vector_size.inspect(|&size| {
            element_type_id = ir_meta.get_vector_type_id_from_element_id(element_type_id, size)
        });

        if is_pointer {
            element_type_id = ir_meta.get_pointer_type_id(element_type_id);
        }

        element_type_id
    }
    // Used by some binary operations between scalars and vectors, returns the vector type.
    fn promote_scalar_if_necessary(ir_meta: &mut IRMeta, type1: TypeId, type2: TypeId) -> TypeId {
        if matches!(ir_meta.get_type(type1), Type::Scalar(_)) { type2 } else { type1 }
    }

    // The type of `vector.x` derived from the type of `vector`, used for AccessVectorComponent and
    // ExtractVectorComponent.
    pub fn vector_component(ir_meta: &mut IRMeta, operand_type: TypeId) -> TypeId {
        get_indexed_type_id(ir_meta, operand_type, None)
    }

    // The type of `vector.xy` derived from the type of `vector`, used for
    // AccessVectorComponentMulti and ExtractVectorComponentMulti.
    pub fn vector_component_multi(
        ir_meta: &mut IRMeta,
        operand_type: TypeId,
        vector_size: u32,
    ) -> TypeId {
        get_indexed_type_id(ir_meta, operand_type, Some(vector_size))
    }

    // The type of `a[i]` derived from the type of `a`, used for AccessVectorComponentDynamic,
    // AccessMatrixColumn, AccessArrayElement, ExtractVectorComponentDynamic, ExtractMatrixColumn,
    // and ExtractArrayElement.
    pub fn index(ir_meta: &mut IRMeta, operand_type: TypeId) -> TypeId {
        get_indexed_type_id(ir_meta, operand_type, None)
    }

    // The type of `strct.field` derived from the type of `strct` and the selected field, used for
    // AccessStructField and ExtractStructField,
    pub fn struct_field(ir_meta: &mut IRMeta, type_id: TypeId, field: u32) -> TypeId {
        let mut type_info = ir_meta.get_type(type_id);
        let is_pointer = type_info.is_pointer();
        if is_pointer {
            type_info = ir_meta.get_type(type_info.get_element_type_id().unwrap());
        }

        let mut field_type_id = type_info.get_struct_field(field).type_id;

        if is_pointer {
            field_type_id = ir_meta.get_pointer_type_id(field_type_id);
        }

        field_type_id
    }

    // The result type of Negate and BitwiseNot is the same as their operand.
    pub fn negate(_ir_meta: &mut IRMeta, operand_type: TypeId) -> TypeId {
        operand_type
    }
    pub fn bitwise_not(_ir_meta: &mut IRMeta, operand_type: TypeId) -> TypeId {
        operand_type
    }

    // Add, Sub, Mul, Div, IMod, BitwiseOr, BitwiseAnd, and BitwiseXor implicitly promote a scalar
    // operand to vector if they other operand is a vector.
    pub fn add(ir_meta: &mut IRMeta, lhs_type: TypeId, rhs_type: TypeId) -> TypeId {
        promote_scalar_if_necessary(ir_meta, lhs_type, rhs_type)
    }
    pub fn sub(ir_meta: &mut IRMeta, lhs_type: TypeId, rhs_type: TypeId) -> TypeId {
        promote_scalar_if_necessary(ir_meta, lhs_type, rhs_type)
    }
    pub fn mul(ir_meta: &mut IRMeta, lhs_type: TypeId, rhs_type: TypeId) -> TypeId {
        promote_scalar_if_necessary(ir_meta, lhs_type, rhs_type)
    }
    pub fn div(ir_meta: &mut IRMeta, lhs_type: TypeId, rhs_type: TypeId) -> TypeId {
        promote_scalar_if_necessary(ir_meta, lhs_type, rhs_type)
    }
    pub fn imod(ir_meta: &mut IRMeta, lhs_type: TypeId, rhs_type: TypeId) -> TypeId {
        promote_scalar_if_necessary(ir_meta, lhs_type, rhs_type)
    }
    pub fn bitwise_or(ir_meta: &mut IRMeta, lhs_type: TypeId, rhs_type: TypeId) -> TypeId {
        promote_scalar_if_necessary(ir_meta, lhs_type, rhs_type)
    }
    pub fn bitwise_and(ir_meta: &mut IRMeta, lhs_type: TypeId, rhs_type: TypeId) -> TypeId {
        promote_scalar_if_necessary(ir_meta, lhs_type, rhs_type)
    }
    pub fn bitwise_xor(ir_meta: &mut IRMeta, lhs_type: TypeId, rhs_type: TypeId) -> TypeId {
        promote_scalar_if_necessary(ir_meta, lhs_type, rhs_type)
    }

    // VectorTimesScalar and MatrixTimesScalar apply the scalar to each component, and the result
    // is identical to the vector/matrix itself.
    pub fn vector_times_scalar(
        _ir_meta: &mut IRMeta,
        lhs_type: TypeId,
        _rhs_type: TypeId,
    ) -> TypeId {
        lhs_type
    }
    pub fn matrix_times_scalar(
        _ir_meta: &mut IRMeta,
        lhs_type: TypeId,
        _rhs_type: TypeId,
    ) -> TypeId {
        lhs_type
    }

    // For VectorTimesMatrix, the result type is a vector of the matrix's row count.
    pub fn vector_times_matrix(
        ir_meta: &mut IRMeta,
        _lhs_type: TypeId,
        rhs_type: TypeId,
    ) -> TypeId {
        let size = ir_meta.get_type(rhs_type).get_matrix_size().unwrap();
        ir_meta.get_vector_type_id(BasicType::Float, size)
    }
    // For MatrixTimesVector, the result type is a vector of the matrix's column count.
    pub fn matrix_times_vector(
        ir_meta: &mut IRMeta,
        lhs_type: TypeId,
        _rhs_type: TypeId,
    ) -> TypeId {
        ir_meta.get_type(lhs_type).get_element_type_id().unwrap()
    }
    // For MatrixTimesMatrix, the result type is a matrix with rhs's column count of lhs's column
    // type.
    pub fn matrix_times_matrix(ir_meta: &mut IRMeta, lhs_type: TypeId, rhs_type: TypeId) -> TypeId {
        let result_column_type_id = ir_meta.get_type(lhs_type).get_element_type_id().unwrap();
        let result_row_count = ir_meta.get_type(result_column_type_id).get_vector_size().unwrap();
        let result_column_count = ir_meta.get_type(rhs_type).get_matrix_size().unwrap();
        ir_meta.get_matrix_type_id(result_column_count, result_row_count)
    }

    // BitShiftLeft and BitShiftRight produce a result with the same type as the value being
    // shifted.
    pub fn bit_shift_left(_ir_meta: &mut IRMeta, lhs_type: TypeId, _rhs_type: TypeId) -> TypeId {
        lhs_type
    }
    pub fn bit_shift_right(_ir_meta: &mut IRMeta, lhs_type: TypeId, _rhs_type: TypeId) -> TypeId {
        lhs_type
    }
}

// Helper functions that derive the precision of an operation.
pub mod precision {
    use crate::ir::*;

    // Taking some precision and comparing it with another:
    //
    // * Upgrade to highp is either no up or an upgrade, so it can unconditionally be done.
    // * Upgrade to mediump is only necessary if the original precision was lowp.
    // * Upgrade to lowp is never necessary.
    //
    // If either precision is NotApplicable, use the other one.  This can happen with constants as
    // they don't have a precision.
    pub fn higher_precision(one: Precision, other: Precision) -> Precision {
        match one {
            Precision::High => Precision::High,
            Precision::Medium if other == Precision::Low => Precision::Medium,
            _ => {
                // The other's precision is at least as high as this one, unless it doesn't have a
                // precision at all.
                if other == Precision::NotApplicable { one } else { other }
            }
        }
    }

    // Constructor precision is derived from its parameters, except for structs.
    pub fn construct(
        ir_meta: &IRMeta,
        type_id: TypeId,
        args: &mut impl Iterator<Item = Precision>,
    ) -> Precision {
        if matches!(ir_meta.get_type(type_id), Type::Struct(..)) {
            Precision::NotApplicable
        } else {
            args.fold(Precision::NotApplicable, |accumulator, precision| {
                higher_precision(accumulator, precision)
            })
        }
    }

    // Array length is usually a constant and therefore has no precision.  If it is called on an
    // unsized array at the end of an SSBO, it's highp.
    pub fn array_length() -> Precision {
        Precision::High
    }

    // The result of Negate has the same precision as the operand.
    pub fn negate(operand: Precision) -> Precision {
        operand
    }
    pub fn bitwise_not(operand: Precision) -> Precision {
        operand
    }

    // The result of most binary operations is the higher of the operands.
    pub fn add(lhs: Precision, rhs: Precision) -> Precision {
        higher_precision(lhs, rhs)
    }
    pub fn sub(lhs: Precision, rhs: Precision) -> Precision {
        higher_precision(lhs, rhs)
    }
    pub fn mul(lhs: Precision, rhs: Precision) -> Precision {
        higher_precision(lhs, rhs)
    }
    pub fn div(lhs: Precision, rhs: Precision) -> Precision {
        higher_precision(lhs, rhs)
    }
    pub fn imod(lhs: Precision, rhs: Precision) -> Precision {
        higher_precision(lhs, rhs)
    }
    pub fn vector_times_scalar(lhs: Precision, rhs: Precision) -> Precision {
        higher_precision(lhs, rhs)
    }
    pub fn matrix_times_scalar(lhs: Precision, rhs: Precision) -> Precision {
        higher_precision(lhs, rhs)
    }
    pub fn vector_times_matrix(lhs: Precision, rhs: Precision) -> Precision {
        higher_precision(lhs, rhs)
    }
    pub fn matrix_times_vector(lhs: Precision, rhs: Precision) -> Precision {
        higher_precision(lhs, rhs)
    }
    pub fn matrix_times_matrix(lhs: Precision, rhs: Precision) -> Precision {
        higher_precision(lhs, rhs)
    }
    pub fn bitwise_or(lhs: Precision, rhs: Precision) -> Precision {
        higher_precision(lhs, rhs)
    }
    pub fn bitwise_xor(lhs: Precision, rhs: Precision) -> Precision {
        higher_precision(lhs, rhs)
    }
    pub fn bitwise_and(lhs: Precision, rhs: Precision) -> Precision {
        higher_precision(lhs, rhs)
    }

    // Shift operations have the same precision in the result as the value being shifted.
    pub fn bit_shift_left(lhs: Precision, _rhs: Precision) -> Precision {
        lhs
    }
    pub fn bit_shift_right(lhs: Precision, _rhs: Precision) -> Precision {
        lhs
    }

    // Operations that produce bool have no precision.
    pub fn logical_not(_operand: Precision) -> Precision {
        Precision::NotApplicable
    }
    pub fn logical_xor(_lhs: Precision, _rhs: Precision) -> Precision {
        Precision::NotApplicable
    }
    pub fn equal(_lhs: Precision, _rhs: Precision) -> Precision {
        Precision::NotApplicable
    }
    pub fn not_equal(_lhs: Precision, _rhs: Precision) -> Precision {
        Precision::NotApplicable
    }
    pub fn less_than(_lhs: Precision, _rhs: Precision) -> Precision {
        Precision::NotApplicable
    }
    pub fn greater_than(_lhs: Precision, _rhs: Precision) -> Precision {
        Precision::NotApplicable
    }
    pub fn less_than_equal(_lhs: Precision, _rhs: Precision) -> Precision {
        Precision::NotApplicable
    }
    pub fn greater_than_equal(_lhs: Precision, _rhs: Precision) -> Precision {
        Precision::NotApplicable
    }
}

// The result of creating an instruction.  It's either a constant (if constant-folded), a void
// instruction or a register id referencing the instruction in IRMeta::instructions.  Sometimes
// an instruction can detect that it's a no-op too.
#[cfg_attr(debug_assertions, derive(Debug))]
pub enum Result {
    Constant(ConstantId, TypeId),
    Void(OpCode),
    Register(TypedRegisterId),
    NoOp(TypedId),
}

impl Result {
    pub fn get_result_id(&self) -> TypedId {
        match self {
            &instruction::Result::Constant(id, type_id) => TypedId::from_constant_id(id, type_id),
            &instruction::Result::Register(id) => TypedId::from_register_id(id),
            &instruction::Result::NoOp(id) => id,
            _ => panic!("Internal error: Expected instruction with value"),
        }
    }

    // Use to make sure the result of the instruction is not a new register, but rather an
    // existing one.  During transformations, an instruction producing register N may need to
    // be replaced with other instructions.  Making sure the end result has the same id
    // obviates the need to transform the IR further to replace the result id with a new one.
    pub fn override_result_id(&mut self, ir_meta: &mut IRMeta, id: TypedRegisterId) {
        match self {
            Result::Register(replace_by) => {
                ir_meta.replace_instruction(id.id, replace_by.id);
                *replace_by = id;
            }
            Result::Constant(constant_id, type_id) => {
                // Create an alias for this constant.
                let mut alias =
                    instruction::alias(ir_meta, TypedId::from_constant_id(*constant_id, *type_id));
                alias.override_result_id(ir_meta, id);
                *self = alias;
            }
            Result::NoOp(replace_by) => {
                // Create an alias for this id.
                let mut alias = instruction::alias(ir_meta, *replace_by);
                alias.override_result_id(ir_meta, id);
                *self = alias;
            }
            _ => panic!("Internal error: Expected typed instruction when replacing result id"),
        }
    }
}

// Helper macros to create an instruction, returning a Result.  For example:
//
//     instruction::make!(negate, ir_meta, operand)
//     instruction::make!(add, ir_meta, lhs, rhs)
macro_rules! make {
        ($func:ident, $ir_meta:expr, $($params:expr),*) => {
            instruction::$func($ir_meta, $($params),*)
        }
    }
pub(crate) use make;
// Similar to make!(), but with a specific result id.
//
//     instruction::make_with_result_id!(negate, ir_meta, result_id, operand)
macro_rules! make_with_result_id {
        ($func:ident, $ir_meta:expr, $result:expr, $($params:expr),*) => {
            {
                let mut inst = instruction::$func($ir_meta, $($params),*);
                inst.override_result_id($ir_meta, $result);
                inst
            }
        }
    }
pub(crate) use make_with_result_id;

fn make_register(
    ir_meta: &mut IRMeta,
    op: OpCode,
    type_id: TypeId,
    precision: Precision,
) -> Result {
    Result::Register(ir_meta.new_register(op, type_id, precision))
}

// A generic helper to make a unary operator.
fn unary_op<Promote, DerivePrecision, UnaryOp, ConstFold>(
    ir_meta: &mut IRMeta,
    operand: TypedId,
    promote: Promote,
    derive_precision: DerivePrecision,
    op: UnaryOp,
    const_fold: ConstFold,
) -> Result
where
    Promote: FnOnce(&mut IRMeta, TypeId) -> TypeId,
    DerivePrecision: FnOnce(Precision) -> Precision,
    UnaryOp: FnOnce(&mut IRMeta, TypedId, TypeId) -> OpCode,
    ConstFold: FnOnce(&mut IRMeta, ConstantId, TypeId) -> ConstantId,
{
    let result_type_id = promote(ir_meta, operand.type_id);

    // If the operand is constant, constant fold the operation
    if let Id::Constant(constant_id) = operand.id {
        let folded = const_fold(ir_meta, constant_id, result_type_id);
        Result::Constant(folded, result_type_id)
    } else {
        // Otherwise make an instruction
        let precision = derive_precision(operand.precision);
        let op = op(ir_meta, operand, result_type_id);
        make_register(ir_meta, op, result_type_id, precision)
    }
}

// A generic helper to make a binary operator.
fn binary_op<Promote, DerivePrecision, BinaryOp, ConstFold>(
    ir_meta: &mut IRMeta,
    lhs: TypedId,
    rhs: TypedId,
    promote: Promote,
    derive_precision: DerivePrecision,
    op: BinaryOp,
    const_fold: ConstFold,
) -> Result
where
    Promote: FnOnce(&mut IRMeta, TypeId, TypeId) -> TypeId,
    DerivePrecision: FnOnce(Precision, Precision) -> Precision,
    BinaryOp: FnOnce(&mut IRMeta, TypedId, TypedId, TypeId) -> OpCode,
    ConstFold: FnOnce(&mut IRMeta, ConstantId, ConstantId, TypeId) -> ConstantId,
{
    let result_type_id = promote(ir_meta, lhs.type_id, rhs.type_id);

    // If both operands are constant, constant fold the operation
    if let (Id::Constant(lhs_constant_id), Id::Constant(rhs_constant_id)) = (lhs.id, rhs.id) {
        let folded = const_fold(ir_meta, lhs_constant_id, rhs_constant_id, result_type_id);
        Result::Constant(folded, result_type_id)
    } else {
        // Otherwise make an instruction
        let precision = derive_precision(lhs.precision, rhs.precision);
        let op = op(ir_meta, lhs, rhs, result_type_id);
        make_register(ir_meta, op, result_type_id, precision)
    }
}

// Call a user-defined function.
pub fn call(ir_meta: &mut IRMeta, id: FunctionId, args: Vec<TypedId>) -> Result {
    let call = OpCode::Call(id, args);

    let function = ir_meta.get_function(id);
    let return_type_id = function.return_type_id;
    let return_precision = function.return_precision;

    // The result depends on whether the function returns void or a value.
    // If the function is not void, push a new id for its result.
    if return_type_id != TYPE_ID_VOID {
        make_register(ir_meta, call, return_type_id, return_precision)
    } else {
        Result::Void(call)
    }
}

// Block terminating instructions.
pub fn branch_discard() -> Result {
    Result::Void(OpCode::Discard)
}
pub fn branch_return(value: Option<TypedId>) -> Result {
    Result::Void(OpCode::Return(value))
}
pub fn branch_break() -> Result {
    Result::Void(OpCode::Break)
}
pub fn branch_continue() -> Result {
    Result::Void(OpCode::Continue)
}
pub fn branch_passthrough() -> Result {
    Result::Void(OpCode::Passthrough)
}
pub fn branch_next_block() -> Result {
    Result::Void(OpCode::NextBlock)
}
pub fn branch_merge(id: Option<TypedId>) -> Result {
    Result::Void(OpCode::Merge(id))
}

fn load_from_pointer(ir_meta: &mut IRMeta, to_load: TypedId, pointee_type_id: TypeId) -> Result {
    make_register(ir_meta, OpCode::Load(to_load), pointee_type_id, to_load.precision)
}

// Load a value from a pointer and return it.
pub fn load(ir_meta: &mut IRMeta, to_load: TypedId) -> Result {
    let type_info = ir_meta.get_type(to_load.type_id);

    match type_info {
        &Type::Pointer(pointee_type_id) => load_from_pointer(ir_meta, to_load, pointee_type_id),
        _ => Result::NoOp(to_load),
    }
}

// Store a value into a pointer.
pub fn store(_ir_meta: &mut IRMeta, pointer: TypedId, value: TypedId) -> Result {
    Result::Void(OpCode::Store(pointer, value))
}

pub fn alias(ir_meta: &mut IRMeta, id: TypedId) -> Result {
    make_register(ir_meta, OpCode::Alias(id), id.type_id, id.precision)
}

fn same_precision_as_operand(precision: Precision) -> Precision {
    precision
}

fn same_precision_as_lhs(lhs: Precision, _rhs: Precision) -> Precision {
    lhs
}

// Take a component of a vector, like `vector.y`.
pub fn vector_component(ir_meta: &mut IRMeta, vector: TypedId, component: u32) -> Result {
    // To avoid constant index on swizzles, like var.xyz[1], check if the value being indexed
    // is a swizzle, in which case the swizzle components can be folded and the swizzle
    // applied to the original vector.
    //
    // Note that this logic is not valid if the original swizzle was on a pointer and the
    // latter is on a loaded value, because the pointer's pointee value may have changed in
    // between.
    let mut vector = vector;
    let mut component = component;

    if let Id::Register(register_id) = vector.id {
        let instruction = ir_meta.get_instruction(register_id);
        match &instruction.op {
            &OpCode::ExtractVectorComponentMulti(original_vector, ref original_components)
            | &OpCode::AccessVectorComponentMulti(original_vector, ref original_components) => {
                vector = original_vector;
                component = original_components[component as usize];
            }
            _ => (),
        }
    }

    unary_op(
        ir_meta,
        vector,
        promote::vector_component,
        same_precision_as_operand,
        |ir_meta, operand, result_type_id| {
            let is_pointer = ir_meta.get_type(result_type_id).is_pointer();
            if is_pointer {
                OpCode::AccessVectorComponent(operand, component)
            } else {
                OpCode::ExtractVectorComponent(operand, component)
            }
        },
        |ir_meta, constant_id, result_type_id| {
            const_fold::composite_element(ir_meta, constant_id, component, result_type_id)
        },
    )
}

fn merge_swizzle_components(components: &Vec<u32>, to_apply: &Vec<u32>) -> Vec<u32> {
    to_apply.iter().map(|&index| components[index as usize]).collect()
}

// Take multiple components of a vector, like `vector.yxy`.
pub fn vector_component_multi(
    ir_meta: &mut IRMeta,
    vector: TypedId,
    components: Vec<u32>,
) -> Result {
    // If the swizzle selects every element in order, optimize that out.
    {
        let mut type_info = ir_meta.get_type(vector.type_id);
        if type_info.is_pointer() {
            type_info = ir_meta.get_type(type_info.get_element_type_id().unwrap());
        }
        let vec_size = type_info.get_vector_size().unwrap() as usize;
        let identity = [0, 1, 2, 3];
        if components[..] == identity[0..vec_size] {
            return Result::NoOp(vector);
        }
    }

    // To avoid swizzles of swizzles, like var.xyz.xz, check if the value being swizzled is
    // itself a swizzle, in which case the swizzle components can be folded and the swizzle
    // applied to the original vector.
    //
    // Note that this logic is not valid if the original swizzle was on a pointer and the
    // latter is on a loaded value, because the pointer's pointee value may have changed in
    // between.
    let mut vector = vector;
    let mut components = components;

    if let Id::Register(register_id) = vector.id {
        let instruction = ir_meta.get_instruction(register_id);
        match &instruction.op {
            &OpCode::ExtractVectorComponentMulti(original_vector, ref original_components)
            | &OpCode::AccessVectorComponentMulti(original_vector, ref original_components) => {
                vector = original_vector;
                components = merge_swizzle_components(original_components, &components);
            }
            _ => (),
        }
    }

    let components_clone = components.clone();
    unary_op(
        ir_meta,
        vector,
        |ir_meta, operand_type| {
            promote::vector_component_multi(ir_meta, operand_type, components.len() as u32)
        },
        same_precision_as_operand,
        |ir_meta, operand, result_type_id| {
            let is_pointer = ir_meta.get_type(result_type_id).is_pointer();
            if is_pointer {
                OpCode::AccessVectorComponentMulti(operand, components_clone)
            } else {
                OpCode::ExtractVectorComponentMulti(operand, components_clone)
            }
        },
        |ir_meta, constant_id, result_type_id| {
            const_fold::vector_components(ir_meta, constant_id, &components, result_type_id)
        },
    )
}

// Equivalent of operator [].
pub fn index(ir_meta: &mut IRMeta, indexed: TypedId, index: TypedId) -> Result {
    // Note: constant index on a vector should use vector_component() instead.
    debug_assert!(
        !(matches!(ir_meta.get_type(indexed.type_id), Type::Vector(..))
            && matches!(index.id, Id::Constant(_)))
    );

    binary_op(
        ir_meta,
        indexed,
        index,
        |ir_meta, indexed_type, _| promote::index(ir_meta, indexed_type),
        same_precision_as_lhs,
        |ir_meta, indexed, index, _| {
            let mut referenced_indexed_type = ir_meta.get_type(indexed.type_id);
            let is_pointer = referenced_indexed_type.is_pointer();

            if is_pointer {
                referenced_indexed_type =
                    ir_meta.get_type(referenced_indexed_type.get_element_type_id().unwrap());
            }

            match referenced_indexed_type {
                Type::Vector(..) => {
                    if is_pointer {
                        OpCode::AccessVectorComponentDynamic(indexed, index)
                    } else {
                        OpCode::ExtractVectorComponentDynamic(indexed, index)
                    }
                }
                Type::Matrix(..) => {
                    if is_pointer {
                        OpCode::AccessMatrixColumn(indexed, index)
                    } else {
                        OpCode::ExtractMatrixColumn(indexed, index)
                    }
                }
                _ => {
                    if is_pointer {
                        OpCode::AccessArrayElement(indexed, index)
                    } else {
                        OpCode::ExtractArrayElement(indexed, index)
                    }
                }
            }
        },
        const_fold::index,
    )
}

// Select a field of a struct, like `block.field`.
pub fn struct_field(ir_meta: &mut IRMeta, struct_id: TypedId, field_index: u32) -> Result {
    // Use the precision of the field itself on the result
    let mut struct_type_info = ir_meta.get_type(struct_id.type_id);
    if struct_type_info.is_pointer() {
        struct_type_info = ir_meta.get_type(struct_type_info.get_element_type_id().unwrap());
    }
    let field_precision = struct_type_info.get_struct_field(field_index).precision;

    unary_op(
        ir_meta,
        struct_id,
        |ir_meta, operand_type| promote::struct_field(ir_meta, operand_type, field_index),
        |_| field_precision,
        |ir_meta, operand, result_type_id| {
            let is_pointer = ir_meta.get_type(result_type_id).is_pointer();

            if is_pointer {
                OpCode::AccessStructField(operand, field_index)
            } else {
                OpCode::ExtractStructField(operand, field_index)
            }
        },
        |ir_meta, constant_id, result_type_id| {
            const_fold::composite_element(ir_meta, constant_id, field_index, result_type_id)
        },
    )
}

fn verify_construct_arg_component_count(
    ir_meta: &mut IRMeta,
    type_id: TypeId,
    args: &Vec<TypedId>,
) -> bool {
    let type_info = ir_meta.get_type(type_id);
    match type_info {
        // Structs and arrays always require an exact number of elements.
        Type::Array(..) | Type::Struct(..) => {
            return true;
        }
        // Matrix-from-matrix constructors are allowed to have mismatching component count.
        Type::Matrix(..) if args.len() == 1 => {
            return true;
        }
        _ => (),
    }

    let expected_total_components = type_info.get_total_component_count(ir_meta);

    let args_total_components = args.iter().fold(0, |total, id| {
        let type_info = ir_meta.get_type(id.type_id);
        total + type_info.get_total_component_count(ir_meta)
    });

    // Vec and matrix constructors either take a scalar, or the caller should have ensured the
    // number of arguments in args matches the constructed type.
    args_total_components == 1 || args_total_components == expected_total_components
}

// Construct a value of a type from the given arguments.
pub fn construct(ir_meta: &mut IRMeta, type_id: TypeId, args: Vec<TypedId>) -> Result {
    // Note: For vector and matrix constructors with multiple components, it is expected that
    // the total components in `args` matches the components needed for type_id.
    debug_assert!(verify_construct_arg_component_count(ir_meta, type_id, &args));

    // Constructor precision is derived from its parameters, except for structs.
    let promoted_precision =
        precision::construct(ir_meta, type_id, &mut args.iter().map(|id| id.precision));

    // If the type of the first argument is the same as the result, the rest of the arguments
    // will be stripped (if any) and the cast is a no-op.  In that case, push args[0] back to
    // the stack and early out.
    //
    // Note: Per the GLSL spec, the constructor precision is derived from its arguments,
    // meaning that `vecN(vN, vM)` where vN is mediump and vM is highp should have highp
    // results, so replacing that with `vN` with the same precision is not entirely correct.
    // For maximum correctness, a new `Copy` opcode is needed to change the precision and give
    // this a new id, but this is such a niche case that we're ignoring it for simplicity.
    if args[0].type_id == type_id {
        return Result::NoOp(args[0]);
    }

    let all_constants = args.iter().all(|id| matches!(id.id, Id::Constant(_)));
    if all_constants {
        let folded = const_fold::construct(
            ir_meta,
            args.iter().map(|id| id.id.get_constant().unwrap()).collect(),
            type_id,
        );
        Result::Constant(folded, type_id)
    } else {
        let type_info = ir_meta.get_type(type_id);
        let arg0_type_info = ir_meta.get_type(args[0].type_id);

        // Decide the opcode
        let op = match type_info {
            Type::Scalar(..) => OpCode::ConstructScalarFromScalar(args[0]),
            Type::Vector(..) => {
                if args.len() == 1 && matches!(arg0_type_info, Type::Scalar(..)) {
                    OpCode::ConstructVectorFromScalar(args[0])
                } else {
                    OpCode::ConstructVectorFromMultiple(args)
                }
            }
            Type::Matrix(..) => {
                if args.len() == 1 && matches!(arg0_type_info, Type::Scalar(..)) {
                    OpCode::ConstructMatrixFromScalar(args[0])
                } else if args.len() == 1 && matches!(arg0_type_info, Type::Matrix(..)) {
                    OpCode::ConstructMatrixFromMatrix(args[0])
                } else {
                    OpCode::ConstructMatrixFromMultiple(args)
                }
            }
            Type::Struct(..) => OpCode::ConstructStruct(args),
            Type::Array(..) => OpCode::ConstructArray(args),
            _ => panic!("Internal error: Type cannot be constructed"),
        };

        make_register(ir_meta, op, type_id, promoted_precision)
    }
}

// Get the array length of an unsized array.
pub fn array_length(ir_meta: &mut IRMeta, operand: TypedId) -> Result {
    // This should only be called for unsized arrays, otherwise the size is already known to
    // the caller.
    let type_info = ir_meta.get_type(operand.type_id);
    debug_assert!(type_info.is_pointer());
    let pointee_type_id = type_info.get_element_type_id().unwrap();

    let type_info = ir_meta.get_type(pointee_type_id);
    debug_assert!(type_info.is_unsized_array());

    make_register(
        ir_meta,
        OpCode::Unary(UnaryOpCode::ArrayLength, operand),
        TYPE_ID_INT,
        precision::array_length(),
    )
}

// Evaluate -operand
//
// Result type: Same as operand's
// Result precision: Same as operand's
pub fn negate(ir_meta: &mut IRMeta, operand: TypedId) -> Result {
    unary_op(
        ir_meta,
        operand,
        promote::negate,
        precision::negate,
        |_, operand, _| OpCode::Unary(UnaryOpCode::Negate, operand),
        const_fold::negate,
    )
}

// Evaluate operand1+operand2
//
// Result type: Same as operand1, unless it's a scalar and operand2 is a vector/matrix,
//              in which case same as operand2.
// Result precision: Higher of the two operands.
pub fn add(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        promote::add,
        precision::add,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::Add, lhs, rhs),
        const_fold::add,
    )
}

// Evaluate operand1-operand2
//
// Result type: Same as operand1, unless it's a scalar and operand2 is a vector/matrix,
//              in which case same as operand2.
// Result precision: Higher of the two operands.
pub fn sub(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        promote::sub,
        precision::sub,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::Sub, lhs, rhs),
        const_fold::sub,
    )
}

// Evaluate operand1*operand2
//
// Result type: Same as operand1, unless it's a scalar and operand2 is a vector/matrix,
//              in which case same as operand2.
// Result precision: Higher of the two operands.
pub fn mul(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        promote::mul,
        precision::mul,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::Mul, lhs, rhs),
        const_fold::mul,
    )
}

// Evaluate operand1*operand2
//
// Result type: Same as operand1.
// Result precision: Higher of the two operands.
pub fn vector_times_scalar(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        promote::vector_times_scalar,
        precision::vector_times_scalar,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::VectorTimesScalar, lhs, rhs),
        const_fold::mul,
    )
}

// Evaluate operand1*operand2
//
// Result type: Same as operand1.
// Result precision: Higher of the two operands.
pub fn matrix_times_scalar(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        promote::matrix_times_scalar,
        precision::matrix_times_scalar,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::MatrixTimesScalar, lhs, rhs),
        const_fold::mul,
    )
}

// Evaluate operand1*operand2
//
// Result type: Vector of operand2's row count.
// Result precision: Higher of the two operands.
pub fn vector_times_matrix(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        promote::vector_times_matrix,
        precision::vector_times_matrix,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::VectorTimesMatrix, lhs, rhs),
        const_fold::vector_times_matrix,
    )
}

// Evaluate operand1*operand2
//
// Result type: operand1's column type.
// Result precision: Higher of the two operands.
pub fn matrix_times_vector(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        promote::matrix_times_vector,
        precision::matrix_times_vector,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::MatrixTimesVector, lhs, rhs),
        const_fold::matrix_times_vector,
    )
}

// Evaluate operand1*operand2
//
// Result type: matrix with operand2's column count of operand1's column type.
// Result precision: Higher of the two operands.
pub fn matrix_times_matrix(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        promote::matrix_times_matrix,
        precision::matrix_times_matrix,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::MatrixTimesMatrix, lhs, rhs),
        const_fold::matrix_times_matrix,
    )
}

// Evaluate operand1/operand2
//
// Result type: Same as operand1, unless it's a scalar and operand2 is a vector/matrix,
//              in which case same as operand2.
// Result precision: Higher of the two operands.
pub fn div(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        promote::div,
        precision::div,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::Div, lhs, rhs),
        const_fold::div,
    )
}

// Evaluate operand1%operand2
//
// Result type: Same as operand1, unless it's a scalar and operand2 is a vector,
//              in which case same as operand2.
// Result precision: Higher of the two operands.
pub fn imod(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        promote::imod,
        precision::imod,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::IMod, lhs, rhs),
        const_fold::add,
    )
}

// Evaluate !operand
//
// Result type: Same as operand (bool)
// Result precision: Not applicable
pub fn logical_not(ir_meta: &mut IRMeta, operand: TypedId) -> Result {
    // If ! is applied to an instruction that already has a !, use the original instead.
    if let Id::Register(register_id) = operand.id {
        let operand_instruction = ir_meta.get_instruction(register_id);
        if let OpCode::Unary(UnaryOpCode::LogicalNot, original) = operand_instruction.op {
            return Result::NoOp(original);
        }
    }

    unary_op(
        ir_meta,
        operand,
        |_, _| TYPE_ID_BOOL,
        precision::logical_not,
        |_, operand, _| OpCode::Unary(UnaryOpCode::LogicalNot, operand),
        const_fold::logical_not,
    )
}

// Evaluate operand1^^operand2
//
// Result type: Same as operand1 and operand2 (bool)
// Result precision: Not applicable
pub fn logical_xor(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        |_, _, _| TYPE_ID_BOOL,
        precision::logical_xor,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::LogicalXor, lhs, rhs),
        const_fold::logical_xor,
    )
}

// Evaluate operand1==operand2
//
// Result type: bool
// Result precision: Not applicable
pub fn equal(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        |_, _, _| TYPE_ID_BOOL,
        precision::equal,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::Equal, lhs, rhs),
        const_fold::equal,
    )
}

// Evaluate operand1!=operand2
//
// Result type: bool
// Result precision: Not applicable
pub fn not_equal(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        |_, _, _| TYPE_ID_BOOL,
        precision::not_equal,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::NotEqual, lhs, rhs),
        const_fold::not_equal,
    )
}

// Evaluate operand1<operand2
//
// Result type: bool
// Result precision: Not applicable
pub fn less_than(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        |_, _, _| TYPE_ID_BOOL,
        precision::less_than,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::LessThan, lhs, rhs),
        const_fold::less_than,
    )
}

// Evaluate operand1>operand2
//
// Result type: bool
// Result precision: Not applicable
pub fn greater_than(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        |_, _, _| TYPE_ID_BOOL,
        precision::greater_than,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::GreaterThan, lhs, rhs),
        const_fold::greater_than,
    )
}

// Evaluate operand1<=operand2
//
// Result type: bool
// Result precision: Not applicable
pub fn less_than_equal(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        |_, _, _| TYPE_ID_BOOL,
        precision::less_than_equal,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::LessThanEqual, lhs, rhs),
        const_fold::less_than_equal,
    )
}

// Evaluate operand1>=operand2
//
// Result type: bool
// Result precision: Not applicable
pub fn greater_than_equal(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        |_, _, _| TYPE_ID_BOOL,
        precision::greater_than_equal,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::GreaterThanEqual, lhs, rhs),
        const_fold::greater_than_equal,
    )
}

// Evaluate ~operand
//
// Result type: Same as operand
// Result precision: Same as operand
pub fn bitwise_not(ir_meta: &mut IRMeta, operand: TypedId) -> Result {
    unary_op(
        ir_meta,
        operand,
        promote::bitwise_not,
        precision::bitwise_not,
        |_, operand, _| OpCode::Unary(UnaryOpCode::BitwiseNot, operand),
        const_fold::bitwise_not,
    )
}

// Evaluate operand1<<operand2
//
// Result type: Same as operand1
// Result precision: Same as operand1
pub fn bit_shift_left(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        promote::bit_shift_left,
        precision::bit_shift_left,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::BitShiftLeft, lhs, rhs),
        const_fold::bit_shift_left,
    )
}

// Evaluate operand1>>operand2
//
// Result type: Same as operand1
// Result precision: Same as operand1
pub fn bit_shift_right(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        promote::bit_shift_right,
        precision::bit_shift_right,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::BitShiftRight, lhs, rhs),
        const_fold::bit_shift_right,
    )
}

// Evaluate operand1|operand2
//
// Result type: Same as operand1, unless it's a scalar and operand2 is a vector,
//              in which case same as operand2.
// Result precision: Higher of the two operands.
pub fn bitwise_or(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        promote::bitwise_or,
        precision::bitwise_or,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::BitwiseOr, lhs, rhs),
        const_fold::bitwise_or,
    )
}

// Evaluate operand1^operand2
//
// Result type: Same as operand1, unless it's a scalar and operand2 is a vector,
//              in which case same as operand2.
// Result precision: Higher of the two operands.
pub fn bitwise_xor(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        promote::bitwise_xor,
        precision::bitwise_xor,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::BitwiseXor, lhs, rhs),
        const_fold::bitwise_xor,
    )
}

// Evaluate operand1&operand2
//
// Result type: Same as operand1, unless it's a scalar and operand2 is a vector,
//              in which case same as operand2.
// Result precision: Higher of the two operands.
pub fn bitwise_and(ir_meta: &mut IRMeta, lhs: TypedId, rhs: TypedId) -> Result {
    binary_op(
        ir_meta,
        lhs,
        rhs,
        promote::bitwise_and,
        precision::bitwise_and,
        |_, lhs, rhs, _| OpCode::Binary(BinaryOpCode::BitwiseAnd, lhs, rhs),
        const_fold::bitwise_and,
    )
}
