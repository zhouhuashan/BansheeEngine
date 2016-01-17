#pragma once

#include "BsSLPrerequisites.h"
#include "BsShader.h"
#include "BsGpuProgram.h"
#include "BsRasterizerState.h"
#include "BsDepthStencilState.h"
#include "BsBlendState.h"

extern "C" {
#include "BsASTFX.h"
}

namespace BansheeEngine
{
	/**	Contains the results of compilation returned from the BSLFXCompiler. */
	struct BSLFXCompileResult
	{
		ShaderPtr shader; /**< Resulting shader if compilation was successful. Null if error occurred. */
		String errorMessage; /**< Error message if compilation failed. */
		int errorLine = 0; /**< Line of the error if one occurred. */
		int errorColumn = 0; /**< Column of the error if one occurred. */
	};

	/**	Transforms a source file written in BSL FX syntax into a Shader object. */
	class BSLFXCompiler
	{
		/**	Possible types of code blocks within a shader. */
		enum class CodeBlockType
		{
			Vertex, Fragment, Geometry, Hull, Domain, Compute, Common
		};

		/**
		 * Represents a block of code written in a GPU program language for a specific GPU program type. (i.e. non-FX code)
		 */
		struct CodeBlock
		{
			CodeBlockType type;
			String code;
		};

		/**	Temporary data describing a pass during parsing. */
		struct PassData
		{
			BLEND_STATE_DESC blendDesc;
			RASTERIZER_STATE_DESC rasterizerDesc;
			DEPTH_STENCIL_STATE_DESC depthStencilDesc;

			bool blendIsDefault = true;
			bool rasterizerIsDefault = true;
			bool depthStencilIsDefault = true;

			String commonCode;
			String vertexCode;
			String fragmentCode;
			String geometryCode;
			String hullCode;
			String domainCode;
			String computeCode;
		};

	public:
		/**	Transforms a source file written in BSL FX syntax into a Shader object. */
		static BSLFXCompileResult compile(const String& source);

	private:
		/** Converts the provided source into an abstract syntax tree using the lexer & parser for BSL FX syntax. */
		static void parseFX(ParseState* parseState, const char* source);

		/**
		 * Retrieves non-FX code blocks (i.e. actual shader code) from the source code and removes them from the input so 
		 * all that remains is a pure FX code. Found blocks are returned so they may be compiled using their appropriate 
		 * compiler.
		 */
		static Vector<CodeBlock> parseCodeBlocks(String& source);

		/**
		 * Merges two shader AST nodes. The first node will contain the combination of both after the method executes.
		 *
		 * @param[in, out]	mergeInto		Parse state containing the node to be merged into.
		 * @param[in]		mergeFrom		Second of the nodes to be merged. All the contents of this node will be merged
		 *									into the first node. This node and its children will remain unmodified.
		 * @param[in]		codeBlockOffset	Offset to apply to any code block indexes belonging to the second node.
		 */
		static void mergeAST(ParseState* mergeInto, ASTFXNode* mergeFrom, UINT32 codeBlockOffset);

		/**
		 * Finds the blocks node in the root node of the provided parse state, and merges any entries from @p blocksNode
		 * into it. A new node is created if one doesn't exist.
		 */
		static void mergeBlocks(ParseState* mergeInto, ASTFXNode* blocksNode);

		/**
		 * Finds the parameters node in the root node of the provided parse state, and merges any entries from 
		 * @p paramsNode into it. A new node is created if one doesn't exist.
		 */
		static void mergeParameters(ParseState* mergeInto, ASTFXNode* paramsNode);

		/**
		 * Retrieves the renderer and language specified for the technique. These two values are considered a unique 
		 * identifier for a technique.
		 */
		static void getTechniqueIdentifier(ASTFXNode* technique, StringID& renderer, String& language);

		/** Checks if two techniques can be matched based on the options specified in their child nodes. */
		static bool isTechniqueMergeValid(ASTFXNode* into, ASTFXNode* from);

		/**
		 * Copies an existing AST node option and inserts it into another node options list.
		 *
		 * @param[in, out]	into	Parse state of the into which the node option will be inserted to.
		 * @param[in]		parent	A set of node options to insert the node option copy into.
		 * @param[in]		option	Node option to copy.
		 * @return					True if the copied node was a complex type.
		 */
		static bool copyNodeOption(ParseState* into, NodeOptions* parent, NodeOption* option);

		/**
		 * Merges pass states and code blocks. All code blocks from @p mergeFromNode will have their indexes incremented by
		 * @p codeBlockOffset.
		 */
		static void mergePass(ParseState* mergeInto, ASTFXNode* mergeIntoNode, ASTFXNode* mergeFromNode, 
			UINT32 codeBlockOffset);

		/**
		 * Merges code blocks. All code blocks from @p mergeFromNode will have their indexes incremented by 
		 * @p codeBlockOffset.
		 */
		static void mergeCode(ParseState* mergeInto, ASTFXNode* mergeIntoNode, ASTFXNode* mergeFromNode, 
			UINT32 codeBlockOffset);

		/**
		 * Merges all pass states by copying all child nodes and their options to the destination node. 
		 *			
		 * @note	
		 * Certain node types are ignored as we handle their merging specially. Should only be called on Technique nodes or
		 * its children.
		 */
		static void mergePassStates(ParseState* mergeInto, ASTFXNode* mergeIntoNode, ASTFXNode* mergeFromNode);

		/**
		 * Merges two techniques. All technique states, code blocks and passes will be merged. Passes will be merged 
		 * according to the pass index (new passes will be inserted if the destination doesn't already have a pass with an
		 * index belonging to the source pass). All code blocks from @p mergeFromNode will have their indexes incremented
		 * by @p codeBlockOffset.
		 */
		static void mergeTechnique(ParseState* mergeInto, ASTFXNode* mergeIntoNode, ASTFXNode* mergeFromNode, 
			UINT32 codeBlockOffset);

		/**
		 * Find matching techniques from the root shader node in @p mergeInto and merges them with @p techniqueNode.
		 * All code blocks from @p techniqueNode will have their indexes incremented by @p codeBlockOffset.
		 *
		 * @see		BSLFXCompiler::mergeTechnique
		 */
		static void mergeTechniques(ParseState* mergeInto, ASTFXNode* techniqueNode, UINT32 codeBlockOffset);

		/**	Converts FX renderer name into an in-engine renderer identifier. */
		static StringID parseRenderer(const String& name);

		/**
		 * Converts FX language into an in-engine shader language (e.g. hlsl, glsl) and a rendering API that supports the
		 * provided language.
		 */
		static void parseLanguage(const String& name, StringID& renderAPI, String& language);

		/**	Maps FX buffer usage enum into in-engine param block usage. */
		static GpuParamBlockUsage parseBlockUsage(BufferUsageValue usage);

		/**	Maps FX filter mode enum into in-engine filter mode. */
		static UINT32 parseFilterMode(FilterValue filter);

		/**	Maps FX comparison function enum into in-engine compare function. */
		static CompareFunction parseCompFunc(CompFuncValue compFunc);

		/**	Maps FX addressing mode enum into in-engine addressing mode. */
		static TextureAddressingMode parseAddrMode(AddrModeValue addrMode);

		/**	Maps FX operation to in-engine blend factor. */
		static BlendFactor parseBlendFactor(OpValue factor);

		/**	Maps FX blend operation to in-engine blend operation. */
		static BlendOperation parseBlendOp(BlendOpValue op);

		/**
		 * Maps FX parameter type to in-engine shader parameter.
		 *
		 * @param[in]	type		Input FX parameter type.
		 * @param[in]	isObjType	Output parameter signaling whether the in-engine parameter is a data or an object type.
		 * @param[in]	typeId		Type ID corresponding to a value of in-game GpuParamDataType or GpuParamObjectType
		 *							enum (depending on isObjType()).
		 */
		static void parseParamType(ParamType type, bool& isObjType, UINT32& typeId);

		/**	Maps FX operation to in-engine stencil operation. */
		static StencilOperation parseStencilOp(OpValue op);
		
		/**	Maps FX cull mode enum to in-engine cull mode. */
		static CullingMode parseCullMode(CullModeValue cm);

		/**	Maps FX fill mode enum to in-engine fill mode. */
		static PolygonMode parseFillMode(FillModeValue fm);

		/**
		 * Populates the front facing operation portion of the depth-stencil state descriptor from the provided stencil-op
		 * AST node.
		 */
		static void parseStencilFront(DEPTH_STENCIL_STATE_DESC& desc, ASTFXNode* stencilOpNode);

		/**
		 * Populates the back backing operation portion of the depth-stencil state descriptor from the provided stencil-op
		 * AST node.
		 */
		static void parseStencilBack(DEPTH_STENCIL_STATE_DESC& desc, ASTFXNode* stencilOpNode);

		/**
		 * Populates the addressing mode portion of the sampler state descriptor for U/V/W axes from the provided addressing
		 * mode AST node.
		 */
		static void parseAddrMode(SAMPLER_STATE_DESC& desc, ASTFXNode* addrModeNode);

		/** Populates the color (RGB) portion of the blend state descriptor from the provided blend definition AST node. */
		static void parseColorBlendDef(RENDER_TARGET_BLEND_STATE_DESC& desc, ASTFXNode* blendDefNode);

		/** Populates the alpha portion of the blend state descriptor from the provided blend definition AST node. */
		static void parseAlphaBlendDef(RENDER_TARGET_BLEND_STATE_DESC& desc, ASTFXNode* blendDefNode);

		/**
		 * Populates blend state descriptor for a single render target from the provided AST node. Which target gets 
		 * updated depends on the index set in the AST node.
		 */
		static void parseRenderTargetBlendState(BLEND_STATE_DESC& desc, ASTFXNode* targetNode);

		/**
		 * Parses the blend state AST node and outputs a blend state descriptor. Returns false if the descriptor wasn't 
		 * modified.
		 */
		static bool parseBlendState(BLEND_STATE_DESC& desc, ASTFXNode* passNode);

		/**
		 * Parses the rasterizer state AST node and outputs a rasterizer state descriptor. Returns false if the descriptor
		 * wasn't modified.
		 */
		static bool parseRasterizerState(RASTERIZER_STATE_DESC& desc, ASTFXNode* passNode);

		/**
		 * Parses the depth-stencil state AST node and outputs a depth-stencil state descriptor. Returns false if the 
		 * descriptor wasn't modified.
		 */
		static bool parseDepthStencilState(DEPTH_STENCIL_STATE_DESC& desc, ASTFXNode* passNode);

		/** Parses the sampler state AST node and outputs a sampler state object, or a nullptr in case AST node is empty. */
		static SamplerStatePtr parseSamplerState(ASTFXNode* samplerStateNode);

		/**
		 * Parses a code AST node and outputs the result in one of the streams within the provided pass data.
		 *
		 * @param[in]	codeNode	AST node to parse
		 * @param[in]	codeBlocks	GPU program source code retrieved from parseCodeBlocks().
		 * @param[in]	passData	Pass data containing temporary pass data, including the code streams that the code 
		 *							block code will be written to.
		 */
		static void parseCodeBlock(ASTFXNode* codeNode, const Vector<CodeBlock>& codeBlocks, PassData& passData);

		/**
		 * Parses the pass AST node and generates a single pass object. Returns null if no pass can be parsed. This method
		 * will generate any child state objects and compile any child GPU programs.
		 *
		 * @param[in]	passNode		Node to parse.
		 * @param[in]	codeBlocks		GPU program source code retrieved from parseCodeBlocks().
		 * @param[in]	passData		Data containing pass render state descriptors.
		 * @param[in]	renderAPI		API to use for compiling the GPU programs.
		 * @param[in]	language		GPU program language to use for parsing the provided code blocks.
		 * @param[in]	seqIdx			Output sequential index of the pass that determines its rendering order.
		 */
		static PassPtr parsePass(ASTFXNode* passNode, const Vector<CodeBlock>& codeBlocks, PassData& passData, 
			const StringID& renderAPI, const String& language, UINT32& seqIdx);

		/**
		 * Parses the technique AST node and generates a single technique object. Returns null if no technique can be 
		 * parsed.
		 *
		 * @param[in]	techniqueNode	Node to parse.
		 * @param[in]	codeBlocks		GPU program source code retrieved from parseCodeBlocks().
		 */
		static TechniquePtr parseTechnique(ASTFXNode* techniqueNode, const Vector<CodeBlock>& codeBlocks);

		/**
		 * Parses the parameters AST node and populates the shader descriptor with information about GPU program parameters
		 * and their default values.
		 */
		static void parseParameters(SHADER_DESC& desc, ASTFXNode* parametersNode);

		/**
		 * Parses the blocks AST node and populates the shader descriptor with information about GPU program parameter 
		 * blocks.
		 */
		static void parseBlocks(SHADER_DESC& desc, ASTFXNode* blocksNode);

		/**
		 * Parses the AST node hierarchy and generates a shader object.
		 *
		 * @param[in]		name		Optional name for the shader.
		 * @param[in, out]	parseState	Parser state object that has previously been initialized with the AST using 
		 *								parseFX().
		 * @param	codeBlocks			GPU program source code retrieved from parseCodeBlocks().
		 * @return						A result object containing the shader if successful, or error message if not.
		 */
		static BSLFXCompileResult parseShader(const String& name, ParseState* parseState, Vector<CodeBlock>& codeBlocks);

		/**
		 * Converts a null-terminated string into a standard string, and eliminates quotes that are assumed to be at the 
		 * first and last index.
		 */
		static String removeQuotes(const char* input);

		/**	Retrieves a GPU program profile to use with the specified API and GPU program type. */
		static GpuProgramProfile getProfile(const StringID& renderAPI, GpuProgramType type);

		/** Returns one of the builtin textures based on their name. */
		static HTexture getBuiltinTexture(const String& name);
	};
}