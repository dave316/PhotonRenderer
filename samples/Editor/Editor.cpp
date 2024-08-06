#include "Editor.h"

#include <Graphics/MeshPrimitives.h>

#include <IO/SceneExporter.h>
#include <IO/AssimpImporter.h>
#include <IO/GLTFImporter.h>
#include <IO/Unity/UnityImporter.h>
#include <IO/Image/ImageDecoder.h>
#include <Utils/Color.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <sstream>
#include <stack>

using namespace std::placeholders;

Editor::Editor(const char* title, unsigned int width, unsigned int height) :
	window(title, width, height),
	renderer(width, height)
{
	camera.setAspect((float)width / (float)height);
}

class EditCommand
{
public:
	virtual void redo() = 0;
	virtual void undo() = 0;
	typedef std::shared_ptr<EditCommand> Ptr;
};

class TranslateCommand : public EditCommand
{
private:
	std::vector<Transform::Ptr> targetTransforms;
	glm::vec3 offset;
	glm::vec3 invOffset;
public:
	TranslateCommand(std::vector<Transform::Ptr>& transforms, glm::vec3 offset) :
		targetTransforms(transforms),
		offset(offset),
		invOffset(-offset)
	{
	}
	void redo()
	{
		for (auto t : targetTransforms)
			t->translate(offset);
	}
	void undo()
	{
		for (auto t : targetTransforms)
			t->translate(invOffset);
	}

	typedef std::shared_ptr<TranslateCommand> Ptr;
	static Ptr create(std::vector<Transform::Ptr>& transforms, glm::vec3 offset)
	{
		return std::make_shared<TranslateCommand>(transforms, offset);
	}
};

void testUndoRedo(std::vector<Transform::Ptr> transforms)
{
	std::vector<EditCommand::Ptr> undoCommands;
	std::vector<EditCommand::Ptr> redoCommands;

	// user edits object (fills undo stack)
	undoCommands.push_back(TranslateCommand::create(transforms, glm::vec3(0, 1, 0)));
	undoCommands.push_back(TranslateCommand::create(transforms, glm::vec3(1, 0, 0)));
	undoCommands.push_back(TranslateCommand::create(transforms, glm::vec3(0, 0, 1)));
	undoCommands.push_back(TranslateCommand::create(transforms, glm::vec3(0, 2, 0)));
	undoCommands.push_back(TranslateCommand::create(transforms, glm::vec3(0, -1, 0)));

	// user pressed ctrl+z twice
	for (int i = 0; i < 2; i++)
	{
		auto cmd = undoCommands.back();
		cmd->undo();
		redoCommands.push_back(cmd);
		undoCommands.pop_back();
	}

	// user pressed ctrl+y once
	auto cmd = redoCommands.back();
	cmd->redo();
	undoCommands.push_back(cmd);
	redoCommands.pop_back();

	// user does a new edit (redo stack is cleared)
	undoCommands.push_back(TranslateCommand::create(transforms, glm::vec3(0, 0, -1)));
	redoCommands.clear();

	// user leaves application (both stacks are cleared)
	undoCommands.clear();
	redoCommands.clear();
}

bool Editor::init()
{
	if (!window.isInitialized())
		return false;
	window.attachInput(input);

	const char* glsl_version = "#version 460";

	std::string assetPath = "../../../../assets";

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// TODO: check if font file available first.....
	std::string fontPath = assetPath + "/Fonts/arial.ttf";
	io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 28);
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	if (!renderer.init())
		return false;

	scene = Scene::create("Scene");

	std::string envFn = assetPath + "/Footprint_Court/Footprint_Court_2k.hdr";
	auto panoImg = IO::decodeHDRFromFile(envFn, true);

	Skybox skybox;
	skybox.texture = panoImg->upload(false);
	scene->setSkybox(skybox);

	renderer.setIBL(false);
	renderer.setBloom(false);
	renderer.setTonemappingOp(0);

	renderer.updateCamera(camera);
	renderer.prepare(scene);

	setupInput();

	return true;
}

void Editor::initCamera()
{
	auto bbox = scene->getBoundingBox();
	glm::vec3 minPoint = bbox.getMinPoint();
	glm::vec3 maxPoint = bbox.getMaxPoint();
	glm::vec3 diag = maxPoint - minPoint;

	float aspect = camera.getAspect();
	float fovy = camera.getFov();
	float fovx = fovy * aspect;
	float xZoom = diag.x * 0.5 / glm::tan(fovx / 2.0f);
	float yZoom = diag.y * 0.5 / glm::tan(fovy / 2.0f);
	float dist = glm::max(xZoom, yZoom);
	glm::vec3 center = bbox.getCenter();
	center.z += dist * 2.0f;
	camera.setPosition(center);

	float longestDistance = glm::distance(minPoint, maxPoint);
	float zNear = dist - (longestDistance * 5.0f);
	float zFar = dist + (longestDistance * 5.0f);
	zNear = glm::max(zNear, zFar / 10000.0f);
	camera.setPlanes(zNear, zFar);
	camera.setSpeed(longestDistance);
	camera.setVelocity(longestDistance * 0.1f);
}

void Editor::setupInput()
{
	input.addKeyCallback(GLFW_KEY_ESCAPE, GLFW_PRESS, std::bind(&GLWindow::close, &window));
	input.addKeyCallback(GLFW_KEY_W, GLFW_PRESS, std::bind(&FPSCamera::setDirection, &camera, FPSCamera::Direction::FORWARD));
	input.addKeyCallback(GLFW_KEY_S, GLFW_PRESS, std::bind(&FPSCamera::setDirection, &camera, FPSCamera::Direction::BACK));
	input.addKeyCallback(GLFW_KEY_A, GLFW_PRESS, std::bind(&FPSCamera::setDirection, &camera, FPSCamera::Direction::LEFT));
	input.addKeyCallback(GLFW_KEY_D, GLFW_PRESS, std::bind(&FPSCamera::setDirection, &camera, FPSCamera::Direction::RIGHT));
	input.addKeyCallback(GLFW_KEY_W, GLFW_RELEASE, std::bind(&FPSCamera::releaseDirection, &camera, FPSCamera::Direction::FORWARD));
	input.addKeyCallback(GLFW_KEY_S, GLFW_RELEASE, std::bind(&FPSCamera::releaseDirection, &camera, FPSCamera::Direction::BACK));
	input.addKeyCallback(GLFW_KEY_A, GLFW_RELEASE, std::bind(&FPSCamera::releaseDirection, &camera, FPSCamera::Direction::LEFT));
	input.addKeyCallback(GLFW_KEY_D, GLFW_RELEASE, std::bind(&FPSCamera::releaseDirection, &camera, FPSCamera::Direction::RIGHT));
	input.setMouseMoveCallback(std::bind(&FPSCamera::updateRotation, &camera, _1, _2));
	input.setMouseWheelCallback(std::bind(&FPSCamera::updateSpeed, &camera, _1, _2));
	input.addKeyCallback(GLFW_MOUSE_BUTTON_1, GLFW_RELEASE, std::bind(&Editor::selectModel, this));

	input.addKeyCallback(GLFW_KEY_SPACE, GLFW_PRESS, [&] {animate = !animate; });

	input.addKeyCallback(GLFW_KEY_E, GLFW_PRESS, [&] { op = ImGuizmo::TRANSLATE;  });
	input.addKeyCallback(GLFW_KEY_R, GLFW_PRESS, [&] { op = ImGuizmo::ROTATE;  });
	input.addKeyCallback(GLFW_KEY_T, GLFW_PRESS, [&] { op = ImGuizmo::SCALE;  });

	input.setDropCallback(std::bind(&Editor::handleDrop, this, _1, _2));
}

bool Editor::loadGLTFModel(const std::string& name, const std::string& fullpath)
{
	IO::glTF::Importer importer;
	auto rootEntity = importer.importModel(fullpath);
	if (rootEntity)
	{
		scene->addRootEntity(name, rootEntity);
		return true;
	}
	return false;
}

bool Editor::loadModelASSIMP(std::string name, std::string path)
{
	IO::AssimpImporter importer;
	auto model = importer.importModel(path);
	if (model == nullptr)
		return false;
	scene->addRootEntity(name, model);
	return true;
}

void Editor::handleDrop(int count, const char** paths)
{
	if (count != 1)
	{
		std::cout << "only one file allowed!" << std::endl;
		return;
	}

	for (int i = 0; i < count; i++)
	{
		std::string fullPath(paths[i]);
		std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
		int fnIndex = fullPath.find_last_of('/') + 1;
		std::string filename = fullPath.substr(fnIndex, fullPath.length() - fnIndex);
		int lastDot = filename.find_last_of('.');
		std::string name = filename.substr(0, lastDot);

		int extIndex = lastDot + 1;
		std::string ext = filename.substr(extIndex, filename.length() - extIndex);

		if (ext.compare("gltf") == 0 || ext.compare("glb") == 0)
			loadGLTFModel(name, fullPath);
		else
#ifdef WITH_ASSIMP
			loadModelASSIMP(name, fullPath);
#else
			std::cout << "not supported file extension: " << ext << std::endl;
#endif
	}

	renderer.updateCamera(camera);
	renderer.prepare(scene);

	if (animate)
		scene->playAnimations();
	else
		scene->stopAnimations();

	initCamera();
}

void Editor::selectModel()
{
	int mouseX = ImGui::GetMousePos().x - windowPos.x;
	int mouseY = ImGui::GetMousePos().y - windowPos.y;
	//std::cout << "clicket at pos " << mouseX << " " << mouseY << std::endl;

	if (mouseX > 0 && mouseX < windowSize.x && mouseY > 0 && mouseY < windowSize.y)
	{
		if (ImGuizmo::IsUsing())
			return;

		glm::mat4 P = camera.getProjectionMatrix();
		glm::mat4 V = camera.getViewMatrix();

		glm::vec4 vp(0.0f, 0.0f, windowSize.x, windowSize.y);
		glm::vec3 start = glm::vec3(mouseX, windowSize.y - mouseY, 0.0f);
		glm::vec3 end = glm::vec3(mouseX, windowSize.y - mouseY, 1.0f);
		glm::vec3 startWorld = glm::unProject(start, V, P, vp);
		glm::vec3 endWorld = glm::unProject(end, V, P, vp);
		auto hitModels = scene->selectModelsRaycast(startWorld, endWorld);

		if (hitModels.empty())
		{
			selectedModel = nullptr;
			scene->unselect();
		}			
		else
		{
			if (selectedModel != nullptr)
			{
				unsigned int lastID = selectedModel->getID();
				int index = -1;
				for (int i = 0; i < hitModels.size(); i++) // check if current model has been selected again
				{
					if (lastID == hitModels[i]->getID())
						index = i;
				}
				if (index >= 0) // if model has been selected again cycle through next models
				{
					int nextIndex = index + 1;
					if (nextIndex < hitModels.size())
						selectedModel = hitModels[nextIndex];
					else
						selectedModel = hitModels[0];
				}
				else // if model is not selected again use first model
				{ 
					selectedModel = hitModels[0];
				}
			}
			else
			{
				selectedModel = hitModels[0]; // if nothing was selected, use the first model
			}
		}
	}
}

void Editor::addTreeNode(Entity::Ptr entity)
{
	std::string name = entity->getName();
	unsigned int currentID = entity->getID();
	int selectedID = (selectedModel != nullptr ? selectedModel->getID() : -1);

	if (selectedModel != nullptr)
	{
		auto parent = selectedModel->getParent();
		while(parent != nullptr)
		{
			if(parent->getID() == currentID)
				ImGui::SetNextItemOpen(true, ImGuiCond_Always);
			parent = parent->getParent();
		}
	}

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= (currentID == selectedID ? ImGuiTreeNodeFlags_Selected : 0);
	flags |= (entity->numChildren() == 0 ? ImGuiTreeNodeFlags_Leaf : 0);

	bool open = ImGui::TreeNodeEx(name.c_str(), flags);

	ImGui::PushID(name.c_str());
	if (ImGui::BeginPopupContextItem())
	{
		ImGui::Text(name.c_str());
		ImGui::EndPopup();
	}
	ImGui::PopID();

	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
	{
		if (selectedID == currentID)
		{
			selectedModel = nullptr;
			scene->unselect();
		}			
		else
			selectedModel = entity;
	}

	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity");
		if (payload != NULL)
		{
			unsigned int id = *(unsigned int*)(payload->Data);
			auto sourceEntity = scene->getNode(id);
			if (sourceEntity->findByID(entity->getID()) != nullptr)
			{
				std::cout << "target node is part of source tree!" << std::endl;
			}
			else
			{
				std::string sourceName = sourceEntity->getName();
				auto parent = sourceEntity->getParent();
				if (parent == nullptr)
					scene->removeRootEntity(sourceName);
				else
					parent->removeChild(sourceName);

				auto sourceTransform = sourceEntity->getComponent<Transform>();
				glm::mat4 sourceT = sourceTransform->getTransform();
				glm::mat4 targetT = entity->getComponent<Transform>()->getTransform();
				glm::mat4 M = glm::inverse(targetT) * sourceT;
				sourceTransform->setLocalTransform(M);
				sourceEntity->setParent(entity);
				entity->addChild(sourceEntity);
			}
		}
			
		ImGui::EndDragDropTarget();
	}

	if (ImGui::BeginDragDropSource())
	{
		unsigned int id = entity->getID();
		ImGui::SetDragDropPayload("Entity", &id, sizeof(unsigned int));
		ImGui::Text(name.c_str());
		ImGui::EndDragDropSource();
	}

	if (open)
	{
		for (int i = 0; i < entity->numChildren(); i++)
			addTreeNode(entity->getChild(i));
		ImGui::TreePop();
	}		
}

void Editor::gui()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::MenuItem("New");
			if (ImGui::MenuItem("Open GLTF"))
				ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Select GLTF file", ".gltf,.glb", "../../../../assets");
			if (ImGui::MenuItem("Save Scene"))
				IO::saveScene("scene.json", scene);
			if (ImGui::MenuItem("Load Scene"))
			{
				scene = IO::loadScene("scene.json");

				std::string envFn = "../../../../assets/Footprint_Court/Footprint_Court_2k.hdr";
				auto panoImg = IO::decodeHDRFromFile(envFn, true);

				Skybox skybox;
				skybox.texture = panoImg->upload(false);
				skybox.exposure = 1.0f;
				skybox.rotation = 0.0f;
				scene->setSkybox(skybox);

				scene->updateAnimations(0.0f);
				renderer.updateCamera(camera);
				renderer.prepare(scene);
			}				
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Entity"))
		{
			if (ImGui::MenuItem("Empty"))
				scene->addRootEntity("Empty", Entity::create("Empty", nullptr));
			if (ImGui::MenuItem("Model"))
			{
				// TODO: add primitive geometry
			}
			if (ImGui::MenuItem("Light"))
			{
				glm::vec3 lightColor = glm::vec3(1.0f);

				auto entity = Entity::create("light", nullptr);
				entity->addComponent(Light::create(LightType::POINT, lightColor, 50.0f, 100.0f));

				auto r = Renderable::create();
				float radius = 0.005f;
				auto prim = MeshPrimitives::createUVSphere(glm::vec3(0), radius, 32, 32);
				prim->setBoundingBox(glm::vec3(-radius), glm::vec3(radius));
				auto mat = getDefaultMaterial();
				mat->addProperty("material.baseColorFactor", glm::vec4(lightColor, 1.0));
				mat->addProperty("material.unlit", true);

				SubMesh s;
				s.primitive = prim;
				s.material = mat;

				auto mesh = Mesh::create("Sphere");
				mesh->addSubMesh(s);
				r->setMesh(mesh);
				entity->addComponent(r);

				scene->addRootEntity("light", entity);

				renderer.initLights(scene, camera);
				renderer.initShadows(scene);
			}			

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Animation"))
		{
			if (ImGui::MenuItem("Play"))
				scene->playAnimations();

			if (ImGui::MenuItem("Stop"))
				scene->stopAnimations();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			ImGui::MenuItem("About");

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	static ImGuiWindowFlags gizmoWindowFlags = 0;
	if (ImGui::Begin("View", nullptr, gizmoWindowFlags))
	{
		//float windowWidth = (float)ImGui::GetWindowWidth();
		//float windowHeight = (float)ImGui::GetWindowHeight();
		static ImVec2 lastSize = ImVec2(0, 0);
		windowSize = ImGui::GetWindowSize();
		if (windowSize.x != lastSize.x || windowSize.y != lastSize.y)
		{
			//std::cout << "window size: " << windowSize.x << " " << windowSize.y << std::endl;
			//std::cout << "window pos: " << ImGui::GetCursorScreenPos().x << " " << ImGui::GetCursorScreenPos().y << std::endl;
			camera.setAspect(windowSize.x / windowSize.y);
			renderer.resize(windowSize.x, windowSize.y);
		}			
		lastSize = windowSize;

		auto tex = renderer.renderToTexture(scene);
		ImTextureID id;
		ImGui::GetWindowDrawList()->AddImage(
			(void*)tex->getID(), 
			ImVec2(ImGui::GetCursorScreenPos()), 
			ImVec2(ImGui::GetCursorScreenPos().x + windowSize.x, ImGui::GetCursorScreenPos().y + windowSize.y),
			ImVec2(0, 1), 
			ImVec2(1, 0)
		);

		windowPos = ImGui::GetCursorScreenPos();

		ImGuizmo::SetID(0);

		if (selectedModel != nullptr)
		{
			auto t = selectedModel->getComponent<Transform>();
			glm::mat4 modelMatrix = t->getTransform();
			const float* V = glm::value_ptr(camera.getViewMatrix());
			const float* P = glm::value_ptr(camera.getProjectionMatrix());
			glm::mat4 T = glm::mat4(1.0f);
			auto r = selectedModel->getComponent<Renderable>();
			if (r != nullptr)
			{
				Box bbox = r->getBoundingBox();
				glm::vec3 gizmoPos = bbox.getCenter();
				T = glm::translate(glm::mat4(1.0f), gizmoPos);
				modelMatrix = modelMatrix * T;
			}
			else
			{
				if (op == ImGuizmo::TRANSLATE)
				{
					auto renderEntities = selectedModel->getChildrenWithComponent<Renderable>();
					Box selectedBox;
					for (auto e : renderEntities)
					{
						auto t = e->getComponent<Transform>();
						auto r = e->getComponent<Renderable>();
						Box bbox = r->getBoundingBox();
						glm::mat4 local2world = t->getTransform();
						glm::vec3 minPoint = glm::vec3(local2world * glm::vec4(bbox.getMinPoint(), 1.0f));
						glm::vec3 maxPoint = glm::vec3(local2world * glm::vec4(bbox.getMaxPoint(), 1.0f));
						minPoint = glm::vec3(glm::inverse(modelMatrix) * glm::vec4(minPoint, 1.0f));
						maxPoint = glm::vec3(glm::inverse(modelMatrix) * glm::vec4(maxPoint, 1.0f));
						selectedBox.expand(minPoint);
						selectedBox.expand(maxPoint);
					}
					t->setBounds(selectedBox);
				}

				glm::vec3 gizmoPos = t->getBounds().getCenter();
				T = glm::translate(glm::mat4(1.0f), gizmoPos);
				modelMatrix = modelMatrix * T;
			}

			float* M = glm::value_ptr(modelMatrix);

			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);
			ImGuizmo::Manipulate(V, P, op, ImGuizmo::LOCAL, M);

			auto parent = selectedModel->getParent();
			glm::mat4 localTransform = modelMatrix * glm::inverse(T);
			glm::mat4 parentTransform = glm::mat4(1.0f);
			if (parent != nullptr)
			{
				parentTransform = parent->getComponent<Transform>()->getTransform();
				localTransform = glm::inverse(parentTransform) * localTransform;
			}
			t->setLocalTransform(localTransform);

			selectedModel->update(parentTransform);
			renderer.initLights(scene, camera);
			renderer.updateShadows(scene);
			renderer.setLights(scene->getNumLights());
			scene->select(selectedModel);
		}
		
		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();

		vMin.x += ImGui::GetWindowPos().x;
		vMin.y += ImGui::GetWindowPos().y;
		vMax.x += ImGui::GetWindowPos().x;
		vMax.y += ImGui::GetWindowPos().y;

		gizmoWindowFlags = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(vMin, vMax) ? ImGuiWindowFlags_NoMove : 0;
	}
	ImGui::End();

	if (ImGui::Begin("Scene"))
	{
		auto entities = scene->getRootEntities();
		for(auto [name, entity] : entities)
			addTreeNode(entity);
	}
	ImGui::End();

	if (ImGui::Begin("Properties"))
	{
		if (selectedModel)
		{
			auto t = selectedModel->getComponent<Transform>();
			if (t)
			{
				if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
				{
					auto pos = t->getLocalPosition();
					auto rot = t->getLocalRotation();
					auto rotEuler = glm::degrees(glm::eulerAngles(rot));
					auto scale = t->getLocalScale();
					ImGui::InputFloat3("Position", &pos[0]);
					ImGui::InputFloat3("Rotation", &rotEuler[0]);
					ImGui::InputFloat3("Scale", &scale[0]);
					glm::quat q = glm::quat(glm::radians(rotEuler));
					t->setLocalPosition(pos);
					t->setLocalRotation(q);
					t->setLocalScale(scale);
				}
			}

			auto l = selectedModel->getComponent<Light>();
			if (l)
			{
				if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
				{
					static int currentType = (int)l->getType();
					static int prevType = currentType;
					std::vector<std::string> types = { "Directional", "Point", "Spot", "Area" };
					std::string typeStr = types[currentType];
					if (ImGui::BeginCombo("Type", typeStr.c_str(), 0))
					{
						for (int i = 0; i < types.size(); i++)
						{
							const bool isSelected = (currentType == i);
							if (ImGui::Selectable(types[i].c_str(), isSelected))
								currentType = i;
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}

						l->setType((LightType)currentType);

						ImGui::EndCombo();
					}

					static bool useColorTemp = false;
					if (useColorTemp)
					{
						static int temp = 6500;
						ImGui::SliderInt("Color temperature", &temp, 1000, 15000);
						l->setColorTemp(temp);
					}
					else
					{
						auto col = Color::linearToSRGB(l->getColor());
						ImGui::ColorEdit3("Color", &col[0], ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoAlpha);
						l->setColorSRGB(col);
					}
					ImGui::Checkbox("Use color temperature", &useColorTemp);

					float intensity = l->getLumen();
					ImGui::SliderFloat("Intensity", &intensity, 0, 100000);
					l->setLuminousPower(intensity);

					float range = l->getRange();
					ImGui::SliderFloat("Range", &range, 0, 100);
					l->setRange(range);
				}
			}
		}
	}
	ImGui::End();

	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			cameraIndex = 0;
			materialIndex = 0;

			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
			std::string name = fileName.substr(0, fileName.find_last_of("."));
			
			loadGLTFModel(name, filePathName);

			renderer.updateCamera(camera);
			renderer.prepare(scene);

			initCamera();
		}
		ImGuiFileDialog::Instance()->Close();
	}

	bool show_demo_window = true;
	ImGui::ShowDemoWindow(&show_demo_window);
}

void Editor::loop()
{
	double dt = 0.0;
	double animTime = 0.0f;
	double tickTime = 1.0f / 60.0f;
	double updateTime = 0.0f;
	int frames = 0;


	while (!window.shouldClose())
	{
		double startTime = glfwGetTime();

		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		if (animTime > tickTime)
		{
			if (cameraIndex == 0)
			{
				camera.move(animTime);
				camera.rotate(animTime);
				renderer.updateCamera(camera);
			}

			if (animate)
			{
				scene->updateAnimations(animTime);
				scene->updateAnimationState(animTime);
				renderer.initLights(scene, camera);
				renderer.updateShadows(scene);
			}	
			animTime = 0.0f;
		}

		glViewport(0, 0, 1920, 1080);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		gui();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.swapBuffers();

		dt = glfwGetTime() - startTime;
		animTime += dt;
		updateTime += dt;

		if (updateTime >= 1.0)
		{
			std::string title = "Scene Editor, FPS: " + std::to_string(frames);
			window.setWindowTitle(title);
			updateTime = 0.0;
			frames = 0;
		}
		else
		{
			frames++;
		}
	}
}

void Editor::shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}