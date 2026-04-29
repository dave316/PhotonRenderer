#ifndef INCLUDED_RENDERER
#define INCLUDED_RENDERER

#pragma once

#include <Platform/Types.h>
#include <Platform/Window.h>

#include <Core/Light.h>
#include <Core/Renderable.h>
#include <Core/Scene.h>

#include <Graphics/UserCamera.h>
#include <Graphics/GraphicsContext.h>
#include <Graphics/Primitive.h>
#include <Graphics/GUI.h>
#include <Graphics/PostProcessor.h>
#include <Graphics/Shadows.h>
#include <Graphics/Volumes.h>
#include <Graphics/Outline.h>
#include <Graphics/Scatter.h>

namespace pr
{
	struct CameraData
	{
		glm::mat4 VP;
		glm::mat4 VP_I;
		glm::mat4 P;
		glm::mat4 P_I;
		glm::mat4 V;
		glm::mat4 V_I;
		glm::vec4 position;
		glm::vec4 time;
		glm::vec4 projParams;
		float zNear;
		float zFar;
		float scale;
		float bias;
	};

	struct Lights
	{
		LightUniformData lightData[10];
		int numLights;
		int padding[3];
	};

	struct Skybox
	{
		int index;
		int lod;
		int padding[2];
	};

	class Renderer
	{
	public:
		Renderer() {}
		void init(Window::Ptr window, GPU::Swapchain::Ptr swapchain = nullptr);
		void initFramebuffers();
		void initDescriptorLayouts();
		void initDescriptorSets();
		void initPipelines();
		void initScene(UserCamera& userCamera, Scene::Ptr scene);
		void resize(uint32 width, uint32 height);
		void prepare(UserCamera& userCamera, pr::Scene::Ptr scene);
		void buildCmdBuffer(pr::Scene::Ptr scene, GPU::Swapchain::Ptr swapchain = nullptr);
		void buildScatterCmdBuffer(pr::Scene::Ptr scene);
		void buildShadowCmdBuffer(pr::Scene::Ptr scene);
		void addLights(pr::Scene::Ptr scene);
		void updateLights(UserCamera& userCamera, pr::Scene::Ptr scene);
		void updateCamera(Scene::Ptr scene, UserCamera& userCamera, float time, int debugChannel = 0);
		void updateCamera(Scene::Ptr scene, glm::mat4 P, glm::mat4 V, glm::vec3 pos, float time, int debugChannel = 0);
		void updateShadows(pr::Scene::Ptr scene);
		void updatePost(Post& post);
		void renderToTexture(pr::Scene::Ptr scene);

		GPU::DescriptorPool::Ptr getDescriptorPool() { return descriptorPool; }
		GPU::CommandBuffer::Ptr getCommandBuffer(int index) {
			return commandBuffers[index];
		}
		pr::Texture2D::Ptr getFinalTex() {
			return finalTex;
		}

		typedef std::shared_ptr<Renderer> Ptr;
		static Ptr create()
		{
			return std::make_shared<Renderer>();
		}

	private:
		PostProcessor postProcessor;
		Shadows shadows;
		Volumes volumes;
		Outline outline;
		Scatter scatter;

		std::map<std::string, GPU::GraphicsPipeline::Ptr> pipelines;
		GPU::GraphicsPipeline::Ptr skyboxPipeline;
		GPU::Framebuffer::Ptr offscreenFramebuffer;
		GPU::Framebuffer::Ptr offscreenFramebuffer2;
		GPU::Framebuffer::Ptr finalFramebuffer;
		GPU::ImageView::Ptr grabView;
		std::vector<GPU::CommandBuffer::Ptr> commandBuffers;

		GPU::DescriptorPool::Ptr descriptorPool;
		GPU::DescriptorSet::Ptr descriptorSetCamera;
		GPU::DescriptorSet::Ptr descriptorSetSkybox;
		GPU::DescriptorSet::Ptr descriptorSetIBL;
		GPU::DescriptorSet::Ptr descriptorSetLight;
		GPU::DescriptorSet::Ptr descriptorSetVolume;
		GPU::DescriptorSet::Ptr animDescriptorSet;
		GPU::DescriptorSet::Ptr morphDescriptorSet;

		GPU::Buffer::Ptr cameraUBO;
		GPU::Buffer::Ptr skyboxUBO;
		GPU::Buffer::Ptr lightUBO;
		GPU::Buffer::Ptr reflectionProbeUBO;

		// IBL
		pr::Texture2D::Ptr brdfLUT;
		pr::TextureCubeMap::Ptr skybox;
		pr::TextureCubeMap::Ptr irradianceMap;
		pr::TextureCubeMap::Ptr prefilteredMapCharlie;
		pr::TextureCubeMapArray::Ptr reflectionMaps;

		// Post processing
		pr::Texture2D::Ptr screenTex;
		pr::Texture2D::Ptr grabTex;
		pr::Texture2D::Ptr brightTex;
		pr::Texture2D::Ptr depthTex;
		pr::Texture2D::Ptr finalTex;

		uint32 currentCamera = 0;
		std::vector<CameraData> cameras;
		CameraData camera;
		Skybox skyboxData;

		// helper meshes
		pr::Primitive::Ptr unitQuad;
		pr::Primitive::Ptr unitCube;

		uint32 width = 0;
		uint32 height = 0;

		bool updated = false;
		bool offscreen = false;

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
	};
}

#endif // INCLUDED_RENDERER