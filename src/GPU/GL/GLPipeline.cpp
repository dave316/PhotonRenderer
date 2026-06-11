#include "GLPipeline.h"
#include "GLEnums.h"

namespace GL
{
	GraphicsPipeline::GraphicsPipeline(std::string name) : 
		GPU::GraphicsPipeline(name)
	{
		glGenVertexArrays(1, &vao);
	}

	GraphicsPipeline::~GraphicsPipeline()
	{
		glDeleteVertexArrays(1, &vao);
	}

	void GraphicsPipeline::setVertexInputDescripton(GPU::VertexDescription& inputDescription)
	{
		glBindVertexArray(vao);
		uint32 bindingIndex = inputDescription.binding;
		uint32 stride = inputDescription.stride;
		auto inputRate = inputDescription.inputRate; // TODO: add divisor if per instance
		for (int i = 0; i < inputDescription.inputAttributes.size(); i++)
		{
			auto attrib = inputDescription.inputAttributes[i];
			GLuint attribIndex = attrib.location;
			GLint size = getSize(attrib.format); // TODO: add other formats beside float...
			glEnableVertexAttribArray(attribIndex);
			glVertexAttribBinding(attribIndex, bindingIndex);
			if (attrib.format == GPU::VertexAttribFormat::Vectur4UC)
				glVertexAttribFormat(attribIndex, size, GL_UNSIGNED_BYTE, GL_TRUE, attrib.offset);
			else
				glVertexAttribFormat(attribIndex, size, GL_FLOAT, GL_FALSE, attrib.offset);
		}
	}

	void GraphicsPipeline::setLayout(GPU::DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts)
	{
		uint32 bufferDescriptorCount = 0;
		uint32 imageDescriptorCount = 0;
		auto glDescriptorPool = std::dynamic_pointer_cast<DescriptorPool>(descriptorPool);
		for (auto layoutName : setLayouts)
		{
			std::vector<int> setBindings;
			auto layoutBindings = glDescriptorPool->getLayout(layoutName);
			for (auto dslb : layoutBindings)
			{
				if (dslb.descriptorType == GPU::DescriptorType::UniformBuffer)
				{
					setBindings.push_back(bufferDescriptorCount);
					bufferDescriptorCount++;
				}
				else if (dslb.descriptorType == GPU::DescriptorType::CombinedImageSampler)
				{
					for (uint32 i = 0; i < dslb.count; i++)
					{
						setBindings.push_back(imageDescriptorCount);
						imageDescriptorCount++;
					}
				}
			}
			layout[layoutName] = setBindings;
		}

		//for (auto [name, bindings] : layout)
		//{
		//	std::cout << "descriptor set " << name << " bindings" << std::endl;
		//	for (auto binding : bindings)
		//		std::cout << binding << std::endl;
		//}
	}

	void GraphicsPipeline::setLayout(GPU::DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts, std::vector<GPU::PushConstant> pushConstants)
	{
		pushConstantUBO = Buffer::create(GPU::BufferUsage::UniformBuffer, pushConstants[0].size, 0);

		// there are no push constants in OpenGL, so regular uniform variables are used
		setLayout(descriptorPool, setLayouts);
	}

	void GraphicsPipeline::addShaderStage(std::string code, GPU::ShaderStage stage)
	{
		GLenum shaderType = getShaderType(stage);
		Shader shader(shaderType);
		if (shader.compile(code.c_str()))
		{
			program.attachShader(shader);
		}
		else
		{
			std::cout << "error compiling shader " << std::endl;
			std::cout << shader.getErrorLog() << std::endl;
		}
	}

	void GraphicsPipeline::pushConstants(uint8* data)
	{
		pushConstantUBO->uploadMapped(data);
		pushConstantUBO->bindBase(0); // TODO: map push constant UBO to unused bindings...
	}

	void GraphicsPipeline::setDepthTest(bool depthTestEnabled, bool depthWriteEnabled)
	{
		this->depthTestEnabled = depthTestEnabled;
		this->depthWriteEnabled = depthWriteEnabled;
	}

	void GraphicsPipeline::setBlending(bool blendingEnabled)
	{
		this->blendingEnabled = blendingEnabled;
	}

	void GraphicsPipeline::setStencilTest(bool stencilTestEnabled, uint32 stencilMask, uint32 refValue, GPU::CompareOp compareOp)
	{
		this->stencilTestEnabled = stencilTestEnabled;
		this->stencilMask = stencilMask;
		this->stencilRefValue = refValue;
		this->stencilCompOp = compareOp;
	}

	void GraphicsPipeline::setColorMask(bool red, bool green, bool blue, bool alpha)
	{
		colorMask[0] = red;
		colorMask[1] = green;
		colorMask[2] = blue;
		colorMask[3] = alpha;
	}

	void GraphicsPipeline::setScissorTest(bool scissorTestEnable)
	{
		this->scissorTestEnabled = scissorTestEnable;
	}

	void GraphicsPipeline::setCullMode(int mode)
	{
		// TODO: set mode when pipeline is used!
		if (mode > 0)
			glEnable(GL_CULL_FACE);

		switch (mode)
		{
			case 0: glDisable(GL_CULL_FACE); break;
			case 1: glCullFace(GL_FRONT); break;
			case 2: glCullFace(GL_BACK); break;
			case 3: glCullFace(GL_FRONT_AND_BACK); break;
		}
	}

	void GraphicsPipeline::setWindingOrder(int frontFace)
	{

	}

	void GraphicsPipeline::createProgram()
	{
		if (program.link())
		{
			program.loadUniforms();
		}
		else
		{
			std::cout << "error linking shader program " << std::endl;
			std::cout << program.getErrorLog() << std::endl;
		}
	}

	void GraphicsPipeline::use()
	{
		if (depthTestEnabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

		if (depthWriteEnabled)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(GL_FALSE);

		if (stencilTestEnabled)
		{
			glEnable(GL_STENCIL_TEST);
			glStencilMask(stencilMask);
			glStencilFunc(GL::getCompareFunc(stencilCompOp), stencilRefValue, 0xFF);
			glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
		}
		else
		{
			glDisable(GL_STENCIL_TEST);
		}

		glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);

		if (scissorTestEnabled)
			glEnable(GL_SCISSOR_TEST);
		else
			glDisable(GL_SCISSOR_TEST);

		if (blendingEnabled)
		{
			glEnable(GL_BLEND);

			// TODO: add custumizeable blend function/equation to pipeline object
			//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // IMGUI
			glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // ALPHA
			glBlendEquation(GL_FUNC_ADD);
		}			
		else
			glDisable(GL_BLEND);

		glBindVertexArray(vao);
		program.use();
	}

	std::vector<int> GraphicsPipeline::getLayoutBindings(std::string name)
	{
		return layout[name];
	}

	ComputePipeline::ComputePipeline(std::string name) :
		GPU::ComputePipeline(name)
	{

	}
	ComputePipeline::~ComputePipeline()
	{

	}
	void ComputePipeline::setLayout(GPU::DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts)
	{
		uint32 bufferDescriptorCount = 0;
		uint32 imageDescriptorCount = 0;
		auto glDescriptorPool = std::dynamic_pointer_cast<GL::DescriptorPool>(descriptorPool);
		for (auto layoutName : setLayouts)
		{
			std::vector<int> setBindings;
			auto layoutBindings = glDescriptorPool->getLayout(layoutName);
			for (auto dslb : layoutBindings)
			{
				if (dslb.descriptorType == GPU::DescriptorType::UniformBuffer)
				{
					setBindings.push_back(bufferDescriptorCount);
					bufferDescriptorCount++;
				}
				else if (dslb.descriptorType == GPU::DescriptorType::CombinedImageSampler ||
						 dslb.descriptorType == GPU::DescriptorType::StorageImage)
				{
					for (uint32 i = 0; i < dslb.count; i++)
					{
						setBindings.push_back(imageDescriptorCount);
						imageDescriptorCount++;
					}
				}
			}
			layout[layoutName] = setBindings;
		}
	}

	void ComputePipeline::addShaderStage(std::string code, GPU::ShaderStage stage)
	{
		// TODO: check if compute shader
		GLenum shaderType = GL_COMPUTE_SHADER;
		GL::Shader shader(shaderType);
		if (shader.compile(code.c_str()))
		{
			program.attachShader(shader);
		}
		else
		{
			std::cout << "error compiling shader " << std::endl;
			std::cout << shader.getErrorLog() << std::endl;
		}
	}

	void ComputePipeline::createProgram()
	{
		if (program.link())
		{
			program.loadUniforms();
		}
		else
		{
			std::cout << "error linking shader program " << std::endl;
			std::cout << program.getErrorLog() << std::endl;
		}
	}

	void ComputePipeline::use()
	{
		program.use();
	}

	std::vector<int> ComputePipeline::getLayoutBindings(std::string name)
	{
		return layout[name];
	}
}