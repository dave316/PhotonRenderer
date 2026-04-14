#ifndef INCLUDED_GLPIPELINE
#define INCLUDED_GLPIPELINE

#pragma once

#include <GPU/Pipeline.h>
#include <GPU/GL/GLBuffer.h>
#include <GPU/GL/GLProgram.h>
#include <GPU/GL/GLDescriptorPool.h>
#include <GPU/GL/GLSampler.h>
#include <glm/glm.hpp>

namespace GL
{
	GLenum getShaderType(GPU::ShaderStage stage);
	class GraphicsPipeline : public GPU::GraphicsPipeline
	{
	public:	
		GraphicsPipeline(std::string name);
		~GraphicsPipeline();
		void setVertexInputDescripton(GPU::VertexDescription& inputDescription);
		void setLayout(GPU::DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts);
		void setLayout(GPU::DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts, std::vector<GPU::PushConstant> pushConstants);
		void addShaderStage(std::string code, GPU::ShaderStage stage);
		void pushConstants(uint8* data);
		void setDepthTest(bool depthTestEnabled, bool depthWriteEnabled);
		void setStencilTest(bool stencilTestEnabled, uint32 stencilMask, uint32 refValue, GPU::CompareOp compareOp);
		void setColorMask(bool red, bool green, bool blue, bool alpha);
		void setBlending(bool blendingEnabled);		
		void setScissorTest(bool scissorTestEnabled);
		void setCullMode(int mode);
		void setWindingOrder(int frontFace);
		void createProgram();
		void use();
		std::vector<int> getLayoutBindings(std::string name);

		typedef std::shared_ptr<GraphicsPipeline> Ptr;
		static Ptr create(std::string name)
		{
			return std::make_shared<GraphicsPipeline>(name);
		}
	private:
		GLuint vao = 0;
		GL::Program program;
		GL::Buffer::Ptr pushConstantUBO;
		std::map<std::string, std::vector<int>> layout;

		bool depthTestEnabled = true;
		bool depthWriteEnabled = true;
		bool stencilTestEnabled = false;
		uint32 stencilMask = 0x00;
		uint32 stencilRefValue = 0;
		GPU::CompareOp stencilCompOp = GPU::CompareOp::Always;
		bool colorMask[4] = { true, true, true, true };
		bool scissorTestEnabled = false;
		bool blendingEnabled = false;
		int cullMode = 0;

		GraphicsPipeline(const GraphicsPipeline&) = delete;
		GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
	};

	class ComputePipeline : public GPU::ComputePipeline
	{
	public:
		ComputePipeline(std::string name);
		~ComputePipeline();
		void setLayout(GPU::DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts);
		void addShaderStage(std::string code, GPU::ShaderStage stage);
		void createProgram();
		void use();
		std::vector<int> getLayoutBindings(std::string name);
		typedef std::shared_ptr<ComputePipeline> Ptr;
		static Ptr create(std::string name)
		{
			return std::make_shared<ComputePipeline>(name);
		}

	private:
		GL::Program program;
		std::map<std::string, std::vector<int>> layout;
		ComputePipeline(const ComputePipeline&) = delete;
		ComputePipeline& operator=(const ComputePipeline&) = delete;
	};
}

#endif // INCLUDED_GLPIPELINE