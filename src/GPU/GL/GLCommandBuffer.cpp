#include "GLCommandBuffer.h"
#include "GLEnums.h"
namespace GL
{
	class CmdBeginRenderPass : public Command
	{
	public:
		CmdBeginRenderPass(GL::Framebuffer::Ptr framebuffer) :
			framebuffer(framebuffer)
		{

		}
		~CmdBeginRenderPass() {}

		void execute()
		{
			glm::vec4 color = framebuffer->getClearColor();
			glClearColor(color.r, color.g, color.b, color.a);
			glClearDepth(1.0f);

			// TODO: this is a work around for the default framebuffer
			if (framebuffer->getNumAttachments() > 0 || framebuffer->hasDepthAttachment())
			{
				framebuffer->bind();
				framebuffer->setDrawBuffers();
			}
			if (framebuffer->clearOnLoad())
			{
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glDepthMask(GL_TRUE);
				glStencilMask(0xFF);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			}
		}

		typedef std::shared_ptr<CmdBeginRenderPass> Ptr;
		static Ptr create(GL::Framebuffer::Ptr framebuffer)
		{
			return std::make_shared<CmdBeginRenderPass>(framebuffer);
		}

	private:
		GL::Framebuffer::Ptr framebuffer;

		CmdBeginRenderPass(const CmdBeginRenderPass&) = delete;
		CmdBeginRenderPass& operator=(const CmdBeginRenderPass&) = delete;
	};

	class CmdEndRenderPass : public Command
	{
	public:
		CmdEndRenderPass()
		{

		}

		void execute()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		typedef std::shared_ptr<CmdEndRenderPass> Ptr;
		static Ptr create()
		{
			return std::make_shared<CmdEndRenderPass>();
		}

	private:
		CmdEndRenderPass(const CmdEndRenderPass&) = delete;
		CmdEndRenderPass& operator=(const CmdEndRenderPass&) = delete;
	};

	class CmdSetViewport : public Command
	{
	public:
		CmdSetViewport(float x, float y, float width, float height) :
			x(x), y(y), width(width), height(height)
		{
		}

		void execute()
		{
			glViewport((GLint)x, (GLint)y, (GLsizei)width, (GLsizei)height);
		}

		typedef std::shared_ptr<CmdSetViewport> Ptr;
		static Ptr create(float x, float y, float width, float height)
		{
			return std::make_shared<CmdSetViewport>(x, y, width, height);
		}

	private:
		float x;
		float y;
		float width;
		float height;

		CmdSetViewport(const CmdSetViewport&) = delete;
		CmdSetViewport& operator=(const CmdSetViewport&) = delete;
	};

	class CmdSetScissor : public Command
	{
	public:
		CmdSetScissor(int32 x, int32 y, uint32 width, uint32 height) :
			x(x), y(y), width(width), height(height)
		{
		}

		void execute()
		{
			glScissor(x, y, width, height);
		}

		typedef std::shared_ptr<CmdSetScissor> Ptr;
		static Ptr create(int32 x, int32 y, uint32 width, uint32 height)
		{
			return std::make_shared<CmdSetScissor>(x, y, width, height);
		}

	private:
		GLint x;
		GLint y;
		GLsizei width;
		GLsizei height;

		CmdSetScissor(const CmdSetScissor&) = delete;
		CmdSetScissor& operator=(const CmdSetScissor&) = delete;
	};

	class CmdBindGraphicsPipeline : public Command
	{
	public:
		CmdBindGraphicsPipeline(GL::GraphicsPipeline::Ptr pipeline) :
			pipeline(pipeline)
		{
		}

		void execute()
		{
			pipeline->use();
		}

		typedef std::shared_ptr<CmdBindGraphicsPipeline> Ptr;
		static Ptr create(GL::GraphicsPipeline::Ptr pipeline)
		{
			return std::make_shared<CmdBindGraphicsPipeline>(pipeline);
		}

	private:
		GL::GraphicsPipeline::Ptr pipeline;

		CmdBindGraphicsPipeline(const CmdBindGraphicsPipeline&) = delete;
		CmdBindGraphicsPipeline& operator=(const CmdBindGraphicsPipeline&) = delete;
	};

	class CmdBindComputePipeline : public Command
	{
	public:
		CmdBindComputePipeline(GL::ComputePipeline::Ptr pipeline) :
			pipeline(pipeline)
		{
		}

		void execute()
		{
			pipeline->use();
		}

		typedef std::shared_ptr<CmdBindComputePipeline> Ptr;
		static Ptr create(GL::ComputePipeline::Ptr pipeline)
		{
			return std::make_shared<CmdBindComputePipeline>(pipeline);
		}

	private:
		GL::ComputePipeline::Ptr pipeline;

		CmdBindComputePipeline(const CmdBindComputePipeline&) = delete;
		CmdBindComputePipeline& operator=(const CmdBindComputePipeline&) = delete;
	};

	class CmdPushConstants : public Command
	{
	public:
		CmdPushConstants(GL::GraphicsPipeline::Ptr pipeline, uint8* data, uint32 size) :
			pipeline(pipeline),
			size(size)
		{
			this->data = new uint8[size];
			std::memcpy(this->data, data, size);
		}

		void execute()
		{
			pipeline->pushConstants(data);
		}

		typedef std::shared_ptr<CmdPushConstants> Ptr;
		static Ptr create(GL::GraphicsPipeline::Ptr pipeline, uint8* data, uint32 size)
		{
			return std::make_shared<CmdPushConstants>(pipeline, data, size);
		}

	private:
		uint8* data;
		uint32 size;
		GL::GraphicsPipeline::Ptr pipeline;

		CmdPushConstants(const CmdPushConstants&) = delete;
		CmdPushConstants& operator=(const CmdPushConstants&) = delete;
	};


	class CmdBindDescriptorSets : public Command
	{
	public:
		CmdBindDescriptorSets(GL::DescriptorSet::Ptr descriptorSet, std::vector<int> bindings) :
			descriptorSet(descriptorSet),
			bindings(bindings)
		{
		}

		void execute()
		{
			//auto bindings = pipeline->getLayoutBindings(descriptorSet->getLayoutName());
			descriptorSet->bind(bindings);
		}

		typedef std::shared_ptr<CmdBindDescriptorSets> Ptr;
		static Ptr create(GL::DescriptorSet::Ptr descriptorSet, std::vector<int> bindings)
		{
			return std::make_shared<CmdBindDescriptorSets>(descriptorSet, bindings);
		}

	private:
		GL::DescriptorSet::Ptr descriptorSet;
		std::vector<int> bindings;

		CmdBindDescriptorSets(const CmdBindDescriptorSets&) = delete;
		CmdBindDescriptorSets& operator=(const CmdBindDescriptorSets&) = delete;
	};

	class CmdBindVertexBuffers : public Command
	{
	public:
		CmdBindVertexBuffers(GLuint buffer,	GLsizei stride) :
			buffer(buffer),
			stride(stride)
		{
		}

		void execute()
		{
			glBindVertexBuffer(0, buffer, 0, stride);
		}

		typedef std::shared_ptr<CmdBindVertexBuffers> Ptr;
		static Ptr create(GLuint buffer, GLsizei stride)
		{
			return std::make_shared<CmdBindVertexBuffers>(buffer, stride);
		}

	private:
		GLuint buffer;
		GLsizei stride;

		CmdBindVertexBuffers(const CmdBindVertexBuffers&) = delete;
		CmdBindVertexBuffers& operator=(const CmdBindVertexBuffers&) = delete;
	};


	class CmdBindIndexBuffers : public Command
	{
	public:
		CmdBindIndexBuffers(GLuint buffer) :
			buffer(buffer)
		{
		}

		void execute()
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
		}

		typedef std::shared_ptr<CmdBindIndexBuffers> Ptr;
		static Ptr create(GLuint buffer)
		{
			return std::make_shared<CmdBindIndexBuffers>(buffer);
		}

	private:
		GLuint buffer;

		CmdBindIndexBuffers(const CmdBindIndexBuffers&) = delete;
		CmdBindIndexBuffers& operator=(const CmdBindIndexBuffers&) = delete;
	};

	class CmdSetCullMode : public Command
	{
	public:
		CmdSetCullMode(int mode) :
			mode(mode)
		{
		}

		void execute()
		{
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

		typedef std::shared_ptr<CmdSetCullMode> Ptr;
		static Ptr create(int mode)
		{
			return std::make_shared<CmdSetCullMode>(mode);
		}

	private:
		int mode;

		CmdSetCullMode(const CmdSetCullMode&) = delete;
		CmdSetCullMode& operator=(const CmdSetCullMode&) = delete;
	};

	class CmdDrawIndexed : public Command
	{
	public:
		CmdDrawIndexed(uint32 indexCount, GPU::Topology topology) :
			indexCount(indexCount),
			topology(topology)
		{
		}

		void execute()
		{
			glDrawElements(getTopology(topology), indexCount, GL_UNSIGNED_INT, 0);
		}

		typedef std::shared_ptr<CmdDrawIndexed> Ptr;
		static Ptr create(uint32 indexCount, GPU::Topology topology)
		{
			return std::make_shared<CmdDrawIndexed>(indexCount, topology);
		}

	private:
		uint32 indexCount;
		GPU::Topology topology;

		CmdDrawIndexed(const CmdDrawIndexed&) = delete;
		CmdDrawIndexed& operator=(const CmdDrawIndexed&) = delete;
	};

	class CmdDrawIndexedBaseVertex : public Command
	{
	public:
		CmdDrawIndexedBaseVertex(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset) :
			indexCount(indexCount),
			indexOffset(indexOffset),
			vertexOffset(vertexOffset)
		{
		}

		void execute()
		{
			glDrawElementsBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, (void*)(intptr_t)(indexOffset * sizeof(GLushort)), vertexOffset);
		}

		typedef std::shared_ptr<CmdDrawIndexedBaseVertex> Ptr;
		static Ptr create(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset)
		{
			return std::make_shared<CmdDrawIndexedBaseVertex>(indexCount, indexOffset, vertexOffset);
		}

	private:
		uint32 indexCount;
		uint32 indexOffset;
		uint32 vertexOffset;

		CmdDrawIndexedBaseVertex(const CmdDrawIndexedBaseVertex&) = delete;
		CmdDrawIndexedBaseVertex& operator=(const CmdDrawIndexedBaseVertex&) = delete;
	};

	class CmdDrawArrays : public Command
	{
	public:
		CmdDrawArrays(uint32 vertexCount) :
			vertexCount(vertexCount)
		{
		}

		void execute()
		{
			glDrawArrays(GL_TRIANGLES, 0, vertexCount);
		}

		typedef std::shared_ptr<CmdDrawArrays> Ptr;
		static Ptr create(uint32 vertexCount)
		{
			return std::make_shared<CmdDrawArrays>(vertexCount);
		}

	private:
		uint32 vertexCount;

		CmdDrawArrays(const CmdDrawArrays&) = delete;
		CmdDrawArrays& operator=(const CmdDrawArrays&) = delete;
	};

	class CmdDispatchCompute : public Command
	{
	public:
		CmdDispatchCompute(uint32 grpCountX, uint32 grpCountY, uint32 grpCountZ) :
			grpCountX(grpCountX),
			grpCountY(grpCountY),
			grpCountZ(grpCountZ)
		{
		}

		void execute()
		{
			glDispatchCompute(grpCountX, grpCountY, grpCountZ);
		}

		typedef std::shared_ptr<CmdDispatchCompute> Ptr;
		static Ptr create(uint32 grpCountX, uint32 grpCountY, uint32 grpCountZ)
		{
			return std::make_shared<CmdDispatchCompute>(grpCountX, grpCountY, grpCountZ);
		}

	private:
		uint32 grpCountX;
		uint32 grpCountY;
		uint32 grpCountZ;

		CmdDispatchCompute(const CmdDispatchCompute&) = delete;
		CmdDispatchCompute& operator=(const CmdDispatchCompute&) = delete;
	};

	class CmdPipelineBarrier : public Command
	{
	public:
		CmdPipelineBarrier()
		{
		}

		void execute()
		{
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}

		typedef std::shared_ptr<CmdPipelineBarrier> Ptr;
		static Ptr create()
		{
			return std::make_shared<CmdPipelineBarrier>();
		}

	private:
		CmdPipelineBarrier(const CmdPipelineBarrier&) = delete;
		CmdPipelineBarrier& operator=(const CmdPipelineBarrier&) = delete;
	};

	class CmdGenerateMipmap : public Command
	{
	public:
		CmdGenerateMipmap(GLenum target, GLuint texture) :
			target(target),
			texture(texture)
		{
		}

		void execute()
		{
			glBindTexture(target, texture);
			glGenerateMipmap(target);
		}

		typedef std::shared_ptr<CmdGenerateMipmap> Ptr;
		static Ptr create(GLenum target, GLuint texture)
		{
			return std::make_shared<CmdGenerateMipmap>(target, texture);
		}

	private:
		GLenum target;
		GLuint texture;
		CmdGenerateMipmap(const CmdGenerateMipmap&) = delete;
		CmdGenerateMipmap& operator=(const CmdGenerateMipmap&) = delete;
	};

	CommandBuffer::CommandBuffer()
	{}

	CommandBuffer::~CommandBuffer()
	{}

	void CommandBuffer::begin()
	{
		commands.clear();
	}

	void CommandBuffer::end()
	{
		// TODO: prevent new commands being added!
	}

	void CommandBuffer::beginRenderPass(GPU::Framebuffer::Ptr framebuffer)
	{
		auto glFramebuffer = std::dynamic_pointer_cast<Framebuffer>(framebuffer);
		commands.push_back(CmdBeginRenderPass::create(glFramebuffer));
		//glm::vec4 color = glFramebuffer->getClearColor();
		//glClearColor(color.r, color.g, color.b, color.a);
		//glClearDepth(1.0f);

		//// TODO: this is a work around for the default framebuffer
		//if (glFramebuffer->getNumAttachments() > 0 || glFramebuffer->hasDepthAttachment())
		//{
		//	glFramebuffer->bind();
		//	glFramebuffer->setDrawBuffers();
		//}
		//if (glFramebuffer->clearOnLoad())
		//{
		//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//}
	}

	void CommandBuffer::endRenderPass()
	{
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		commands.push_back(CmdEndRenderPass::create());
	}

	void CommandBuffer::setViewport(float x, float y, float width, float height)
	{
		//glViewport(x, y, width, height);

		commands.push_back(CmdSetViewport::create(x, y, width, height));
	}

	void CommandBuffer::setScissor(int32 x, int32 y, uint32 width, uint32 height)
	{
		//glScissor(x, y, width, height);

		commands.push_back(CmdSetScissor::create(x, y, width, height));
	}

	void CommandBuffer::bindPipeline(GPU::GraphicsPipeline::Ptr pipeline)
	{
		auto glPipeline = std::dynamic_pointer_cast<GraphicsPipeline>(pipeline);
		//glPipeline->use();

		commands.push_back(CmdBindGraphicsPipeline::create(glPipeline));
	}

	void CommandBuffer::bindPipeline(GPU::ComputePipeline::Ptr pipeline)
	{
		auto glPipeline = std::dynamic_pointer_cast<ComputePipeline>(pipeline);
		//glPipeline->use();

		commands.push_back(CmdBindComputePipeline::create(glPipeline));
	}

	void CommandBuffer::pushConstants(GPU::GraphicsPipeline::Ptr pipeline, GPU::ShaderStage stage, uint32 offset, uint32 size, const void* values)
	{
		auto glPipeline = std::dynamic_pointer_cast<GraphicsPipeline>(pipeline);
		//glPipeline->pushConstants((uint8*)values);

		commands.push_back(CmdPushConstants::create(glPipeline, (uint8*)values, size));
	}

	void CommandBuffer::bindDescriptorSets(GPU::GraphicsPipeline::Ptr pipeline, GPU::DescriptorSet::Ptr descriptorSet, uint32 firstSet)
	{
		auto glPipeline = std::dynamic_pointer_cast<GraphicsPipeline>(pipeline);
		auto glDescriptorSet = std::dynamic_pointer_cast<DescriptorSet>(descriptorSet);
		auto bindings = glPipeline->getLayoutBindings(glDescriptorSet->getLayoutName());
		//glDescriptorSet->bind(bindings);

		commands.push_back(CmdBindDescriptorSets::create(glDescriptorSet, bindings));
	}

	void CommandBuffer::bindDescriptorSets(GPU::ComputePipeline::Ptr pipeline, GPU::DescriptorSet::Ptr descriptorSet, uint32 firstSet)
	{
		auto glPipeline = std::dynamic_pointer_cast<ComputePipeline>(pipeline);
		auto glDescriptorSet = std::dynamic_pointer_cast<DescriptorSet>(descriptorSet);
		auto bindings = glPipeline->getLayoutBindings(glDescriptorSet->getLayoutName());
		//glDescriptorSet->bind(bindings);

		commands.push_back(CmdBindDescriptorSets::create(glDescriptorSet, bindings));
	}

	void CommandBuffer::bindVertexBuffers(GPU::Buffer::Ptr vertexBuffer)
	{
		auto vbo = std::dynamic_pointer_cast<Buffer>(vertexBuffer);
		//glBindVertexBuffer(0, vbo->getID(), 0, vbo->getStride());

		commands.push_back(CmdBindVertexBuffers::create(vbo->getID(), vbo->getStride()));
	}

	void CommandBuffer::bindIndexBuffers(GPU::Buffer::Ptr indexBuffer, GPU::IndexType indexType)
	{
		auto vbo = std::dynamic_pointer_cast<Buffer>(indexBuffer);
		//vbo->bind();
		commands.push_back(CmdBindIndexBuffers::create(vbo->getID()));
	}

	void CommandBuffer::setCullMode(int mode)
	{
		//if (mode > 0)
		//	glEnable(GL_CULL_FACE);

		//switch (mode)
		//{
		//	case 0: glDisable(GL_CULL_FACE); break;
		//	case 1: glCullFace(GL_FRONT); break;
		//	case 2: glCullFace(GL_BACK); break;
		//	case 3: glCullFace(GL_FRONT_AND_BACK); break;
		//}

		commands.push_back(CmdSetCullMode::create(mode));
	}

	void CommandBuffer::drawIndexed(uint32 indexCount, GPU::Topology topology)
	{
		//glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

		commands.push_back(CmdDrawIndexed::create(indexCount, topology));
	}

	void CommandBuffer::drawIndexed(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset)
	{
		// TODO: get index type and stride from index buffer
		//glDrawElementsBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, (void*)(intptr_t)(indexOffset * sizeof(GLushort)), vertexOffset);
		commands.push_back(CmdDrawIndexedBaseVertex::create(indexCount, indexOffset, vertexOffset));
	}

	void CommandBuffer::drawArrays(uint32 vertexCount)
	{
		//glDrawArrays(GL_TRIANGLES, 0, vertexCount);

		commands.push_back(CmdDrawArrays::create(vertexCount));
	}

	void CommandBuffer::dispatchCompute(uint32 grpCountX, uint32 grpCountY, uint32 grpCountZ)
	{
		//glDispatchCompute(grpCountX, grpCountY, grpCountZ);
		commands.push_back(CmdDispatchCompute::create(grpCountX, grpCountY, grpCountZ));
	}

	void CommandBuffer::pipelineBarrier()
	{
		//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		commands.push_back(CmdPipelineBarrier::create());
	}

	void CommandBuffer::flush()
	{
		for (auto cmd : commands)
			cmd->execute();
	}

	void CommandBuffer::generateMipmap(GLenum target, GLuint texture)
	{
		commands.push_back(CmdGenerateMipmap::create(target, texture));
	}
}