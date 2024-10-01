// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// The IR builder connects to ParseContext.cpp, turning the parse output to an IR.  It is not meant
// to be used by transformations, as they can generate IR more efficiently.

use crate::ir::*;
use crate::*;

// Used when building the IR from GLSL, holds an intermediate block to be filled with instructions.
#[cfg_attr(debug_assertions, derive(Debug))]
struct IntermediateBlock {
    block: Block,
    is_merge_block: bool,
    // Once the block is terminated with return, break, continue or discard, any new instructions
    // that are encountered are dead code.  Those instructions are processed as usual, but don't
    // result in any changes to the block.
    new_instructions_are_dead_code: bool,
}

impl IntermediateBlock {
    fn new() -> IntermediateBlock {
        IntermediateBlock {
            block: Block::new(),
            is_merge_block: false,
            new_instructions_are_dead_code: false,
        }
    }

    // Used with debug_assert!, ensures that the block does not have any left overs.
    fn is_reset(&self) -> bool {
        self.block.variables.is_empty()
            && self.block.instructions.is_empty()
            && self.block.merge_block.is_none()
            && self.block.block1.is_none()
            && self.block.block2.is_none()
            && self.block.case_blocks.is_empty()
            && self.block.input.is_none()
            && !self.is_merge_block
            && !self.new_instructions_are_dead_code
    }

    fn reset(&mut self) {
        self.block.input = None;
        self.is_merge_block = false;
        self.new_instructions_are_dead_code = false;

        debug_assert!(self.is_reset());
    }
}

// Helper to construct the CFG of a function.
#[cfg_attr(debug_assertions, derive(Debug))]
struct CFGBuilder {
    // The current block that is being built.  Once the block is complete it will be pushed to the
    // `interm_blocks` stack if necessary
    //
    // Note that `current_block` is logically the top of `interm_block` stack, but is created
    // separately to avoid lots of `panic!`s as otherwise Rust doesn't know there is always at
    // least one element on the stack.
    current_block: IntermediateBlock,
    // The blocks of the CFG that is being built.  Once a function is complete, this stack
    // should be empty.
    interm_blocks: Vec<IntermediateBlock>,
}

impl CFGBuilder {
    fn new() -> CFGBuilder {
        CFGBuilder { current_block: IntermediateBlock::new(), interm_blocks: Vec::new() }
    }

    fn is_empty(&self) -> bool {
        self.current_block.is_reset() && self.interm_blocks.is_empty()
    }

    // When a new GLSL block is encountered that affects the control flow (body of a function, true
    // or false block of an if, etc), `current_block` starts off as a non-merge block.
    //
    // If the GLSL block finishes without divergance, `current_block` remains as non-merged.  If
    // the GLSL block diverges, `current_block` is pushed onto stack and a new non-merge
    // `current_block` is created.  Once merged (for example end of `if`) a new merge
    // `current_block` is created.  This may happen multiple times, resulting in a set of nodes on
    // the stack and one node as `current_block`.
    //
    // Note again that `current_block` is logically the top element of the stack.  Once the GLSL
    // block finishes, the stack (including `current_block`) is traversed until a non-merge block
    // is found; the blocks that are popped in the process are chained via their `merge_block`.
    // The non-merge block itself is then placed where appropriate, (such as Function::entry,
    // Block::block1 of top of stack, etc)
    //
    // For `for` and `while` loops, before the condition is processed, `current_block` is pushed to
    // the stack and a new non-merge block is created (because it needs to be executed multiple
    // times), which will contain the condition only.  For `for` loops if there is a continue
    // expression, similarly `current_block` is pushed to the stack and a new non-merge block is
    // created.  Once the loop is finished, not only is the body popped but also the continue and
    // condition blocks.
    //
    // For `do-while` loops, the only difference is that the condition block is placed on top of
    // the body block instead of under.

    fn take_block(block: &mut IntermediateBlock) -> IntermediateBlock {
        std::mem::replace(block, IntermediateBlock::new())
    }

    // Push a new non-merge block on the stack
    fn push_block(&mut self) {
        debug_assert!(self.current_block.block.is_terminated());
        self.interm_blocks.push(Self::take_block(&mut self.current_block));
        self.current_block.reset();
    }

    // Extract a chain of merge blocks until a non-merge block is reached.  The blocks are linked
    // together by their `merge_block`.
    fn pop_block(&mut self) -> Block {
        debug_assert!(self.current_block.block.is_terminated());
        let mut result = Self::take_block(&mut self.current_block);

        while result.is_merge_block {
            let mut parent_block = self.interm_blocks.pop().unwrap();
            parent_block.block.set_merge_block(result.block);
            result = parent_block;
        }

        result.block
    }

    fn are_new_instructions_dead_code_in_parent_block(&self) -> bool {
        self.interm_blocks.last().unwrap().new_instructions_are_dead_code
    }

    fn add_variable_declaration(&mut self, variable_id: VariableId) {
        if !self.current_block.new_instructions_are_dead_code {
            self.current_block.block.add_variable_declaration(variable_id);
        }
    }

    // Add an instruction to the current block.  This may be a break, continue, return or discard,
    // in which case the block terminates early.  If another instruction is added after the block
    // is terminated, it is dropped as dead-code.
    fn add_void_instruction(&mut self, op: OpCode) {
        if !self.current_block.new_instructions_are_dead_code {
            if matches!(op, OpCode::Discard | OpCode::Return(_) | OpCode::Break | OpCode::Continue)
            {
                self.current_block.new_instructions_are_dead_code = true;
            }

            self.current_block.block.add_void_instruction(op);
        }
    }
    fn add_typed_instruction(&mut self, id: RegisterId) {
        if !self.current_block.new_instructions_are_dead_code {
            self.current_block.block.add_typed_instruction(id);
        }
    }

    fn begin_if_true_block(&mut self, condition: TypedId) {
        // TODO: add a test with nested dead code like:
        //
        //     ...
        //     break;
        //     if (something)
        //     {
        //        ...
        //        continue;
        //        if (something else)
        //        {
        //            ...
        //        }
        //     }

        // The current block should terminate with a jump based on the condition.  It is pushed to
        // the stack.
        if !self.current_block.new_instructions_are_dead_code {
            self.current_block.block.terminate(OpCode::If(condition));
        }
        self.push_block();
    }

    fn end_if_body_block_common<SetBlock>(
        &mut self,
        merge_param: Option<TypedId>,
        set_block: &SetBlock,
    ) where
        SetBlock: Fn(&mut Block, Block),
    {
        if !self.current_block.block.is_terminated() {
            self.current_block.block.terminate(OpCode::Merge(merge_param));
        }
        let true_or_false_block = self.pop_block();
        if !self.are_new_instructions_dead_code_in_parent_block() {
            set_block(&mut self.interm_blocks.last_mut().unwrap().block, true_or_false_block);
        }
    }

    fn end_if_true_block(&mut self, merge_param: Option<TypedId>) {
        // Take the block and set it as the true block of the previously pushed if header block.
        self.end_if_body_block_common(merge_param, &|if_block, true_block| {
            if_block.set_if_true_block(true_block)
        });
    }

    fn begin_if_false_block(&mut self) {
        // The true block of the block is already processed, so there is nothing in the
        // `current_block`, and it's ready to record the instructions of the false block already.
        debug_assert!(self.current_block.is_reset());
    }

    fn end_if_false_block(&mut self, merge_param: Option<TypedId>) {
        // Take the block and set it as the false block of the previously pushed if header block.
        self.end_if_body_block_common(merge_param, &|if_block, false_block| {
            if_block.set_if_false_block(false_block)
        });
    }

    fn end_if(&mut self, input: Option<TypedRegisterId>) -> Option<TypedRegisterId> {
        // The true and else blocks have been processed.  The header of the if is already at the
        // top of the stack.  Prepare a merge block to continue what comes after the if.
        //
        // If the entire `if` was dead-code eliminated however, restore the previous (already
        // terminated) block
        if self.are_new_instructions_dead_code_in_parent_block() {
            self.current_block = self.interm_blocks.pop().unwrap();
            return None;
        }

        debug_assert!(self.current_block.is_reset());

        // If the condition of the if is a constant, it can be constant-folded.
        let mut if_block = self.interm_blocks.pop().unwrap();
        let if_condition = if_block.block.get_terminating_op().get_if_condition().id.get_constant();

        match if_condition {
            Some(condition) => {
                // Remove the if instruction from the block and revive it as the current block.
                if_block.block.unterminate();
                self.current_block = if_block;

                let true_block = self.current_block.block.block1.take();
                let false_block = self.current_block.block.block2.take();

                // Append the true or false block based on condition.  At the same time,
                // replace the `input` id with the parameter of the block that is replacing the
                // if.
                self.const_fold_if_with_block(if condition == CONSTANT_ID_TRUE {
                    true_block
                } else {
                    false_block
                })
            }
            None => {
                // Not a constant, push the if block back and make the current block a merge
                // block.
                self.interm_blocks.push(if_block);
                self.current_block.is_merge_block = true;
                self.current_block.block.input = input;
                None
            }
        }
    }

    fn const_fold_if_with_block(&mut self, block: Option<Box<Block>>) -> Option<TypedRegisterId> {
        match block {
            Some(block) => {
                // Make the current block jump to this block.
                self.current_block.block.terminate(OpCode::NextBlock);
                self.push_block();

                self.current_block.block = *block;
                self.current_block.is_merge_block = true;

                // If there was a merge parameter, replace the merge input with this id directly.
                let merge_param =
                    self.current_block.block.get_terminating_op().get_merge_parameter();
                self.current_block.block.unterminate();

                merge_param.map(|id| id.to_register_id())
            }
            None => {
                // The if should be entirely eliminated, as there is nothing to replace it with.
                // For example, `if (false) { only_true_block(); }`.
                //
                // The if header (code leading to the if) is already set as `current_block` and
                // unterminated, so there is nothing to do.
                None
            }
        }
    }

    fn begin_loop_condition(&mut self) {
        // TODO: add a test where a loop is in dead code.

        // Loops have a condition to evaluate every time, so a new block is created for that.
        if !self.current_block.new_instructions_are_dead_code {
            self.current_block.block.terminate(OpCode::Loop);
        }
        self.push_block();
    }

    fn end_loop_condition_common(&mut self, condition: TypedId) {
        // There cannot be control flow in the condition expression, so this block must always be
        // alive.
        debug_assert!(!self.current_block.new_instructions_are_dead_code);
        self.current_block.block.terminate(OpCode::LoopIf(condition));
        let condition_block = self.pop_block();

        if !self.are_new_instructions_dead_code_in_parent_block() {
            self.interm_blocks.last_mut().unwrap().block.set_loop_condition_block(condition_block);
        }
    }

    fn end_loop_condition(&mut self, condition: TypedId) {
        self.end_loop_condition_common(condition);

        // Body of the loop is recorded in the current (empty) block.
        debug_assert!(self.current_block.is_reset());
    }

    fn is_loop_condition_empty(&self) -> bool {
        // Returns true if the loop condition is implicitly true, as in `for(;;)`.  This is the
        // case if no instructions are generated for the condition whatsoever.
        self.current_block.block.instructions.is_empty()
    }

    fn end_loop_continue(&mut self) {
        // There cannot be control flow in the continue expression, so this block must always be
        // alive.
        debug_assert!(!self.current_block.new_instructions_are_dead_code);
        self.current_block.block.terminate(OpCode::Continue);

        let continue_block = self.pop_block();

        if !self.are_new_instructions_dead_code_in_parent_block() {
            self.interm_blocks.last_mut().unwrap().block.set_loop_continue_block(continue_block);
        }
    }

    fn end_loop(&mut self) {
        if !self.current_block.new_instructions_are_dead_code {
            self.current_block.block.terminate(OpCode::Continue);
        }

        // Retrieve and set the body block of the loop.  The header of the loop is already at the
        // top of the stack.  Prepare a merge block to continue what comes after the loop.
        //
        // If the entire loop was dead-code eliminated however, restore the previous (already
        // terminated) block.
        let body_block = self.pop_block();

        if self.are_new_instructions_dead_code_in_parent_block() {
            self.current_block = self.interm_blocks.pop().unwrap();
            return;
        }

        debug_assert!(self.current_block.is_reset());

        // If the condition of the loop is a constant false, it can be constant-folded.
        let mut loop_block = self.interm_blocks.pop().unwrap();
        let loop_condition = loop_block
            .block
            .loop_condition
            .as_ref()
            .unwrap()
            .get_merge_chain_terminating_op()
            .get_loop_condition()
            .id
            .get_constant();

        match loop_condition {
            Some(condition) if condition == CONSTANT_ID_FALSE => {
                // Remove the loop instruction from the block and revive it as the current block.
                loop_block.block.unterminate();
                self.current_block = loop_block;

                // Remove the condition block.
                self.current_block.block.loop_condition.take();

                // Remove the continue block that might have been added, if any.
                debug_assert!(self.current_block.block.block1.is_none());
                self.current_block.block.block2.take();
            }
            _ => {
                // Not a constant false, push the loop block back and make the current block a
                // merge block.
                loop_block.block.set_loop_body_block(body_block);

                self.interm_blocks.push(loop_block);
                self.current_block.is_merge_block = true;
            }
        }
    }

    fn begin_do_loop(&mut self) {
        // Insert a new block for the body of the do-loop.
        if !self.current_block.new_instructions_are_dead_code {
            self.current_block.block.terminate(OpCode::DoLoop);
        }
        self.push_block();
    }

    fn begin_do_loop_condition(&mut self) {
        // Retrieve and set the body block of the do-loop.  The header of the do-loop is already at
        // the top of the stack.
        if !self.current_block.block.is_terminated() {
            self.current_block.block.terminate(OpCode::Continue);
        }
        let body_block = self.pop_block();

        if !self.are_new_instructions_dead_code_in_parent_block() {
            self.interm_blocks.last_mut().unwrap().block.set_loop_body_block(body_block);
        }

        // Loops have a condition to evaluate every time, which is recorded in the current (empty)
        // block.
        debug_assert!(self.current_block.is_reset());
    }

    fn end_do_loop(&mut self, condition: TypedId) {
        self.end_loop_condition_common(condition);

        // Prepare a merge block to continue what comes after the do-loop.  If the entire do-loop
        // was dead-code eliminated however, restore the previous (already terminated) block.
        if self.are_new_instructions_dead_code_in_parent_block() {
            self.current_block = self.interm_blocks.pop().unwrap();
            return;
        }

        // Make the current block a merge block.
        debug_assert!(self.current_block.is_reset());
        self.current_block.is_merge_block = true;
    }

    fn begin_switch(&mut self, value: TypedId) {
        // Insert a new block for the first case.
        if !self.current_block.new_instructions_are_dead_code {
            self.current_block.block.terminate(OpCode::Switch(value, Vec::new()));
        }
        self.push_block();
    }

    fn begin_case_impl(&mut self, value: Option<ConstantId>) {
        // Add the value to the switch's case blocks.
        if !self.are_new_instructions_dead_code_in_parent_block() {
            self.interm_blocks
                .last_mut()
                .unwrap()
                .block
                .get_terminating_op_mut()
                .add_switch_case(value);
        }
    }

    fn begin_case(&mut self, value: TypedId) {
        self.begin_case_impl(value.id.get_constant());
    }

    fn begin_default(&mut self) {
        self.begin_case_impl(None);
    }

    fn end_case(&mut self) {
        // If the block is terminated, nothing to do, just add it to the list of switch's case
        // blocks.  If it isn't terminated, terminate it with a `Passthrough` first.
        if !self.current_block.block.is_terminated() {
            self.current_block.block.terminate(OpCode::Passthrough);
        }

        let case_block = self.pop_block();
        if !self.are_new_instructions_dead_code_in_parent_block() {
            self.interm_blocks.last_mut().unwrap().block.add_switch_case_block(case_block);
        }
    }

    // Check if the switch in the block at the top the stack has a matching case (e.g. if it uses a
    // constant expression) and what the matching block is (if any).
    fn switch_has_matching_case(switch_block: &mut Block) -> (bool, Option<usize>) {
        let (switch_expr, switch_cases) =
            switch_block.get_terminating_op().get_switch_expression_and_cases();

        // If there are no cases, the switch is a no-op
        if switch_cases.is_empty() {
            return (false, None);
        }

        // If the expression is constant, but no matching cases (or default) exists, the switch is
        // a no-op.
        if let Some(switch_expr) = switch_expr.id.get_constant() {
            let any = switch_cases
                .iter()
                .position(|case| case.map(|case_id| case_id == switch_expr).unwrap_or(true));
            return (any.is_some(), any);
        }

        // Expression is not a constant, assume the switch is not no-op.
        return (true, None);
    }

    fn end_switch(&mut self) {
        // The case blocks have already been processed.  The header of the switch is already at the
        // top of the stack.  Prepare a merge block to continue what comes after the switch.
        //
        // If the entire `switch` was dead-code eliminated however, restore the previous (already
        // terminated) block
        if self.are_new_instructions_dead_code_in_parent_block() {
            self.current_block = self.interm_blocks.pop().unwrap();
            return;
        }

        debug_assert!(self.current_block.is_reset());

        let mut switch_block = self.interm_blocks.pop().unwrap();

        // If the last block didn't explicitly `break`, it is terminated with `Passthrough`.  Fix
        // that up to become `Break`.
        if let Some(last_case_block) = switch_block.block.case_blocks.last_mut() {
            if matches!(last_case_block.get_terminating_op(), OpCode::Passthrough) {
                last_case_block.unterminate();
                last_case_block.terminate(OpCode::Break);
            }
        }

        // TODO: add constant folding tests, especially a switch that matches a case, but that case
        // falls through to other cases.

        // There are a couple of possibilities to constant fold the switch:
        //
        // - The switch has no cases: In this case, it can simply be removed.
        // - The switch has a constant expression, but none of the cases match it: In this case, the
        //   switch can be removed as well.
        // - The switch has a constant expression with a matching case: In this case, it can be
        //   replaced with the matching block (and subsequent passthrough blocks).

        let (has_matching_case, matching_block_index) =
            Self::switch_has_matching_case(&mut switch_block.block);
        if !has_matching_case || matching_block_index.is_some() {
            // Remove the switch instruction from the block and revive it as the current block.
            switch_block.block.unterminate();
            self.current_block = switch_block;

            // Remove the case blocks.
            let case_blocks = std::mem::take(&mut self.current_block.block.case_blocks);

            if let Some(matching_block_index) = matching_block_index {
                // Starting from the matching block, take blocks while their terminator is
                // `Passthrough` and then the final block with a different terminator.  Note that
                // take_while stops _after_ the first element that doesn't match the predicate,
                // i.e. that element is lost.  Because of that, the first element after
                // `Passthrough` is also matched.
                let mut first_block_after_passthrough = true;
                let matching_blocks: Vec<Block> = case_blocks
                    .into_iter()
                    .skip(matching_block_index)
                    .take_while(|block| {
                        matches!(block.get_terminating_op(), &OpCode::Passthrough) || {
                            let is_last_block = first_block_after_passthrough;
                            first_block_after_passthrough = false;
                            is_last_block
                        }
                    })
                    .collect();

                // There are case blocks to replace the switch, chain them and push to the stack:
                for mut block in matching_blocks.into_iter().rev() {
                    self.current_block.block.terminate(OpCode::NextBlock);
                    self.push_block();

                    block.unterminate();
                    self.current_block.block = block;
                    self.current_block.is_merge_block = true;
                }
            }
        } else {
            // Cannot fold, push the switch block back and make the current block a merge block.
            self.interm_blocks.push(switch_block);
            self.current_block.is_merge_block = true;
        }
    }
}

// A helper to build the IR from scratch.  The helper is invoked while parsing the shader, and
// its main purpose is to maintain in-progress items until completed, and adapt the incoming GLSL
// syntax to the IR.
// TODO: This should end up in FFI
#[cfg_attr(debug_assertions, derive(Debug))]
pub struct Builder {
    // The IR being built
    // TODO: at the end, before destroying Builder, move this out to a Box<IR> so CPP can pass it
    // around.
    ir: IR,

    // The current function that is being built (if any).
    current_function: Option<FunctionId>,

    // The CFG of the current function being built, if any.
    current_function_cfg: CFGBuilder,

    // The CFG of the initialization code of global variables.
    //
    // Global variables can either be initialized with constants or non-constant expressions.  If
    // initialized with a constant, the Variable::initializer can be used, but that's impossible
    // with non-constant initializers; there may be complex code (with multiple instructions) to
    // execute before that can happen.  To support that, when code is encountered in the global
    // scope, it is generated in `global_initializers_cfg` and when it's assigned to a variable,
    // a Store op is added to the top block of the cfg.
    //
    // At the end of parsing, this is prepended to the CFG of main().
    global_initializers_cfg: CFGBuilder,

    // Stack of intermediate values.  As the parser observes expressions, it pushes them on the
    // stack.  When these intermediate expressions are combined in an operation, the corresponding
    // operation is popped off the stack.  The exact number of ids to pop depends on the
    // instruction.
    //
    // This stack should be empty after every statement in a GLSL block is completely processed.
    // It should also be empty at the end of every Block.
    interm_ids: Vec<TypedId>,

    // gl_ClipDistance and gl_CullDistance may be sized after `.length()` is called on them.  To
    // support this, a global variable is created to hold that yet-to-be-determined constant.  When
    // the size of gl_ClipDistance and gl_CullDistance is determined, that variable is initialized
    // with it.
    gl_clip_distance_length_var_id: Option<VariableId>,
    gl_cull_distance_length_var_id: Option<VariableId>,
    // TODO: Look at ParseContext.cpp and add everything it needs!
    // TODO: For things that are used by ParseContext.cpp to verify the shader, don't move them
    // over yet.  Most notably, keep the symbol table in cpp.
}

enum VariableScope {
    Global,
    Local,
    FunctionParam,
}

impl Builder {
    pub fn new(shader_type: ShaderType) -> Builder {
        Builder {
            ir: IR::new(shader_type),
            current_function: None,
            current_function_cfg: CFGBuilder::new(),
            global_initializers_cfg: CFGBuilder::new(),
            interm_ids: Vec::new(),
            gl_clip_distance_length_var_id: None,
            gl_cull_distance_length_var_id: None,
        }
    }

    // Called at the end of the shader to finalize it.  Notably, this moves the cached global
    // initialization code to the beginning of `main()`.
    pub fn finish(&mut self) {
        debug_assert!(self.ir.meta.get_main_function_id().is_some());

        if !self.global_initializers_cfg.is_empty() {
            self.global_initializers_cfg.current_block.block.terminate(OpCode::NextBlock);
            let global_init_block = self.global_initializers_cfg.pop_block();
            self.ir.prepend_to_main(global_init_block);
        }

        // Instantly dead-code-eliminate unreachable functions.
        let function_decl_order =
            util::calculate_function_decl_order(&self.ir.meta, &self.ir.function_entries);
        let function_count = self.ir.function_entries.len();
        // Take all entry blocks out ...
        let mut function_entries = std::mem::replace(&mut self.ir.function_entries, vec![]);
        self.ir.function_entries.resize_with(function_count, || None);
        // ... and only place back the ones that are reachable from `main` (i.e. are in the DAG).
        for function_id in function_decl_order {
            let id = function_id.id as usize;
            self.ir.function_entries[id] = std::mem::replace(&mut function_entries[id], None);
        }
    }

    // Get a reference to the IR for direct access, such as for doing type look up etc, avoiding
    // the need for a set of forwarding functions.
    pub fn ir(&mut self) -> &mut IR {
        &mut self.ir
    }
    pub fn take_ir(&mut self) -> IR {
        std::mem::replace(&mut self.ir, IR::new(ShaderType::Vertex))
    }

    // Internal helper to declare a new variable.
    fn declare_variable(
        &mut self,
        name: Name,
        type_id: TypeId,
        precision: Precision,
        decorations: Decorations,
        built_in: Option<BuiltIn>,
        scope: VariableScope,
    ) -> VariableId {
        let variable_id =
            self.ir.meta.declare_variable(name, type_id, precision, decorations, built_in, None);

        // Add the variable to either the list of global variables to be declared or the list of
        // local variables in this scope.  Function parameters are part of the `Function` and so
        // don't need to be explicitly marked as needing declaration.
        match scope {
            VariableScope::Global => self.ir.meta.add_to_global_variables(variable_id),
            VariableScope::Local => self.current_function_cfg.add_variable_declaration(variable_id),
            VariableScope::FunctionParam => (),
        };

        variable_id
    }

    // Declare a built-in variable.  Must have never been declared before.
    pub fn declare_builtin_variable(
        &mut self,
        built_in: BuiltIn,
        type_id: TypeId,
        precision: Precision,
        decorations: Decorations,
    ) -> VariableId {
        // Note: the name of the built-in is not derived.  For text-based generators, the name can
        // be derived from the built-in enum, and the string is unnecessary.  For SPIR-V, the name
        // is unused.
        //
        // Built-ins are always globally declared.
        self.declare_variable(
            Name::new_exact(""),
            type_id,
            precision,
            decorations,
            Some(built_in),
            VariableScope::Global,
        )
    }

    // Declare an interface variable.  Interface variables are special in that their name must be
    // preserved in a predictable form for text-based generators.
    pub fn declare_interface_variable(
        &mut self,
        name: &'static str,
        type_id: TypeId,
        precision: Precision,
        decorations: Decorations,
    ) -> VariableId {
        self.declare_variable(
            Name::new_interface(name),
            type_id,
            precision,
            decorations,
            None,
            VariableScope::Global,
        )
    }

    // Declare a temporary variable.  The name of this variable is kept as closely to the original
    // as possible, but without any guarantees.
    pub fn declare_temp_variable(
        &mut self,
        name: &'static str,
        type_id: TypeId,
        precision: Precision,
        decorations: Decorations,
    ) -> VariableId {
        let scope = if self.current_function.is_none() {
            VariableScope::Global
        } else {
            VariableScope::Local
        };

        self.declare_variable(Name::new_temp(name), type_id, precision, decorations, None, scope)
    }

    // When a function prototype is encountered, the following functions are called:
    //
    //     return_type function @new_function (...)
    //
    // If that is followed by the function definition, then the following functions are called:
    //
    //     return_type function @new_function (type param, ...) @begin_function
    //     {
    //         ...
    //     }
    //     @end_function
    //
    // Note that if the prototype is previously seen, `new_function` is not called again on
    // definition because the function id is already known.

    pub fn new_function(
        &mut self,
        name: &'static str,
        return_type_id: TypeId,
        return_precision: Precision,
    ) -> FunctionId {
        let function = Function::new(name, return_type_id, return_precision);
        let is_main = name == "main";

        let id = self.ir.add_function(function);

        // If this is the main() function, remember its id.
        if is_main {
            self.ir.meta.set_main_function_id(id);
        }

        id
    }

    // Start a new function to build.
    pub fn begin_function(&mut self, id: FunctionId, params: Vec<FunctionParam>) {
        debug_assert!(self.current_function_cfg.is_empty());

        self.current_function = Some(id);
        self.ir.meta.set_function_params(id, params);
    }

    // Declare a parameter for the function that is being defined.  Called for each parameters
    // before calling `begin_function`.  This is not done while parsing the prototype because if it
    // isn't followed by a definition, there is no need to assign variable ids to them.
    pub fn declare_function_param(
        &mut self,
        name: &'static str,
        type_id: TypeId,
        precision: Precision,
        direction: FunctionParamDirection,
    ) -> FunctionParam {
        let variable_id = self.declare_variable(
            Name::new_temp(name),
            type_id,
            precision,
            Decorations::new_none(),
            None,
            VariableScope::FunctionParam,
        );

        FunctionParam::new(variable_id, direction)
    }

    // Once the entire function body is visited, `end_function` puts the graph in the function.
    pub fn end_function(&mut self) {
        // End with a `return` in case not explicitly added.
        self.branch_return();

        self.ir.set_function_entry(
            self.current_function.unwrap(),
            self.current_function_cfg.pop_block(),
        );
        debug_assert!(self.current_function_cfg.is_empty());
        self.current_function = None;

        debug_assert!(self.interm_ids.is_empty());
    }

    // Many instructions are generated either as part of a function or the initialization code for
    // global variables.  The following function should be used so that the generated code is
    // placed in the appropriate scope (function or global).
    fn scope(&mut self) -> &mut CFGBuilder {
        match self.current_function {
            Some(_) => &mut self.current_function_cfg,
            None => &mut self.global_initializers_cfg,
        }
    }

    // Used internally, loads an id from the top of the id stack.  If that is a pointer, its value
    // is loaded before returning.
    fn load(&mut self) -> TypedId {
        let to_load = self.interm_ids.pop().unwrap();

        let result = instruction::load(&mut self.ir.meta, to_load);
        self.add_instruction(result);

        // Actually return the loaded result instead of leaving it on the stack.
        self.interm_ids.pop().unwrap()
    }

    // Store the value at the top of the id stack into the pointer under it.
    pub fn store(&mut self) {
        let value = self.load();
        // The `a = b` expression is itself a value, and we use `a` to load that value back if
        // used, so the pointer is kept on the stack.  Note that the precision of a and b may be
        // different, and if the result of a = b is used, the precision of a is the correct one
        // (already on the stack!).
        let pointer = *self.interm_ids.last().unwrap();

        let result = instruction::store(&mut self.ir.meta, pointer, value);
        self.add_instruction(result);
    }

    // Use the value at the top of the id stack as the initializer for the given variable.  If the
    // initializer is a constant, it will be set in `Variable::initializer`.  Otherwise similarly
    // to `store()`, the non-constant expression is stored with `OpCode::Store` to be deferred to
    // the beginning of `main()`.
    pub fn initialize(&mut self, id: VariableId) {
        let value = self.load();

        match value.id {
            Id::Constant(constant_id) => self.ir.meta.set_variable_initializer(id, constant_id),
            _ => {
                let id = TypedId::from_variable_id(&self.ir.meta, id);
                self.scope().add_void_instruction(OpCode::Store(id, value))
            }
        };
    }

    // Flow control helpers.

    // For `if` support, the following functions are called at the specified points:
    //
    //     if (condition) {
    //         @begin_if_true_block
    //         ...
    //         @end_if_true_block
    //     } else {
    //         @begin_if_false_block
    //         ...
    //         @end_if_false_block
    //     }
    //     @end_if
    //
    // In `begin_if_true_block`, the result of the `condition` is found at the top of the id stack.
    pub fn begin_if_true_block(&mut self) {
        // The condition must be at the top of the stack.
        let condition = self.load();
        self.scope().begin_if_true_block(condition);
    }
    pub fn end_if_true_block(&mut self) {
        self.scope().end_if_true_block(None);
    }
    pub fn begin_if_false_block(&mut self) {
        self.scope().begin_if_false_block();
    }
    pub fn end_if_false_block(&mut self) {
        self.scope().end_if_false_block(None);
    }
    pub fn end_if(&mut self) {
        self.scope().end_if(None);
    }

    // Support for ?: is similar to if/else:
    //
    //     (condition ?
    //         @begin_ternary_true_expression
    //         ...
    //         @end_ternary_true_expression :
    //         @begin_ternary_false_expression
    //         ...
    //         @end_ternary_false_expression) @end_ternary
    //
    // In `begin_if_true_expression`, the result of the `condition` is found at the top of the id
    // stack. In `end_if_true_expression`, the merge parameter is found at the top of the id
    // stack. In `end_if_false_expression`, the merge parameter is found at the top of the id
    // stack.
    //
    // In the end, the result id is found in the input of the merge block of the if/else used to
    // implement it.
    pub fn begin_ternary_true_expression(&mut self) {
        // The condition must be at the top of the stack.
        let condition = self.load();
        self.scope().begin_if_true_block(condition);
    }
    pub fn end_ternary_true_expression(&mut self) {
        let true_value = self.load();
        self.scope().end_if_true_block(Some(true_value));

        // The result of the merge will have the same type as either of the true or false values.
        // Push a new id with this type on the stack so that `end_ternary` would use it to declare
        // the input of the merge block.
        let new_id =
            self.ir.meta.new_register(OpCode::MergeInput, true_value.type_id, true_value.precision);
        self.interm_ids.push(TypedId::from_register_id(new_id));
    }
    pub fn begin_ternary_false_expression(&mut self) {
        self.scope().begin_if_false_block();
    }
    pub fn end_ternary_false_expression(&mut self) {
        let false_value = self.load();
        self.scope().end_if_false_block(Some(false_value));

        // Adjust the precision of the result which was inherited from the true value.
        let result_precision = &mut self.interm_ids.last_mut().unwrap().precision;
        *result_precision =
            instruction::precision::higher_precision(*result_precision, false_value.precision);
    }
    pub fn end_ternary(&mut self) {
        let merge_input = self.interm_ids.pop().unwrap().to_register_id();
        let constant_folded_merge_input = self.scope().end_if(Some(merge_input));
        let result = constant_folded_merge_input.unwrap_or(merge_input);
        self.interm_ids.push(TypedId::new(
            Id::new_register(result.id),
            merge_input.type_id,
            merge_input.precision,
        ));
    }

    // Short circuit support is implemented over ?: and uses the same mechanism:
    //
    //     a || b == a ? true : b
    //     a && b == a ? b : false
    //
    // The following functions are called as specified below:
    //
    //     a @begin_short_circuit_or || b @end_short_circuit_or
    //     a @begin_short_circuit_and && b @end_short_circuit_and
    //
    // In `begin_short_circuit_or/and`, the result of `a` is found at the of the id stack.
    // In `end_short_circuit_or/and`, the result of `b` is found at the of the id stack.
    pub fn begin_short_circuit_or(&mut self) {
        self.begin_ternary_true_expression();
        self.push_constant_bool(true);
        self.end_ternary_true_expression();
        self.begin_ternary_false_expression();
    }
    pub fn end_short_circuit_or(&mut self) {
        self.end_ternary_false_expression();
        self.end_ternary()
    }
    pub fn begin_short_circuit_and(&mut self) {
        self.begin_ternary_true_expression();
    }
    pub fn end_short_circuit_and(&mut self) {
        self.end_ternary_true_expression();
        self.begin_ternary_false_expression();
        self.push_constant_bool(false);
        self.end_ternary_false_expression();
        self.end_ternary()
    }

    // For `while` and `for` support, the following functions are called at the specified points:
    //
    //     while (@begin_loop_condition condition @end_loop_condition) {
    //         ...
    //     }
    //     @end_loop
    //
    //     for (init;
    //          @begin_loop_condition condition @end_loop_condition;
    //          continue @end_loop_continue) {
    //         ...
    //     }
    //     @end_loop
    //
    // In `end_loop_condition`, the result of the `condition` is found at the top of the id stack.
    pub fn begin_loop_condition(&mut self) {
        self.scope().begin_loop_condition();
    }
    pub fn end_loop_condition(&mut self) {
        // The condition must be at the top of the stack, except with `for(...;;...), where the
        // condition is implicitly `true`.  That is detected with the scope for the condition not
        // having any instructions at all.
        if self.scope().is_loop_condition_empty() {
            self.push_constant_bool(true);
        }
        let condition = self.load();
        self.scope().end_loop_condition(condition);
    }
    pub fn end_loop_continue(&mut self) {
        self.scope().end_loop_continue();
    }
    pub fn end_loop(&mut self) {
        self.scope().end_loop();
    }

    // For `do-while` support, the following functions are called at the specified points:
    //
    //     do { @begin_do_loop
    //         ...
    //     @begin_do_loop_condition
    //     } while(condition);
    //     @end_do_loop
    //
    // In `end_do_loop`, the result of the `condition` is found at the top of the id stack.
    pub fn begin_do_loop(&mut self) {
        self.scope().begin_do_loop();
    }
    pub fn begin_do_loop_condition(&mut self) {
        self.scope().begin_do_loop_condition();
    }
    pub fn end_do_loop(&mut self) {
        let condition = self.load();
        self.scope().end_do_loop(condition);
    }

    // For `switch` support, the following functions are called at the specified points:
    //
    //     switch (value @begin_switch) {
    //         case N @begin_case:
    //             ...
    //             @end_case
    //         default @begin_default:
    //             ...
    //             @end_case
    //         ...
    //     }
    //     @end_switch
    //
    // In `begin_switch`, the result of the `value` is found at the top of the id stack.
    //
    // In `begin_case`, the value `N` is found at the top of the id stack.
    //
    // In `end_case`, if the block is not already terminated, it will terminate with `Passthrough`
    // to the next block.
    pub fn begin_switch(&mut self) {
        let value = self.load();
        self.scope().begin_switch(value);
    }
    pub fn begin_case(&mut self) {
        let value = self.load();
        self.scope().begin_case(value);
    }
    pub fn begin_default(&mut self) {
        self.scope().begin_default();
    }
    pub fn end_case(&mut self) {
        self.scope().end_case();
    }
    pub fn end_switch(&mut self) {
        self.scope().end_switch();
    }

    pub fn branch_discard(&mut self) {
        self.add_instruction(instruction::branch_discard());
    }
    pub fn branch_return(&mut self) {
        self.add_instruction(instruction::branch_return(None));
    }
    pub fn branch_return_value(&mut self) {
        let value = self.load();
        self.add_instruction(instruction::branch_return(Some(value)));
    }
    pub fn branch_break(&mut self) {
        self.add_instruction(instruction::branch_break());
    }
    pub fn branch_continue(&mut self) {
        self.add_instruction(instruction::branch_continue());
    }
    pub fn branch_passthrough(&mut self) {
        self.add_instruction(instruction::branch_passthrough());
    }

    // Called when a statement in a block is finished, but only if the statement has a value (i.e.
    // leaves an id on the stack).
    //
    //     statement; @end_statement_with_value
    //
    // or
    //
    //     expression, @end_statement_with_value expression2
    //
    pub fn end_statement_with_value(&mut self) {
        // There _must_ be a value on the stack, unwrap() will assert that.
        self.interm_ids.pop().unwrap();
    }

    pub fn push_id(&mut self, id: Id, type_id: TypeId, precision: Precision) {
        self.interm_ids.push(TypedId::new(id, type_id, precision));
    }

    // Called when constant scalar values are visited.
    pub fn push_constant(&mut self, id: ConstantId, type_id: TypeId) {
        self.push_id(Id::new_constant(id), type_id, Precision::NotApplicable);
    }
    pub fn push_constant_float(&mut self, value: f32) {
        let id = self.ir.meta.get_constant_float(value);
        self.push_constant(id, TYPE_ID_FLOAT);
    }
    pub fn push_constant_int(&mut self, value: i32) {
        let id = self.ir.meta.get_constant_int(value);
        self.push_constant(id, TYPE_ID_INT);
    }
    pub fn push_constant_uint(&mut self, value: u32) {
        let id = self.ir.meta.get_constant_uint(value);
        self.push_constant(id, TYPE_ID_UINT);
    }
    pub fn push_constant_bool(&mut self, value: bool) {
        let id = self.ir.meta.get_constant_bool(value);
        self.push_constant(id, TYPE_ID_BOOL);
    }
    pub fn push_constant_yuv_csc_standard(&mut self, value: YuvCscStandard) {
        let id = self.ir.meta.get_constant_yuv_csc_standard(value);
        self.push_constant(id, TYPE_ID_YUV_CSC_STANDARD);
    }

    // Called when a variable is visited.
    pub fn push_variable(&mut self, id: VariableId) {
        let variable = self.ir.meta.get_variable(id);
        let type_id = variable.type_id;
        let precision = variable.precision;

        debug_assert!(matches!(self.ir.meta.get_type(type_id), Type::Pointer(_)));
        self.push_id(Id::new_variable(id), type_id, precision);
    }

    // Depending on whether the instruction::* function returned a Constant, Void or Register,
    // either add a void or typed instruction (if necessary) and push a value to the stack (if
    // necessary).
    fn add_instruction(&mut self, result: instruction::Result) {
        match result {
            instruction::Result::Constant(id, type_id) => {
                self.interm_ids.push(TypedId::from_constant_id(id, type_id))
            }
            instruction::Result::Void(op) => self.scope().add_void_instruction(op),
            instruction::Result::Register(id) => {
                self.scope().add_typed_instruction(id.id);
                self.interm_ids.push(TypedId::from_register_id(id))
            }
            instruction::Result::NoOp(id) => self.interm_ids.push(id),
        };
    }

    // Helper to get N arguments off the stack in the right order.
    fn get_args<Load, T>(n: usize, mut load: Load) -> Vec<T>
    where
        Load: FnMut() -> T,
    {
        let mut args: Vec<_> = (0..n).map(|_| load()).collect();
        args.reverse();
        args
    }

    // Called when a user-defined function is called
    pub fn call_function(&mut self, id: FunctionId) {
        let arg_count = self.ir.meta.get_function(id).params.len();

        // Collect the call arguments.
        //
        // Note: The function parameters are not load()ed because they may be `out` or `inout`,
        // i.e. they could be pointers.
        let args = Self::get_args(arg_count, || self.interm_ids.pop().unwrap());
        let result = instruction::call(&mut self.ir.meta, id, args);
        self.add_instruction(result);
    }

    // A generic helper to make an assignment operator with an op (like +=).  The rhs operand of
    // the binary operator is found at the top of the stack, and the lhs operand is found under it.
    fn assign_and_op<Op>(&mut self, op: Op)
    where
        Op: FnOnce(&mut Self),
    {
        // X op= Y is calculated as X = X op Y.  This is implemented by `Temp = X op Y`, followed
        // by `X = Temp`.  As a preparation, duplicate lhs on the stack so existing functions can
        // be used.
        let rhs = self.interm_ids.pop().unwrap();
        let lhs = *self.interm_ids.last().unwrap();
        self.interm_ids.push(lhs);
        self.interm_ids.push(rhs);

        // Create the op instruction followed by a store to lhs.
        op(self);
        self.store();
    }

    // Called when a component of a vector is taken, like `vector.y`.
    //
    // The vector being indexed is expected to be found at the top of the stack.
    pub fn vector_component(&mut self, component: u32) {
        let vector = self.interm_ids.pop().unwrap();
        let result = instruction::vector_component(&mut self.ir.meta, vector, component);
        self.add_instruction(result);
    }

    // Called when multiple components of a vector are taken, like `vector.yxy`.
    //
    // The vector being indexed is expected to be found at the top of the stack.
    pub fn vector_component_multi(&mut self, components: Vec<u32>) {
        let vector = self.interm_ids.pop().unwrap();

        let result = instruction::vector_component_multi(&mut self.ir.meta, vector, components);
        self.add_instruction(result);
    }

    // Called when an array-like index is taken:
    //
    // - vector[expr]
    // - matrix[expr]
    // - array[expr]
    //
    // The index is expected to be found at the top of the stack, followed by the expression being
    // indexed.
    pub fn index(&mut self) {
        let index = self.load();
        let indexed = self.interm_ids.pop().unwrap();

        let result = instruction::index(&mut self.ir.meta, indexed, index);
        self.add_instruction(result);
    }

    // Called when a field of a struct is selected, like `block.field`.
    //
    // The struct whose field is being selected is expected to be found at the top of the stack.
    pub fn struct_field(&mut self, field_index: u32) {
        let struct_id = self.interm_ids.pop().unwrap();

        let result = instruction::struct_field(&mut self.ir.meta, struct_id, field_index);
        self.add_instruction(result);
    }

    fn trim_constructor_args_to_component_count(
        &mut self,
        args: Vec<TypedId>,
        count: u32,
    ) -> Vec<TypedId> {
        // To simplify this, do a first pass where matrix arguments are split by column.  This lets
        // the following code simply deal with scalars and vectors.
        let mut matrix_expanded_args = Vec::new();
        for id in args {
            let type_info = self.ir.meta.get_type(id.type_id);

            if let &Type::Matrix(_, column_count) = type_info {
                for row in 0..column_count {
                    self.interm_ids.push(id);
                    self.interm_ids.push(TypedId::from_constant_id(
                        self.ir.meta.get_constant_uint(row),
                        TYPE_ID_UINT,
                    ));
                    self.index();
                    matrix_expanded_args.push(self.load());
                }
            } else {
                matrix_expanded_args.push(id);
            }
        }

        let mut result = Vec::new();
        let mut total = 0;

        for id in matrix_expanded_args {
            let type_info = self.ir.meta.get_type(id.type_id);

            if let &Type::Scalar(..) = type_info {
                // Scalar values add a single component, take them directly.
                result.push(id);
                total += 1;
            } else if let &Type::Vector(_, vec_size) = type_info {
                // If the vector is entirely consumed, take it directly.  Otherwise only take
                // enough components to fulfill the given `count`.
                let needed_components = count - total;
                if needed_components >= vec_size {
                    result.push(id);
                    total += vec_size;
                } else {
                    self.interm_ids.push(id);
                    self.vector_component_multi((0..needed_components).collect());
                    result.push(self.load());
                    total = count;
                }
            }

            debug_assert!(total <= count);
            if total >= count {
                break;
            }
        }

        result
    }

    // Trim extra arguments passed to a constructor.  This is only possible with a vector and
    // matrix constructor.
    //
    // Constructing a matrix from another matrix is special, the input matrix may have more or
    // fewer components.  So matNxM(m) is not trimmed by this function.
    //
    // Additionally, array and struct constructors require an exact match of arguments, so no
    // stripping is done.
    fn trim_constructor_args(&mut self, type_id: TypeId, args: Vec<TypedId>) -> Vec<TypedId> {
        let type_info = self.ir.meta.get_type(type_id);

        match type_info {
            // Nothing to do for arrays and structs
            Type::Array(..) | Type::Struct(..) => {
                return args;
            }
            // Early out for matrix-from-matrix constructors.
            Type::Matrix(..) if args.len() == 1 => {
                return args;
            }
            _ => (),
        }

        let total_components = type_info.get_total_component_count(&self.ir.meta);
        self.trim_constructor_args_to_component_count(args, total_components)
    }

    // Called when a constructor is used, like `vec(a, b, c)`.  Constructors take many
    // forms, including having more parameters than needed.  This function corrects that by taking
    // the appropriate number of parameters.  The following are examples of each form:
    //
    // - float(f), vecN(vN), matNxM(mNxM): Replaced with just f, vN and mNxM respectively
    // - float(v): Replaced with float(v.x)
    // - float(m): Replaced with m[0][0]
    // - vecN(f)
    // - vecN(v1.zy, v2.xzy): If number of components exceeds N, extra components are stripped, such
    //   as vecN(v1.zy, v2.x)
    // - vecN(m)
    // - matNxM(f)
    // - matNxM(m)
    // - matNxM(v1.zy, v2.x, ...): Similarly to constructing a vector, extra components are
    //   stripped.
    // - array(elem0, elem1, ...)
    // - Struct(field0, field1, ...)
    //
    // The arguments of the constructor are found on the stack in reverse order.
    pub fn construct(&mut self, type_id: TypeId, arg_count: usize) {
        // Collect the constructor arguments.
        let args = Self::get_args(arg_count, || self.load());

        // In the case of ConstructVectorFromMultiple and ConstructMatrixFromMultiple, there may be
        // more components in the parameters than the constructor needs.  In this step, these
        // components are removed.  Other constructor kinds are unaffected.
        //
        // Note: this can affect the precision of the result if the stripped args would have
        // increased the end precision.  This is such a niche case that we're ignoring it for
        // simplicity.
        let args = self.trim_constructor_args(type_id, args);

        let result = instruction::construct(&mut self.ir.meta, type_id, args);
        self.add_instruction(result);
    }

    pub fn declare_clip_cull_distance_array_length_variable(
        &mut self,
        existing_id: Option<VariableId>,
        name: &'static str,
    ) -> VariableId {
        existing_id.unwrap_or_else(|| {
            let name = Name::new_temp(name);
            self.declare_variable(
                name,
                TYPE_ID_INT,
                Precision::Low,
                Decorations::new_none(),
                None,
                VariableScope::Global,
            )
        })
    }

    fn on_gl_clip_cull_distance_sized(
        &mut self,
        id: VariableId,
        length: u32,
        length_variable: Option<VariableId>,
    ) {
        let type_id = self.ir.meta.get_variable(id).type_id;
        let type_info = self.ir.meta.get_type(type_id);
        debug_assert!(type_info.is_unsized_array());

        let element_type_id = type_info.get_element_type_id().unwrap();
        let sized_type_id = self.ir.meta.get_array_type_id(element_type_id, length);

        // Update the type of the variable.
        self.ir.meta.get_variable_mut(id).type_id = sized_type_id;

        if let Some(length_variable_id) = length_variable {
            let length = self.ir.meta.get_constant_int(length as i32);
            self.ir.meta.get_variable_mut(length_variable_id).initializer = Some(length);
        }
    }

    // Called when the size of gl_ClipDistance is determined.  It updates the type of the
    // gl_ClipDistance variable and initializes the temp variable used to hold its length (if any).
    pub fn on_gl_clip_distance_sized(&mut self, id: VariableId, length: u32) {
        self.on_gl_clip_cull_distance_sized(id, length, self.gl_clip_distance_length_var_id);
    }
    pub fn on_gl_cull_distance_sized(&mut self, id: VariableId, length: u32) {
        self.on_gl_clip_cull_distance_sized(id, length, self.gl_cull_distance_length_var_id);
    }

    // Called when the length() method is visited.
    pub fn array_length(&mut self) {
        let operand = self.interm_ids.pop().unwrap();
        let type_info = self.ir.meta.get_type(operand.type_id);

        // Support for gl_ClipDistance and gl_CullDistance, who may be sized later.
        let (is_clip_distance, is_cull_distance) = if let Id::Variable(variable_id) = operand.id {
            let built_in = self.ir.meta.get_variable(variable_id).built_in;
            (built_in == Some(BuiltIn::ClipDistance), built_in == Some(BuiltIn::CullDistance))
        } else {
            (false, false)
        };

        match type_info {
            &Type::Pointer(type_id) => {
                let array_type_info = self.ir.meta.get_type(type_id);
                if let &Type::Array(_, size) = array_type_info {
                    // The length is a constant, so push that on the stack.
                    self.push_constant_int(size as i32);
                } else if is_clip_distance || is_cull_distance {
                    // If gl_ClipDistance and gl_CullDistance are not yet sized, use a global
                    // variable for their size.  When they are sized, the global variable is
                    // initialized with the length.
                    let length_var = if is_clip_distance {
                        let new_var = self.declare_clip_cull_distance_array_length_variable(
                            self.gl_clip_distance_length_var_id,
                            "clip_distance_length",
                        );
                        self.gl_clip_distance_length_var_id = Some(new_var);
                        new_var
                    } else {
                        let new_var = self.declare_clip_cull_distance_array_length_variable(
                            self.gl_cull_distance_length_var_id,
                            "cull_distance_length",
                        );
                        self.gl_cull_distance_length_var_id = Some(new_var);
                        new_var
                    };

                    // Load from the (currently uninitialized) variable.  The newly added
                    // variable is initialized with the real length when determined.
                    self.push_variable(length_var);
                    let length = self.load();
                    self.interm_ids.push(length);
                } else {
                    // The array is unsized, so this should be a runtime instruction.
                    let result = instruction::array_length(&mut self.ir.meta, operand);
                    self.add_instruction(result);
                }
            }
            &Type::Array(_, size) => {
                self.push_constant_int(size as i32);
            }
            _ => panic!("Internal error: length() called on non-array"),
        }
    }

    // A generic helper to make a unary operator.  The operand of the unary operator is found at
    // the top of the stack.
    fn unary_instruction_from_stack<MakeInst>(&mut self, inst: MakeInst)
    where
        MakeInst: FnOnce(&mut IRMeta, TypedId) -> instruction::Result,
    {
        // Load the operand.
        let operand = self.load();

        let result = inst(&mut self.ir.meta, operand);
        self.add_instruction(result);
    }

    // A generic helper to make a binary operator.  The rhs operand of the binary operator is found
    // at the top of the stack, and the lhs operand is found under it.
    fn binary_instruction_from_stack<MakeInst>(&mut self, inst: MakeInst)
    where
        MakeInst: FnOnce(&mut IRMeta, TypedId, TypedId) -> instruction::Result,
    {
        // Load the operands.
        let rhs = self.load();
        let lhs = self.load();

        let result = inst(&mut self.ir.meta, lhs, rhs);
        self.add_instruction(result);
    }

    // Arithmetic operations.
    pub fn negate(&mut self) {
        self.unary_instruction_from_stack(instruction::negate);
    }
    pub fn add(&mut self) {
        self.binary_instruction_from_stack(instruction::add);
    }
    pub fn add_assign(&mut self) {
        self.assign_and_op(Self::add);
    }
    pub fn sub(&mut self) {
        self.binary_instruction_from_stack(instruction::sub);
    }
    pub fn sub_assign(&mut self) {
        self.assign_and_op(Self::sub);
    }
    pub fn mul(&mut self) {
        self.binary_instruction_from_stack(instruction::mul);
    }
    pub fn mul_assign(&mut self) {
        self.assign_and_op(Self::mul);
    }
    pub fn vector_times_scalar(&mut self) {
        self.binary_instruction_from_stack(instruction::vector_times_scalar);
    }
    pub fn vector_times_scalar_assign(&mut self) {
        self.assign_and_op(Self::vector_times_scalar);
    }
    pub fn matrix_times_scalar(&mut self) {
        self.binary_instruction_from_stack(instruction::matrix_times_scalar);
    }
    pub fn matrix_times_scalar_assign(&mut self) {
        self.assign_and_op(Self::matrix_times_scalar);
    }
    pub fn vector_times_matrix(&mut self) {
        self.binary_instruction_from_stack(instruction::vector_times_matrix);
    }
    pub fn vector_times_matrix_assign(&mut self) {
        self.assign_and_op(Self::vector_times_matrix);
    }
    pub fn matrix_times_vector(&mut self) {
        self.binary_instruction_from_stack(instruction::matrix_times_vector);
    }
    pub fn matrix_times_matrix(&mut self) {
        self.binary_instruction_from_stack(instruction::matrix_times_matrix);
    }
    pub fn matrix_times_matrix_assign(&mut self) {
        self.assign_and_op(Self::matrix_times_matrix);
    }
    pub fn div(&mut self) {
        self.binary_instruction_from_stack(instruction::div);
    }
    pub fn div_assign(&mut self) {
        self.assign_and_op(Self::div);
    }
    pub fn imod(&mut self) {
        self.binary_instruction_from_stack(instruction::imod);
    }
    pub fn imod_assign(&mut self) {
        self.assign_and_op(Self::imod);
    }

    // Logical oprations.
    pub fn logical_not(&mut self) {
        self.unary_instruction_from_stack(instruction::logical_not);
    }
    pub fn logical_xor(&mut self) {
        self.binary_instruction_from_stack(instruction::logical_xor);
    }

    // Comparisons.
    pub fn equal(&mut self) {
        self.binary_instruction_from_stack(instruction::equal);
    }
    pub fn not_equal(&mut self) {
        self.binary_instruction_from_stack(instruction::not_equal);
    }
    pub fn less_than(&mut self) {
        self.binary_instruction_from_stack(instruction::less_than);
    }
    pub fn greater_than(&mut self) {
        self.binary_instruction_from_stack(instruction::greater_than);
    }
    pub fn less_than_equal(&mut self) {
        self.binary_instruction_from_stack(instruction::less_than_equal);
    }
    pub fn greater_than_equal(&mut self) {
        self.binary_instruction_from_stack(instruction::greater_than_equal);
    }

    // Bit operations.
    pub fn bitwise_not(&mut self) {
        self.unary_instruction_from_stack(instruction::bitwise_not);
    }
    pub fn bit_shift_left(&mut self) {
        self.binary_instruction_from_stack(instruction::bit_shift_left);
    }
    pub fn bit_shift_left_assign(&mut self) {
        self.assign_and_op(Self::bit_shift_left);
    }
    pub fn bit_shift_right(&mut self) {
        self.binary_instruction_from_stack(instruction::bit_shift_right);
    }
    pub fn bit_shift_right_assign(&mut self) {
        self.assign_and_op(Self::bit_shift_right);
    }
    pub fn bitwise_or(&mut self) {
        self.binary_instruction_from_stack(instruction::bitwise_or);
    }
    pub fn bitwise_or_assign(&mut self) {
        self.assign_and_op(Self::bitwise_or);
    }
    pub fn bitwise_xor(&mut self) {
        self.binary_instruction_from_stack(instruction::bitwise_xor);
    }
    pub fn bitwise_xor_assign(&mut self) {
        self.assign_and_op(Self::bitwise_xor);
    }
    pub fn bitwise_and(&mut self) {
        self.binary_instruction_from_stack(instruction::bitwise_and);
    }
    pub fn bitwise_and_assign(&mut self) {
        self.assign_and_op(Self::bitwise_and);
    }
}

// TODO: FFI
