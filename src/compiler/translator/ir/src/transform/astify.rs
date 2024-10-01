// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Prepare the IR for turning into AST (be it ANGLE's legacy AST, or actual GLSL/etc text):
//
// - Remove merge block inputs and replace them with temp variables.
// - Use temporary variables for ops that have side effect but whose results are used multiple
//   times.
// - Replicate the continue block before every `continue`, if any.
// - Replicate the do-loop condition block before every `continue`, if any, but with it ending with
//   `if (!condition) break;`.  To support `continue` inside `switch`, a bool variable is created
//   for those that include a `continue` statement, which is set to true before the `break` that's
//   generated during this replication.  After the switch, an `if (propagate_break) break;` is
//   added.
// - TODO: if there's a pointer swizzle followed by load followed by value swizzle / constant index,
//   need to store the loaded value in a temp before swizzling it again.
use crate::ir::*;
use crate::*;

struct RegisterInfo {
    // How many times the register is read from.
    read_count: u32,
    // Whether the calculation that led to this ID has side effect.  If the ID is read from
    // multiple times, it cannot be replicated in AST if it has side effects.
    has_side_effect: bool,
    // Whether the calculation that led to this ID is complex.  If the ID is read from
    // multiple times, it is inefficient in AST to replicate it.
    is_complex: bool,
}

impl RegisterInfo {
    fn new() -> RegisterInfo {
        RegisterInfo { read_count: 0, has_side_effect: false, is_complex: false }
    }
}

struct BreakInfo {
    // Whether this is a switch block
    is_switch: bool,
    // What is the variable to propagate `break`s.
    propagate_break_var: Option<VariableId>,
}

impl BreakInfo {
    fn new_loop() -> BreakInfo {
        BreakInfo { is_switch: false, propagate_break_var: None }
    }

    fn new_switch() -> BreakInfo {
        BreakInfo { is_switch: true, propagate_break_var: None }
    }
}

struct State<'a> {
    ir_meta: &'a mut IRMeta,
    // Used to know when temporary variables are needed
    register_info: HashMap<RegisterId, RegisterInfo>,
    // Used to replicate the continue block of for loops before each `continue`
    // instruction.  `continue_stack` includes the continue block of constructs that
    // interact with a `continue` instruction (Loop and DoLoop).
    continue_stack: Vec<Option<Block>>,
    // Used to replicate the condition block of do-loops before each `continue`
    // instruction.
    //
    // `break_stack` tracks constructs that interact with a `break` instruction (Loop,
    // DoLoop and Switch).  It contains whether the construct is a Switch block, and if so
    // whether it needs a variable for an indirect break and what that variable is.
    //
    // `condition_stack` includes the condition block of DoLoop, or None for Loops.  The
    // condition block is changed to include `if (!condition) break` followed by
    // `NextBlock`, instead of `LoopIf`.
    break_stack: Vec<BreakInfo>,
    condition_stack: Vec<Option<Block>>,
}

pub fn run(ir: &mut IR) {
    let mut state = State {
        ir_meta: &mut ir.meta,
        register_info: HashMap::new(),
        continue_stack: Vec::new(),
        break_stack: Vec::new(),
        condition_stack: Vec::new(),
    };

    // Pre-process the registers to determine when temporary variables are needed.
    preprocess_registers(&mut state, &ir.function_entries);

    // First, create temporary variables for the result of instructions that have side
    // effects and yet are read from multiple times.
    let mut transformer = traverser::Transformer::new();
    transformer.for_each_instruction(
        &mut state,
        &mut ir.function_entries,
        &|state, instruction| transform_instruction(state, instruction),
    );

    // Then create variables for merge block inputs as that is not representable in the
    // AST.  No need to try and be smart with them, such as producing ?:, && or ||.
    transformer.for_each_function(
        &mut state,
        &mut ir.function_entries,
        &|transformer, state, entry| {
            transformer.for_each_block(
                state,
                entry,
                &|_, state, block| replace_merge_input_with_variable(state, block),
                &|_, _, block| block,
            );
        },
    );

    // Finally, duplicate the continue block, if any, before each `continue` branch, because
    // it may be too complicated to be placed inside a `for ()` expression.  Afterwards,
    // the IR no longer has continue blocks.
    //
    // At the same time, duplicate the condition block of do-loops before each `continue`
    // branch.
    transformer.for_each_function(
        &mut state,
        &mut ir.function_entries,
        &|transformer, state, entry| {
            transformer.for_each_block(
                state,
                entry,
                &|_, state, block| transform_continue_instructions_pre_visit(state, block),
                &|_, state, block| transform_continue_instructions_post_visit(state, block),
            );
        },
    );
}

fn preprocess_block_registers(state: &mut State, block: &Block) {
    for instruction in &block.instructions {
        let (opcode, result) = instruction.get_op_and_result(state.ir_meta);

        // Mark every potentially-register Id in the arguments of the opcode as being
        // accessed.
        let mut read_ids = Vec::with_capacity(4);
        match opcode {
            OpCode::MergeInput
            | OpCode::Discard
            | OpCode::Break
            | OpCode::Continue
            | OpCode::Passthrough
            | OpCode::NextBlock
            | OpCode::Loop
            | OpCode::DoLoop
            | OpCode::Return(None)
            | OpCode::Merge(None) => (),
            OpCode::Call(_, params)
            | OpCode::ConstructVectorFromMultiple(params)
            | OpCode::ConstructMatrixFromMultiple(params)
            | OpCode::ConstructStruct(params)
            | OpCode::ConstructArray(params) => read_ids.extend(params),
            &OpCode::Return(Some(id))
            | &OpCode::Merge(Some(id))
            | &OpCode::If(id)
            | &OpCode::LoopIf(id)
            | &OpCode::Switch(id, _)
            | &OpCode::ExtractVectorComponent(id, _)
            | &OpCode::ExtractVectorComponentMulti(id, _)
            | &OpCode::ExtractStructField(id, _)
            | &OpCode::ConstructScalarFromScalar(id)
            | &OpCode::ConstructVectorFromScalar(id)
            | &OpCode::ConstructMatrixFromScalar(id)
            | &OpCode::ConstructMatrixFromMatrix(id)
            | &OpCode::AccessVectorComponent(id, _)
            | &OpCode::AccessVectorComponentMulti(id, _)
            | &OpCode::AccessStructField(id, _)
            | &OpCode::Load(id)
            | &OpCode::Unary(_, id) => read_ids.push(id),
            &OpCode::ExtractVectorComponentDynamic(lhs, rhs)
            | &OpCode::ExtractMatrixColumn(lhs, rhs)
            | &OpCode::ExtractArrayElement(lhs, rhs)
            | &OpCode::AccessVectorComponentDynamic(lhs, rhs)
            | &OpCode::AccessMatrixColumn(lhs, rhs)
            | &OpCode::AccessArrayElement(lhs, rhs)
            | &OpCode::Store(lhs, rhs)
            | &OpCode::Binary(_, lhs, rhs) => {
                read_ids.push(lhs);
                read_ids.push(rhs);
            }
            &OpCode::Alias(_) => {
                panic!("Internal error: Aliases must have been resolved before text generation")
            }
        };

        for id in read_ids {
            if let Id::Register(id) = id.id {
                state.register_info.entry(id).or_insert(RegisterInfo::new()).read_count += 1;
            }
        }

        // If the instruction has a side-effect, mark its resulting ID as having side
        // effect.  Similarly, if it is complex, mark it as such.
        if let Some(result_id) = result {
            match opcode {
                OpCode::Call(..) => {
                    // TODO: for now, assume every function call has a side effect.  This
                    //       can be optimized with a prepass going over functions and
                    //       checking if they have side effect.  AST assumes user functions
                    //       have side effects, and mostly uses isKnownNotToHaveSideEffects
                    //       for built-ins, they would be separately checked here.  Some
                    //       internal transformations mark a function as no-side effect,
                    //       but no real benefit comes from that IMO.  This is probably
                    //       fine as-is.
                    // TODO: add built-ins with side effect here.
                    let info =
                        state.register_info.entry(result_id.id).or_insert(RegisterInfo::new());
                    info.has_side_effect = true;
                    info.is_complex = true;
                }
                OpCode::AccessVectorComponent(..)
                | OpCode::AccessVectorComponentMulti(..)
                | OpCode::AccessVectorComponentDynamic(..)
                | OpCode::AccessMatrixColumn(..)
                | OpCode::AccessStructField(..)
                | OpCode::AccessArrayElement(..)
                | OpCode::Load(..) => {
                    // For instructions that deal with pointers, consider them not complex
                    // so they can be replicated.  For Access* instructions, it's
                    // impossible to cache the result in the AST, and for load, it doesn't
                    // make sense to store its result in a variable only to have to load
                    // from it on every use again.
                    //
                    state.register_info.insert(result_id.id, RegisterInfo::new());
                }
                _ => {
                    // For everything else, consider the result complex so the logic is not
                    // duplicated.  This can cover everything from a+b to texture calls
                    // etc.
                    state
                        .register_info
                        .entry(result_id.id)
                        .or_insert(RegisterInfo::new())
                        .is_complex = true;
                }
            }
        }
    }
}

fn preprocess_registers(state: &mut State, function_entries: &Vec<Option<Block>>) {
    let traverser = traverser::Visitor::new();
    traverser.for_each_function(
        state,
        function_entries,
        |_, _| {},
        |state, block, _, _| {
            preprocess_block_registers(state, block);
            traverser::Visitor::VISIT_SUB_BLOCKS
        },
        |_, _| {},
    );
}

fn transform_instruction(
    state: &mut State,
    instruction: &BlockInstruction,
) -> Vec<traverser::Transform> {
    // If the instruction is:
    //
    // - a register
    // - with side effect or its complex
    // - read multiple times
    //
    // Then a temporary variable must be created to hold the result.  For the sake of
    // simplicity, any register with a side effect is placed in a temporary, because
    // otherwise it's hard to tell when generating the AST if that statement should be
    // placed directly in the block, or whether it's used in another expression that will
    // eventually turn into a statement in the block.
    if let &BlockInstruction::Register(id) = instruction {
        let info = &state.register_info[&id];
        let cache_in_variable_if_necessary = info.has_side_effect || info.is_complex;
        let read_multiple_times = info.read_count > 1;

        if cache_in_variable_if_necessary && read_multiple_times || info.has_side_effect {
            let instruction = state.ir_meta.get_instruction(id);
            let id = instruction.result;

            // Assume the instruction is:
            //
            //     %id = ...
            //
            // This is replaced with:
            //
            //     %new_id = ...
            //               Store %new_variable %new_id
            //     %id     = Load %new_variable
            let variable_id = state.ir_meta.declare_variable(
                Name::new_temp(""),
                id.type_id,
                id.precision,
                Decorations::new_none(),
                None,
                None,
            );
            let new_register_id = state.ir_meta.assign_new_register_to_instruction(id.id);

            //     %new_id = ...
            let mut transforms = vec![
                traverser::Transform::DeclareVariable(variable_id),
                traverser::Transform::Add(BlockInstruction::new_typed(new_register_id)),
            ];

            let variable_id = TypedId::from_variable_id(state.ir_meta, variable_id);
            let new_register_id =
                TypedId::new(Id::new_register(new_register_id), id.type_id, id.precision);

            //               Store %new_variable %new_id
            traverser::add_void_instruction(
                &mut transforms,
                instruction::make!(store, state.ir_meta, variable_id, new_register_id),
            );
            //     %id     = Load %new_variable
            traverser::add_typed_instruction(
                &mut transforms,
                instruction::make_with_result_id!(load, state.ir_meta, id, variable_id),
            );

            transforms
        } else {
            vec![]
        }
    } else {
        vec![]
    }
}

fn replace_merge_input_with_variable<'block>(
    state: &mut State,
    block: &'block mut Block,
) -> &'block mut Block {
    // Look at the merge block, if there is an input, it is removed and a variable is added
    // to the current block instead.
    if let Some(merge_block) = &mut block.merge_block {
        if let Some(input) = merge_block.input {
            let variable_id = state.ir_meta.declare_variable(
                Name::new_temp(""),
                input.type_id,
                input.precision,
                Decorations::new_none(),
                None,
                None,
            );

            // Add variable to the list of variables to be declared in this block.
            block.variables.push(variable_id);

            // Adjust the merge block as well as blocks that can `Merge`.
            let variable_id = TypedId::from_variable_id(state.ir_meta, variable_id);
            replace_merge_input_with_variable_in_sub_blocks(
                state,
                merge_block,
                &mut block.block1,
                &mut block.block2,
                input,
                variable_id,
            );
        }
    }

    block
}

fn replace_merge_input_with_variable_in_sub_blocks(
    state: &mut State,
    merge_block: &mut Box<Block>,
    block1: &mut Option<Box<Block>>,
    block2: &mut Option<Box<Block>>,
    input_id: TypedRegisterId,
    variable_id: TypedId,
) {
    // Remove the merge block input and replace it with:
    //
    //     %input_id = Load %variable_id
    merge_block.prepend_instruction(instruction::make_with_result_id!(
        load,
        state.ir_meta,
        input_id,
        variable_id
    ));
    merge_block.input = None;

    // Remove the Merge(%value) from the end of each sub-block, and replace it with:
    //
    //     Store %variable_id %value
    //     Merge
    replace_merge_terminator_with_variable_store(state, block1, variable_id);
    replace_merge_terminator_with_variable_store(state, block2, variable_id);
}

fn replace_merge_terminator_with_variable_store(
    state: &mut State,
    block: &mut Option<Box<Block>>,
    variable_id: TypedId,
) {
    if let Some(block) = block {
        let block = block.get_merge_chain_last_block_mut();
        let merge_id = block.get_terminating_op().get_merge_parameter().unwrap();
        block.unterminate();

        //     Store %variable_id %value
        block.add_instruction(instruction::make!(store, state.ir_meta, variable_id, merge_id));
        //     Merge
        block.terminate(OpCode::Merge(None));
    }
}

// TODO: add lots of tests where there's a continue expression and there's a continue
// instruction in all sorts of situations, nested for and while and for, continue
// inside a switch case, etc.

fn transform_continue_instructions_pre_visit<'block>(
    state: &mut State,
    block: &'block mut Block,
) -> &'block mut Block {
    // Transformation 1:
    //
    // When a loop is encountered that has a continue block, that block is removed and
    // instead replicated before every `Continue` branch in the sub-blocks (that correspond
    // to this loop, and not a nested loop).  This is implemented by visiting the blocks
    // in the following way:
    //
    // - When a block terminating with `Loop` or `DoLoop` is visited, check if there is a continue
    //   block.  If so, remove it and push to `continue_stack`.  Otherwise push a None to
    //   `continue_stack`.  Loop and DoLoop are the only constructs that can be the target of a
    //   `continue` instruction, and the above enables matching continue blocks to their
    //   corresponding loops.
    // - When a block terminating with `Continue` is visited, replicate the block at the top of the
    //   stack, if any, and place it after this block.  Note that the continue block already
    //   terminates with `Continue`, and it is sufficient to change the terminating branch of this
    //   block to `NextBlock` and setting the replicated continue block as the merge block.
    // - After the `Loop` and `DoLoop` blocks are visited, pop the corresponding entry from
    //   `continue_stack`.

    // Transformation 2:
    //
    // When a do-loop is encountered, the condition block is removed, its `LoopIf
    // %condition` is changed to `if (!condition) break;`, and is replicated before every
    // `Continue` branch in the sub-blocks (that correpsond to this loop).  This is
    // implemented as follows:
    //
    // - When a block terminating with `Loop`, `DoLoop` or `Switch` is visited, add an entry to
    //   `break_stack` to know which construct a nested `Break` would affect.
    // - When a block terminating with `Loop` is visited, add `None` to `condition_stack` just to
    //   indicate that `continue` applies to `Loop`, and not a `DoLoop`.
    // - When a block terminating with `DoLoop` is visited, take the condtion block and place it in
    //   `condition_stack`.
    // - When a block terminating with `Continue` is visited and there is a block at the
    // top of the `condition_stack` :
    //   - Inspect the top elements of `break_stack`, and add a variable (if not already) to all
    //     that are of `switch` type.
    //   - Replicate the condition block, and place it after this block.  The condition block is
    //     transformed such that instead of `LoopIf %condition`, it terminates with an `if
    //     (!condition) { /* set all relevant switch vars to true */ break; }` followed by a
    //     `Continue` branch.
    //  - Change the terminating branch of this block to `NextBlock` and set the replicated
    //    condition block as the merge block.
    // - After the `Switch` block is visited, check if a new variable was added while visiting the
    //   sub-blocks.  If so, add an `if (propagate_break) break;` in a block after the switch.
    // - After the `Loop`, `DoLoop` and `Switch` blocks are visited, pop the corresponding entries
    //   from `break_stack` and `condition_stack`.
    //
    // Note that the conditions for the two transformations are mutually exclusive (one
    // affects `for` loops, i.e. `Loop`, and the other do-whiles, i.e. `DoLoop`), which is
    // why they can be easily implemented in one pass.
    let mut current_block = block;

    let op = current_block.get_terminating_op();
    match op {
        OpCode::Loop => transform_continue_pre_visit_loop(state, current_block),
        OpCode::DoLoop => transform_continue_pre_visit_do_loop(state, current_block),
        OpCode::Switch(..) => transform_continue_pre_visit_switch(state, current_block),
        OpCode::Continue => {
            current_block = transform_continue_visit_continue(state, current_block);
        }
        _ => (),
    };
    current_block
}

fn transform_continue_pre_visit_loop(state: &mut State, block: &mut Block) {
    // Transformation 1: Take the loop continue block and push in `continue_stack`.
    let continue_block = block.block2.take().map(|block| *block);
    state.continue_stack.push(continue_block);

    // Transformation 2: Add an entry to `break_stack`
    state.break_stack.push(BreakInfo::new_loop());
    // Transformation 2: Add an entry to `condition_stack`
    state.condition_stack.push(None);
}

fn transform_continue_pre_visit_do_loop(state: &mut State, block: &mut Block) {
    // Transformation 1: Add an entry to `continue_stack`
    debug_assert!(block.block2.is_none());
    state.continue_stack.push(None);

    // Transformation 2: Add an entry to `break_stack`
    state.break_stack.push(BreakInfo::new_loop());
    // Transformation 2: Take the do-loop condition block and push in `condition_stack`.
    let condition_block = block.loop_condition.take().map(|block| *block);
    state.condition_stack.push(condition_block);
}

fn transform_continue_pre_visit_switch(state: &mut State, _block: &mut Block) {
    // Transformation 2: Add an entry to `break_stack`
    state.break_stack.push(BreakInfo::new_switch());
}

fn transform_continue_visit_continue<'block>(
    state: &mut State,
    block: &'block mut Block,
) -> &'block mut Block {
    let continue_block = state.continue_stack.last().unwrap().as_ref();
    let condition_block = state.condition_stack.last().unwrap().as_ref();

    let block_to_append = if let Some(continue_block) = continue_block {
        // Transformation 1: If there is a continue block at the top of the stack, the
        // `continue` corresponds to a GLSL `for` loop.  Replicate that before the
        // `Continue` instruction.
        util::duplicate_block(state.ir_meta, &mut HashMap::new(), continue_block)
    } else if let Some(condition_block) = condition_block {
        // Transformation 2: If there is a condition block at the top of the stack, the
        // `continue` corresponds to a GLSL `do-while` loop.  Add helper variables to
        // enclosing switches, if any, replicate the condition block and transform it to be
        // placed before this particular `continue`.
        let mut duplicate =
            util::duplicate_block(state.ir_meta, &mut HashMap::new(), condition_block);

        let variables_to_set = transform_continue_add_variable_to_enclosing_switch_blocks(state);
        transform_continue_adjust_condition_block(state, &mut duplicate, variables_to_set);

        duplicate
    } else {
        return block;
    };

    block.unterminate();
    block.terminate(OpCode::NextBlock);
    block.set_merge_block(block_to_append);
    block.get_merge_chain_last_block_mut()
}

fn transform_continue_add_variable_to_enclosing_switch_blocks(
    state: &mut State,
) -> Vec<VariableId> {
    let mut variables_to_set = Vec::new();

    // For each enclosing `switch` block for this `continue` instruction, add a
    // `propagate_break` variable to be used to jump out of the switch blocks and continue
    // the loop.
    for scope in state.break_stack.iter_mut() {
        if !scope.is_switch {
            break;
        }

        let variable_id = match scope.propagate_break_var {
            Some(variable_id) => variable_id,
            None => state.ir_meta.declare_variable(
                Name::new_temp("propagate_break"),
                TYPE_ID_BOOL,
                Precision::NotApplicable,
                Decorations::new_none(),
                None,
                Some(CONSTANT_ID_FALSE),
            ),
        };
        variables_to_set.push(variable_id);
    }

    variables_to_set
}

fn transform_continue_adjust_condition_block(
    state: &mut State,
    block: &mut Block,
    variables_to_set: Vec<VariableId>,
) {
    let block = block.get_merge_chain_last_block_mut();

    // Since this is the condition block of a do-while loop, it must terminate with a
    // `LoopIf`.  Take the condition out of it.
    let condition = block.get_terminating_op().get_loop_condition();
    block.unterminate();

    // Create another block that only contains the `Continue` branch, and set it as the
    // merge block of the original block.
    let mut continue_block = Block::new();
    continue_block.terminate(OpCode::Continue);
    block.set_merge_block(continue_block);

    // Create a block that sets the given variables all to true, and ends in `Break`.
    let mut break_block = Block::new();
    let constant_true = TypedId::from_constant_id(CONSTANT_ID_TRUE, TYPE_ID_BOOL);
    for variable_id in variables_to_set {
        let variable_id = TypedId::from_bool_variable_id(variable_id);
        break_block.add_void_instruction(OpCode::Store(variable_id, constant_true));
    }
    break_block.terminate(OpCode::Break);

    // Terminate the current block with an `If` of the logical not of the condition, where
    // the true block is `break_block`.
    let not_instruction = instruction::make!(logical_not, state.ir_meta, condition);
    let not_condition = not_instruction.get_result_id();
    block.add_instruction(not_instruction);
    block.terminate(OpCode::If(not_condition));
    block.set_if_true_block(break_block);
}

fn transform_continue_instructions_post_visit<'block>(
    state: &mut State,
    block: &'block mut Block,
) -> &'block mut Block {
    let mut current_block = block;

    let op = current_block.get_terminating_op();
    match op {
        OpCode::Loop | OpCode::DoLoop => transform_continue_post_visit_loop(state, current_block),
        OpCode::Switch(..) => {
            current_block = transform_continue_post_visit_switch(state, current_block);
        }
        _ => (),
    };
    current_block
}

fn transform_continue_post_visit_loop(state: &mut State, _block: &mut Block) {
    // Pop the corresponding entries from the stacks
    state.continue_stack.pop();
    state.break_stack.pop();
    state.condition_stack.pop();
}

fn transform_continue_post_visit_switch<'block>(
    state: &mut State,
    block: &'block mut Block,
) -> &'block mut Block {
    // Pop from `break_stack` only.
    let break_info = state.break_stack.pop().unwrap();

    // Check if a new variable was supposed to be added to the block.  If so, declare it,
    // and add an `if (propagate_break) break;` to the block.
    if let Some(variable_id) = break_info.propagate_break_var {
        block.variables.push(variable_id);

        // A block that only has `Break`.
        let mut break_block = Block::new();
        break_block.terminate(OpCode::Break);

        // A block that contains:
        //
        //     %value = Load %variable
        //     If %value
        let mut propagate_break_block = Block::new();
        let load_instruction =
            instruction::make!(load, state.ir_meta, TypedId::from_bool_variable_id(variable_id));
        let load_value = load_instruction.get_result_id();
        propagate_break_block.add_instruction(load_instruction);
        propagate_break_block.terminate(OpCode::If(load_value));
        propagate_break_block.set_if_true_block(break_block);

        // Insert the new block after the one containing `Switch` (i.e. `block`) in the
        // merge chain.  This function's return value ensures that the newly added block is
        // not revisited.
        block.insert_into_merge_chain(propagate_break_block)
    } else {
        block
    }
}
