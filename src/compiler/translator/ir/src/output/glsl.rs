// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Output GLSL from the IR.
//
// TODO: An equivalent for this that builds AST should be created and end up in FFI
use crate::ir::*;
use crate::*;

#[cfg_attr(debug_assertions, derive(Debug))]
struct GlslType {
    // Text to declare the type, such as "S { ... }".  Note that Type::Struct represents both
    // structs and interface blocks, so the `struct`, `uniform`, `buffer` etc is not part of
    // the declaration text.  The type can have a pre and post text such as for
    // `type name[len]`.
    declaration_text_pre: String,
    declaration_text_post: String,
    // Text to use the type elsewhere, such as "S".
    use_text_pre: String,
    use_text_post: String,
}
#[cfg_attr(debug_assertions, derive(Debug))]
struct GlslConstant {
    text: String,
    type_id: TypeId,
}
#[cfg_attr(debug_assertions, derive(Debug))]
struct GlslVariable {
    declaration_text: String,
    name: String,
    type_id: TypeId,
}
#[cfg_attr(debug_assertions, derive(Debug))]
struct GlslFunction {
    declaration_text: String,
    name: String,
    return_type_id: TypeId,
}

pub struct Generator {
    types: HashMap<TypeId, GlslType>,
    constants: HashMap<ConstantId, GlslConstant>,
    variables: HashMap<VariableId, GlslVariable>,
    functions: HashMap<FunctionId, GlslFunction>,

    preamble: String,
    type_declarations: String,
    global_variables: String,
    function_declarations: String,

    expressions: HashMap<RegisterId, String>,
}

impl Generator {
    pub fn new() -> Generator {
        Generator {
            types: HashMap::new(),
            constants: HashMap::new(),
            variables: HashMap::new(),
            functions: HashMap::new(),
            preamble: String::new(),
            type_declarations: String::new(),
            global_variables: String::new(),
            function_declarations: String::new(),
            expressions: HashMap::new(),
        }
    }

    fn basic_type_str(id: TypeId) -> String {
        (match id {
            TYPE_ID_VOID => "void",
            TYPE_ID_FLOAT => "float",
            TYPE_ID_INT => "int",
            TYPE_ID_UINT => "uint",
            TYPE_ID_BOOL => "bool",
            TYPE_ID_ATOMIC_COUNTER => "atomic_uint",
            TYPE_ID_YUV_CSC_STANDARD => "yuvCscStandardEXT",
            TYPE_ID_VEC2 => "vec2",
            TYPE_ID_VEC3 => "vec3",
            TYPE_ID_VEC4 => "vec4",
            TYPE_ID_IVEC2 => "ivec2",
            TYPE_ID_IVEC3 => "ivec3",
            TYPE_ID_IVEC4 => "ivec4",
            TYPE_ID_UVEC2 => "uvec2",
            TYPE_ID_UVEC3 => "uvec3",
            TYPE_ID_UVEC4 => "uvec4",
            TYPE_ID_BVEC2 => "bvec2",
            TYPE_ID_BVEC3 => "bvec3",
            TYPE_ID_BVEC4 => "bvec4",
            TYPE_ID_MAT2 => "mat2",
            TYPE_ID_MAT2X3 => "mat2x3",
            TYPE_ID_MAT2X4 => "mat2x4",
            TYPE_ID_MAT3X2 => "mat3x2",
            TYPE_ID_MAT3 => "mat3",
            TYPE_ID_MAT3X4 => "mat3x4",
            TYPE_ID_MAT4X2 => "mat4x2",
            TYPE_ID_MAT4X3 => "mat4x3",
            TYPE_ID_MAT4 => "mat4",
            _ => panic!("Internal error: Invalid basic type"),
        })
        .to_string()
    }

    fn image_type_str(basic_type: ImageBasicType, image_type: ImageType) -> String {
        let prefix = match basic_type {
            ImageBasicType::Float => "",
            ImageBasicType::Int => "i",
            ImageBasicType::Uint => "u",
        };
        let mut base_name = if image_type.is_sampled { "sampler" } else { "image" };
        let suffix = match image_type.dimension {
            ImageDimension::D2 => "2D",
            ImageDimension::D3 => "3D",
            ImageDimension::Cube => "Cube",
            ImageDimension::Rect => "Rect",
            ImageDimension::Buffer => "Buffer",
            ImageDimension::External => "ExternalOES",
            ImageDimension::ExternalY2Y => "External2DY2YEXT",
            ImageDimension::Video => "VideoWEBGL",
            ImageDimension::PixelLocal => {
                base_name = "pixelLocalANGLE";
                ""
            }
            ImageDimension::Subpass => {
                base_name = "subpassInput";
                ""
            }
        };
        let multisample_suffix = if image_type.is_ms { "MS" } else { "" };
        let array_suffix = if image_type.is_array { "Array" } else { "" };
        let shadow_suffix = if image_type.is_shadow { "Shadow" } else { "" };
        format!("{prefix}{base_name}{suffix}{multisample_suffix}{array_suffix}{shadow_suffix}")
    }

    fn block_storage_str(storage: BlockStorage) -> String {
        (match storage {
            BlockStorage::Shared => "shared",
            BlockStorage::Packed => "packed",
            BlockStorage::Std140 => "std140",
            BlockStorage::Std430 => "std430",
        })
        .to_string()
    }

    fn matrix_packing_str(packing: MatrixPacking) -> String {
        (match packing {
            MatrixPacking::ColumnMajor => "column_major",
            MatrixPacking::RowMajor => "row_major",
        })
        .to_string()
    }

    fn depth_str(depth: Depth) -> String {
        (match depth {
            Depth::Any => "depth_any",
            Depth::Greater => "depth_greater",
            Depth::Less => "depth_less",
            Depth::Unchanged => "depth_unchanged",
        })
        .to_string()
    }

    fn image_internal_format_str(format: ImageInternalFormat) -> String {
        (match format {
            ImageInternalFormat::RGBA32F => "rgba32f",
            ImageInternalFormat::RGBA16F => "rgba16f",
            ImageInternalFormat::R32F => "r32f",
            ImageInternalFormat::RGBA32UI => "rgba32ui",
            ImageInternalFormat::RGBA16UI => "rgba16ui",
            ImageInternalFormat::RGBA8UI => "rgba8ui",
            ImageInternalFormat::R32UI => "r32ui",
            ImageInternalFormat::RGBA32I => "rgba32i",
            ImageInternalFormat::RGBA16I => "rgba16i",
            ImageInternalFormat::RGBA8I => "rgba8i",
            ImageInternalFormat::R32I => "r32i",
            ImageInternalFormat::RGBA8 => "rgba8",
            ImageInternalFormat::RGBA8SNORM => "rgba8snorm",
        })
        .to_string()
    }

    fn add_qualifier_str(
        decoration: Decoration,
        qualifiers: &mut Vec<String>,
        layout_qualifiers: &mut Vec<String>,
    ) {
        match decoration {
            Decoration::Invariant => qualifiers.push("invariant".to_string()),
            Decoration::Precise => qualifiers.push("precise".to_string()),
            Decoration::Interpolant => qualifiers.push("interpolant".to_string()),
            Decoration::Smooth => qualifiers.push("smooth".to_string()),
            Decoration::Flat => qualifiers.push("flat".to_string()),
            Decoration::NoPerspective => qualifiers.push("noperspective".to_string()),
            Decoration::Centroid => qualifiers.push("centroid".to_string()),
            Decoration::Sample => qualifiers.push("sample".to_string()),
            Decoration::Patch => qualifiers.push("patch".to_string()),
            Decoration::Shared => qualifiers.push("shared".to_string()),
            Decoration::ReadOnly => qualifiers.push("readonly".to_string()),
            Decoration::WriteOnly => qualifiers.push("writeonly".to_string()),
            Decoration::Coherent => qualifiers.push("coherent".to_string()),
            Decoration::Restrict => qualifiers.push("restrict".to_string()),
            Decoration::Volatile => qualifiers.push("volatile".to_string()),
            Decoration::Uniform => qualifiers.push("uniform".to_string()),
            Decoration::Buffer => qualifiers.push("buffer".to_string()),
            Decoration::NonCoherent => qualifiers.push("noncoherent".to_string()),
            Decoration::YUV => layout_qualifiers.push("yuv".to_string()),
            Decoration::Input => qualifiers.push("in".to_string()),
            Decoration::Output => qualifiers.push("out".to_string()),
            Decoration::InputOutput => qualifiers.push("inout".to_string()),
            Decoration::Location(n) => layout_qualifiers.push(format!("location={n}")),
            Decoration::Index(n) => layout_qualifiers.push(format!("index={n}")),
            Decoration::InputAttachmentIndex(n) => {
                layout_qualifiers.push(format!("input_attachment_index={n}"))
            }
            Decoration::SpecConst(n) => layout_qualifiers.push(format!("constant_id={n}")),
            Decoration::Block(storage) => layout_qualifiers.push(Self::block_storage_str(storage)),
            Decoration::Binding(n) => layout_qualifiers.push(format!("binding={n}")),
            Decoration::Offset(n) => layout_qualifiers.push(format!("offset={n}")),
            Decoration::MatrixPacking(packing) => {
                layout_qualifiers.push(Self::matrix_packing_str(packing))
            }
            Decoration::Depth(depth) => layout_qualifiers.push(Self::depth_str(depth)),
            Decoration::ImageInternalFormat(format) => {
                layout_qualifiers.push(Self::image_internal_format_str(format))
            }
            Decoration::NumViews(n) => layout_qualifiers.push(format!("num_views={n}")),
            Decoration::RasterOrdered => layout_qualifiers.push("d3d_raster_ordered".to_string()),
        }
    }

    fn qualifiers_str(precision: Precision, decorations: &Decorations) -> String {
        let mut qualifiers = vec![];
        let mut layout_qualifiers = vec![];

        decorations.decorations.iter().for_each(|&decoration| {
            Self::add_qualifier_str(decoration, &mut qualifiers, &mut layout_qualifiers);
        });

        let mut result = String::new();
        if !layout_qualifiers.is_empty() {
            write!(
                result,
                "layout({}) ",
                layout_qualifiers.into_iter().collect::<Vec<_>>().join(", ")
            )
            .unwrap();
        }
        if !qualifiers.is_empty() {
            result.push_str(&qualifiers.into_iter().collect::<Vec<_>>().join(" "));
            result.push(' ');
        }
        match precision {
            Precision::NotApplicable => (),
            Precision::Low => result.push_str("lowp "),
            Precision::Medium => result.push_str("mediump "),
            Precision::High => result.push_str("highp "),
        };
        result
    }

    fn built_in_str(built_in: BuiltIn) -> String {
        (match built_in {
            BuiltIn::InstaceID => "gl_InstaceID",
            BuiltIn::VertexID => "gl_VertexID",
            BuiltIn::Position => "gl_Position",
            BuiltIn::PointSize => "gl_PointSize",
            BuiltIn::DrawID => "gl_DrawID",
            BuiltIn::FragCoord => "gl_FragCoord",
            BuiltIn::FrontFacing => "gl_FrontFacing",
            BuiltIn::PointCoored => "gl_PointCoored",
            BuiltIn::HelperInvocation => "gl_HelperInvocation",
            BuiltIn::FragColor => "gl_FragColor",
            BuiltIn::FragData => "gl_FragData",
            BuiltIn::FragDepth => "gl_FragDepth",
            BuiltIn::SecondaryFragColorEXT => "gl_SecondaryFragColorEXT",
            BuiltIn::SecondaryFragDataEXT => "gl_SecondaryFragDataEXT",
            BuiltIn::ViewIDOVR => "gl_ViewIDOVR",
            BuiltIn::ViewportIndex => "gl_ViewportIndex",
            BuiltIn::ClipDistance => "gl_ClipDistance",
            BuiltIn::CullDistance => "gl_CullDistance",
            BuiltIn::LastFragColor => "gl_LastFragColor",
            BuiltIn::LastFragData => "gl_LastFragData",
            BuiltIn::SampleID => "gl_SampleID",
            BuiltIn::SamplePosition => "gl_SamplePosition",
            BuiltIn::SampleMaskIn => "gl_SampleMaskIn",
            BuiltIn::SampleMask => "gl_SampleMask",
            BuiltIn::NumSamples => "gl_NumSamples",
            BuiltIn::NumWorkGroups => "gl_NumWorkGroups",
            BuiltIn::WorkGroupSize => "gl_WorkGroupSize",
            BuiltIn::WorkGroupID => "gl_WorkGroupID",
            BuiltIn::LocalInvaocationID => "gl_LocalInvaocationID",
            BuiltIn::GlobalInvocationID => "gl_GlobalInvocationID",
            BuiltIn::LocalInvocationIndex => "gl_LocalInvocationIndex",
            BuiltIn::PerVertexIn => "gl_PerVertexIn",
            BuiltIn::PerVertexOut => "gl_PerVertexOut",
            BuiltIn::PrimitiveIDIn => "gl_PrimitiveIDIn",
            BuiltIn::InvocationID => "gl_InvocationID",
            BuiltIn::PrimitiveID => "gl_PrimitiveID",
            BuiltIn::LayerOut => "gl_Layer",
            BuiltIn::LayerIn => "gl_Layer",
            BuiltIn::BoundingBoxOES => "gl_BoundingBoxOES",
            BuiltIn::PixelLocalEXT => "gl_PixelLocalEXT",
        })
        .to_string()
    }

    fn name_str(name: &Name, temp_prefix: &'static str, id: u32) -> String {
        format!(
            "{}{}{}",
            match name.source {
                // Make sure unnamed interface blocks remain unnamed.
                NameSource::ShaderInterface =>
                    if name.name != "" {
                        USER_SYMBOL_PREFIX
                    } else {
                        ""
                    },
                NameSource::Temporary => temp_prefix,
                _ => "",
            },
            name.name,
            if name.source == NameSource::Temporary { format!("_{id}") } else { "".to_string() }
        )
    }

    fn variable_declaration_str(
        &self,
        name: &Name,
        type_id: TypeId,
        precision: Precision,
        decorations: &Decorations,
        built_in: Option<BuiltIn>,
        initializer: Option<ConstantId>,
        temp_prefix: &'static str,
        id: u32,
    ) -> (String, String) {
        let type_info = &self.types[&type_id];
        let qualifiers = Self::qualifiers_str(precision, decorations);
        let var_name = if let Some(built_in) = built_in {
            Self::built_in_str(built_in)
        } else {
            Self::name_str(name, temp_prefix, id)
        };

        let mut declaration_text = format!(
            "{qualifiers}{} {var_name}{}",
            &type_info.use_text_pre, &type_info.use_text_post
        );

        if let Some(constant_id) = initializer {
            write!(declaration_text, " = {}", self.get_constant_expression(constant_id)).unwrap();
        }

        (declaration_text, var_name)
    }

    fn yuv_csc_standard_str(yuv_csc: YuvCscStandard) -> String {
        (match yuv_csc {
            YuvCscStandard::Itu601 => "itu_601",
            YuvCscStandard::Itu601FullRange => "itu_601_full_range",
            YuvCscStandard::Itu709 => "itu_709",
        })
        .to_string()
    }

    fn function_param_direction_str(direction: FunctionParamDirection) -> String {
        (match direction {
            FunctionParamDirection::Input => "in",
            FunctionParamDirection::Output => "out",
            FunctionParamDirection::InputOutput => "inout",
        })
        .to_string()
    }

    fn blend_equation_advanced_str(equations: &AdvancedBlendEquations) -> String {
        if equations.all() {
            return "layout(blend_support_all_equations) out;\n".to_string();
        }

        let mut equations_list = Vec::new();
        if equations.multiply {
            equations_list.push("blend_support_multiply");
        }
        if equations.screen {
            equations_list.push("blend_support_screen");
        }
        if equations.overlay {
            equations_list.push("blend_support_overlay");
        }
        if equations.darken {
            equations_list.push("blend_support_darken");
        }
        if equations.lighten {
            equations_list.push("blend_support_lighten");
        }
        if equations.colordodge {
            equations_list.push("blend_support_colordodge");
        }
        if equations.colorburn {
            equations_list.push("blend_support_colorburn");
        }
        if equations.hardlight {
            equations_list.push("blend_support_hardlight");
        }
        if equations.softlight {
            equations_list.push("blend_support_softlight");
        }
        if equations.difference {
            equations_list.push("blend_support_difference");
        }
        if equations.exclusion {
            equations_list.push("blend_support_exclusion");
        }
        if equations.hsl_hue {
            equations_list.push("blend_support_hsl_hue");
        }
        if equations.hsl_saturation {
            equations_list.push("blend_support_hsl_saturation");
        }
        if equations.hsl_color {
            equations_list.push("blend_support_hsl_color");
        }
        if equations.hsl_luminosity {
            equations_list.push("blend_support_hsl_luminosity");
        }

        if equations_list.is_empty() {
            return String::new();
        }

        format!("layout({}) out;\n", equations_list.join(", "))
    }

    fn tes_qualifiers_str(
        primitive: TessellationPrimitive,
        vertex_spacing: TessellationSpacing,
        ordering: TessellationOrdering,
        point_mode: bool,
    ) -> String {
        let mut qualifiers = Vec::new();

        match primitive {
            TessellationPrimitive::Undefined => (),
            TessellationPrimitive::Triangles => qualifiers.push("triangles"),
            TessellationPrimitive::Quads => qualifiers.push("quads"),
            TessellationPrimitive::Isolines => qualifiers.push("isolines"),
        }
        match vertex_spacing {
            TessellationSpacing::Undefined => (),
            TessellationSpacing::EqualSpacing => qualifiers.push("equal"),
            TessellationSpacing::FractionalEvenSpacing => {
                qualifiers.push("fractional_even_spacing")
            }
            TessellationSpacing::FractionalOddSpacing => qualifiers.push("fractional_odd_spacing"),
        }
        match ordering {
            TessellationOrdering::Undefined => (),
            TessellationOrdering::Cw => qualifiers.push("cw"),
            TessellationOrdering::Ccw => qualifiers.push("ccw"),
        }
        if point_mode {
            qualifiers.push("point_mode");
        }

        format!("layout({}) in;\n", qualifiers.join(", "))
    }

    fn add_gs_primitive_str(qualifiers: &mut Vec<String>, primitive: GeometryPrimitive) {
        match primitive {
            GeometryPrimitive::Undefined => (),
            GeometryPrimitive::Points => qualifiers.push("points".to_string()),
            GeometryPrimitive::Lines => qualifiers.push("lines".to_string()),
            GeometryPrimitive::LinesAdjacency => qualifiers.push("lines_adjacency".to_string()),
            GeometryPrimitive::Triangles => qualifiers.push("triangles".to_string()),
            GeometryPrimitive::TrianglesAdjacency => {
                qualifiers.push("triangles_adjacency".to_string())
            }
            GeometryPrimitive::LineStrip => qualifiers.push("line_strip".to_string()),
            GeometryPrimitive::TriangleStrip => qualifiers.push("triangle_strip".to_string()),
        }
    }

    fn gs_qualifiers_in_str(primitive: GeometryPrimitive, invocations: u32) -> String {
        let mut qualifiers = Vec::new();
        Self::add_gs_primitive_str(&mut qualifiers, primitive);
        if invocations > 0 {
            qualifiers.push(format!("invocations = {invocations}"));
        }

        format!("layout({}) in;\n", qualifiers.join(", "))
    }

    fn gs_qualifiers_out_str(primitive: GeometryPrimitive, max_vertices: u32) -> String {
        let mut qualifiers = Vec::new();
        Self::add_gs_primitive_str(&mut qualifiers, primitive);
        qualifiers.push(format!("max_vertices = {max_vertices}"));

        format!("layout({}) out;\n", qualifiers.join(", "))
    }

    fn declare_variables(
        glsl_variables: &HashMap<VariableId, GlslVariable>,
        block: &mut String,
        variables: &Vec<VariableId>,
    ) {
        variables.iter().for_each(|id| {
            write!(block, "{};\n", glsl_variables[id].declaration_text).unwrap();
        });
    }

    fn get_swizzle(index: u32) -> &'static str {
        match index {
            0 => "x",
            1 => "y",
            2 => "z",
            3 => "w",
            _ => panic!("Internal error: Unexpected swizzle index"),
        }
    }

    fn get_swizzle_multi(indices: &Vec<u32>) -> String {
        indices.iter().map(|&index| Self::get_swizzle(index)).collect::<Vec<_>>().join("")
    }

    fn get_constant_expression(&self, id: ConstantId) -> &String {
        &self.constants[&id].text
    }

    fn get_expression(&self, id: TypedId) -> &String {
        match &id.id {
            Id::Register(id) => &self.expressions[id],
            Id::Constant(id) => &self.constants[id].text,
            Id::Variable(id) => &self.variables[id].name,
        }
    }

    fn indent_block(block: String, times: usize) -> String {
        debug_assert!(block.chars().last().unwrap() == '\n');
        // Only 1 and 2 are given, avoid excessive string overhead and handle the two cases.
        let indent = if times == 1 { "  " } else { "    " };
        let replace_with = if times == 1 { "\n  " } else { "\n    " };
        let mut result = block.replace("\n", replace_with);
        // The result is missing indentation from the beginning, and has two extra spaces in
        // the end.
        result.truncate(result.len() - indent.len());
        result.insert_str(0, indent);
        result
    }
}

impl ast::Target for Generator {
    type BlockResult = String;

    fn begin(&mut self) {}
    fn end(&mut self) {
        // TODO: add #version and #extension lines when ANGLE uses the IR all the way through to
        // codegen. TODO: return the resulting string instead
        println!("{}", self.function_declarations);
    }

    fn new_type(&mut self, id: TypeId, type_info: &Type) {
        let (declaration_text_pre, declaration_text_post, use_text_pre, use_text_post) =
            match type_info {
                Type::Scalar(..) | Type::Vector(..) | Type::Matrix(..) => {
                    (Self::basic_type_str(id), "".to_string(), None, None)
                }
                &Type::Array(type_id, count) => {
                    let base_type = &self.types[&type_id];
                    (
                        base_type.declaration_text_pre.clone(),
                        format!("{}[{count}]", &base_type.declaration_text_post),
                        Some(base_type.use_text_pre.clone()),
                        Some(format!("{}[{count}]", &base_type.use_text_post)),
                    )
                }
                &Type::UnsizedArray(type_id) => {
                    let base_type = &self.types[&type_id];
                    (
                        base_type.declaration_text_pre.clone(),
                        format!("{}[]", &base_type.declaration_text_post),
                        Some(base_type.use_text_pre.clone()),
                        Some(format!("{}[]", &base_type.use_text_post)),
                    )
                }
                &Type::Image(basic_type, image_type) => {
                    (Self::image_type_str(basic_type, image_type), "".to_string(), None, None)
                }
                Type::Struct(name, fields, specialization) => {
                    let name = Self::name_str(name, TEMP_IMPOSSIBLE_PREFIX, 0);
                    let declaration_text = format!(
                        "{} {{\n{}}}",
                        &name,
                        fields
                            .iter()
                            .map(|field| format!(
                                "  {};\n",
                                self.variable_declaration_str(
                                    // TODO: gl_PerVertex fields may have built_in decoration, this
                                    // is not represented in the IR currently.
                                    // TODO: maybe we can get rid of gl_PerVertex in the IR
                                    // and build it if necessary during GLSL generation?
                                    &field.name,
                                    field.type_id,
                                    field.precision,
                                    &field.decorations,
                                    None,
                                    None,
                                    TEMP_IMPOSSIBLE_PREFIX,
                                    0
                                )
                                .0
                            ))
                            .collect::<Vec<_>>()
                            .join("")
                    );

                    // Declare the struct for future use.
                    if *specialization == StructSpecialization::Struct {
                        write!(self.type_declarations, "struct {};\n", &declaration_text).unwrap();
                    }

                    (
                        declaration_text,
                        "".to_string(),
                        if *specialization == StructSpecialization::InterfaceBlock {
                            None
                        } else {
                            Some(name)
                        },
                        None,
                    )
                }
                Type::Pointer(pointee_type_id) => {
                    let pointee_type = &self.types[pointee_type_id];
                    (
                        pointee_type.declaration_text_pre.clone(),
                        pointee_type.declaration_text_post.clone(),
                        Some(pointee_type.use_text_pre.clone()),
                        Some(pointee_type.use_text_post.clone()),
                    )
                }
            };

        let glsl_type = GlslType {
            use_text_pre: use_text_pre.unwrap_or(declaration_text_pre.clone()),
            use_text_post: use_text_post.unwrap_or(declaration_text_post.clone()),
            declaration_text_pre,
            declaration_text_post,
        };

        self.types.insert(id, glsl_type);
    }

    fn new_constant(&mut self, id: ConstantId, constant: &Constant) {
        let type_info = &self.types[&constant.type_id];
        let constant_text = match &constant.value {
            &ConstantValue::Float(f) => format!("{f:?}"),
            &ConstantValue::Int(i) => i.to_string(),
            &ConstantValue::Uint(u) => format!("{u}u"),
            &ConstantValue::Bool(b) => b.to_string(),
            &ConstantValue::YuvCsc(yuv_csc) => Self::yuv_csc_standard_str(yuv_csc),
            ConstantValue::Composite(elements) => format!(
                "{}{}({})",
                &type_info.use_text_pre,
                &type_info.use_text_post,
                elements
                    .iter()
                    .map(|element| self.constants[element].text.clone())
                    .collect::<Vec<_>>()
                    .join(", ")
            ),
        };

        let glsl_constant = GlslConstant { text: constant_text, type_id: constant.type_id };

        self.constants.insert(id, glsl_constant);
    }

    fn new_variable(&mut self, id: VariableId, variable: &Variable) {
        let (declaration_text, name) = self.variable_declaration_str(
            &variable.name,
            variable.type_id,
            variable.precision,
            &variable.decorations,
            variable.built_in,
            variable.initializer,
            TEMP_VARIABLE_PREFIX,
            id.id,
        );

        let glsl_variable = GlslVariable { declaration_text, name, type_id: variable.type_id };

        self.variables.insert(id, glsl_variable);
    }

    fn new_function(&mut self, id: FunctionId, function: &Function) {
        let qualifiers = Self::qualifiers_str(function.return_precision, &Decorations::new_none());
        let return_type = &self.types[&function.return_type_id];
        let name = Self::name_str(&function.name, TEMP_FUNCTION_PREFIX, id.id);

        let declaration_text = format!(
            "{qualifiers}{}{} {}({})",
            &return_type.use_text_pre,
            &return_type.use_text_post,
            &name,
            function
                .params
                .iter()
                .map(|param| {
                    format!(
                        "{} {}",
                        Self::function_param_direction_str(param.direction),
                        &self.variables[&param.variable_id].declaration_text
                    )
                })
                .collect::<Vec<_>>()
                .join(", ")
        );

        let glsl_function =
            GlslFunction { declaration_text, name, return_type_id: function.return_type_id };

        self.functions.insert(id, glsl_function);
    }

    fn global_scope(&mut self, ir_meta: &IRMeta) {
        match ir_meta.get_shader_type() {
            ShaderType::Fragment => {
                if ir_meta.get_early_fragment_tests() {
                    self.preamble.push_str("layout(early_fragment_tests) in;\n");
                }
                self.preamble +=
                    &Self::blend_equation_advanced_str(ir_meta.get_advanced_blend_equations());
            }
            ShaderType::TessellationControl => {
                write!(self.preamble, "layout(vertices = {}) out;\n", ir_meta.get_tcs_vertices())
                    .unwrap();
            }
            ShaderType::TessellationEvaluation => {
                self.preamble += &Self::tes_qualifiers_str(
                    ir_meta.get_tes_primitive(),
                    ir_meta.get_tes_vertex_spacing(),
                    ir_meta.get_tes_ordering(),
                    ir_meta.get_tes_point_mode(),
                );
            }
            ShaderType::Geometry => {
                self.preamble += &Self::gs_qualifiers_in_str(
                    ir_meta.get_gs_primitive_in(),
                    ir_meta.get_gs_invocations(),
                );
                self.preamble += &Self::gs_qualifiers_out_str(
                    ir_meta.get_gs_primitive_out(),
                    ir_meta.get_gs_max_vertices(),
                );
            }
            _ => (),
        };

        Self::declare_variables(
            &self.variables,
            &mut self.global_variables,
            ir_meta.all_global_variables(),
        );

        println!("{}", self.preamble);
        println!("{}", self.type_declarations);
        println!("{}", self.global_variables);
    }

    fn begin_block(&mut self, variables: &Vec<VariableId>) -> String {
        let mut block = String::new();
        Self::declare_variables(&self.variables, &mut block, variables);
        block
    }

    fn merge_blocks(&mut self, blocks: Vec<String>) -> String {
        blocks.join("")
    }

    fn swizzle_single(
        &mut self,
        _block_result: &mut String,
        result: RegisterId,
        id: TypedId,
        index: u32,
    ) {
        let expr = format!("{}.{}", self.get_expression(id), Self::get_swizzle(index));
        self.expressions.insert(result, expr);
    }

    fn swizzle_multi(
        &mut self,
        _block_result: &mut String,
        result: RegisterId,
        id: TypedId,
        indices: &Vec<u32>,
    ) {
        let expr = format!("{}.{}", self.get_expression(id), Self::get_swizzle_multi(indices));
        self.expressions.insert(result, expr);
    }

    fn index(
        &mut self,
        _block_result: &mut String,
        result: RegisterId,
        id: TypedId,
        index: TypedId,
    ) {
        let expr = format!("{}[{}]", self.get_expression(id), self.get_expression(index));
        self.expressions.insert(result, expr);
    }

    fn select_field(
        &mut self,
        _block_result: &mut String,
        result: RegisterId,
        id: TypedId,
        _index: u32,
        field: &Field,
    ) {
        let lhs = self.get_expression(id);
        let field_name = Self::name_str(&field.name, TEMP_IMPOSSIBLE_PREFIX, 0);
        // Note: if selecting the field of a nameless interface block, just use the field.
        let expr = if lhs == "" { field_name } else { format!("{}.{}", lhs, field_name) };
        self.expressions.insert(result, expr);
    }

    fn construct_single(
        &mut self,
        _block_result: &mut String,
        result: RegisterId,
        type_id: TypeId,
        id: TypedId,
    ) {
        let type_info = &self.types[&type_id];
        let expr = format!(
            "{}{}({})",
            type_info.use_text_pre,
            type_info.use_text_post,
            self.get_expression(id)
        );
        self.expressions.insert(result, expr);
    }

    fn construct_multi(
        &mut self,
        _block_result: &mut String,
        result: RegisterId,
        type_id: TypeId,
        ids: &Vec<TypedId>,
    ) {
        let type_info = &self.types[&type_id];
        let expr = format!(
            "{}{}({})",
            type_info.use_text_pre,
            type_info.use_text_post,
            ids.iter().map(|&id| self.get_expression(id).clone()).collect::<Vec<_>>().join(", ")
        );
        self.expressions.insert(result, expr);
    }

    fn load(&mut self, _block_result: &mut String, result: RegisterId, pointer: TypedId) {
        let expr = self.get_expression(pointer).clone();
        self.expressions.insert(result, expr);
    }

    fn store(&mut self, block_result: &mut String, pointer: TypedId, value: TypedId) {
        let statement =
            format!("{} = ({});\n", self.get_expression(pointer), self.get_expression(value));
        block_result.push_str(&statement);
    }

    fn call(
        &mut self,
        block_result: &mut String,
        result: Option<RegisterId>,
        function_id: FunctionId,
        params: &Vec<TypedId>,
    ) {
        let statement = format!(
            "{}({})",
            self.functions[&function_id].name,
            params.iter().map(|&id| self.get_expression(id).clone()).collect::<Vec<_>>().join(", ")
        );
        match result {
            Some(result) => {
                self.expressions.insert(result, statement);
            }
            None => {
                write!(block_result, "{statement};\n").unwrap();
            }
        };
    }

    fn unary(
        &mut self,
        _block_result: &mut String,
        result: RegisterId,
        unary_op: UnaryOpCode,
        id: TypedId,
    ) {
        let id = self.get_expression(id);
        let expr = match unary_op {
            UnaryOpCode::ArrayLength => format!("({id}).length()"),
            UnaryOpCode::Negate => format!("-({id})"),
            UnaryOpCode::LogicalNot => format!("!({id})"),
            UnaryOpCode::BitwiseNot => format!("~({id})"),
        };
        self.expressions.insert(result, expr);
    }

    fn binary(
        &mut self,
        _block_result: &mut String,
        result: RegisterId,
        binary_op: BinaryOpCode,
        lhs: TypedId,
        rhs: TypedId,
    ) {
        let lhs = self.get_expression(lhs);
        let rhs = self.get_expression(rhs);
        let op = match binary_op {
            BinaryOpCode::Add => "+",
            BinaryOpCode::Sub => "-",
            BinaryOpCode::Mul => "*",
            BinaryOpCode::VectorTimesScalar => "*",
            BinaryOpCode::MatrixTimesScalar => "*",
            BinaryOpCode::VectorTimesMatrix => "*",
            BinaryOpCode::MatrixTimesVector => "*",
            BinaryOpCode::MatrixTimesMatrix => "*",
            BinaryOpCode::Div => "/",
            BinaryOpCode::IMod => "%",
            BinaryOpCode::LogicalXor => "^^",
            BinaryOpCode::Equal => "==",
            BinaryOpCode::NotEqual => "!=",
            BinaryOpCode::LessThan => "<",
            BinaryOpCode::GreaterThan => ">",
            BinaryOpCode::LessThanEqual => "<=",
            BinaryOpCode::GreaterThanEqual => ">=",
            BinaryOpCode::BitShiftLeft => "<<",
            BinaryOpCode::BitShiftRight => ">>",
            BinaryOpCode::BitwiseOr => "|",
            BinaryOpCode::BitwiseXor => "^",
            BinaryOpCode::BitwiseAnd => "&",
        };
        let expr = format!("({lhs}) {op} ({rhs})");
        self.expressions.insert(result, expr);
    }

    fn branch_discard(&mut self, block: &mut String) {
        block.push_str("discard;\n");
    }
    fn branch_return(&mut self, block: &mut String, value: Option<TypedId>) {
        if let Some(id) = value {
            write!(block, "return {};\n", self.get_expression(id)).unwrap();
        } else {
            block.push_str("return;\n");
        }
    }
    fn branch_break(&mut self, block: &mut String) {
        block.push_str("break;\n");
    }
    fn branch_continue(&mut self, block: &mut String) {
        block.push_str("continue;\n");
    }
    fn branch_if(
        &mut self,
        block: &mut String,
        condition: TypedId,
        true_block: Option<String>,
        false_block: Option<String>,
    ) {
        // The true block should always be present.
        write!(
            block,
            "if ({}) {{\n{}}}\n",
            self.get_expression(condition),
            Self::indent_block(true_block.unwrap(), 1)
        )
        .unwrap();
        if let Some(false_block) = false_block {
            write!(block, "else {{\n{}}}\n", Self::indent_block(false_block, 1)).unwrap();
        }
    }
    fn branch_loop(
        &mut self,
        block: &mut String,
        loop_condition_block: Option<String>,
        body_block: Option<String>,
    ) {
        // The condition and body blocks should always be present.
        write!(
            block,
            "for (;;) {{\n{}\n{}}}\n",
            Self::indent_block(loop_condition_block.unwrap(), 1),
            Self::indent_block(body_block.unwrap(), 1)
        )
        .unwrap();
    }
    fn branch_do_loop(&mut self, block: &mut String, body_block: Option<String>) {
        // The condition and body blocks should always be present.  The difference between
        // DoLoop and Loop is effectively that the condition block is evaluated after the body
        // instead of before.
        write!(block, "for (;;) {{\n{}}}\n", Self::indent_block(body_block.unwrap(), 1)).unwrap();
    }
    fn branch_loop_if(&mut self, block: &mut String, condition: TypedId) {
        // The condition block of a loop ends in `if (!condition) break;`
        write!(block, "if (!({}))\n  break;\n", self.get_expression(condition)).unwrap();
    }
    fn branch_switch(
        &mut self,
        block: &mut String,
        value: TypedId,
        case_ids: &Vec<Option<ConstantId>>,
        case_blocks: Vec<String>,
    ) {
        write!(block, "switch ({}) {{\n", self.get_expression(value)).unwrap();
        case_blocks.into_iter().zip(case_ids).for_each(|(case, &case_id)| {
            let case_line = if let Some(case_id) = case_id {
                format!("  case {}:\n", self.get_constant_expression(case_id))
            } else {
                "  default:\n".to_string()
            };

            let case_body = if case.is_empty() {
                "".to_string()
            } else {
                format!("  {{\n{}  }}\n", Self::indent_block(case, 2))
            };

            block.push_str(&case_line);
            block.push_str(&case_body);
        });
        block.push_str("}\n");
    }

    // Take the current AST and place it as the body of the given function.
    fn end_function(&mut self, block_result: String, id: FunctionId) {
        write!(
            self.function_declarations,
            "{} {{\n{}}}\n",
            self.functions[&id].declaration_text,
            Self::indent_block(block_result, 1),
        )
        .unwrap();
    }
}
