
#include "EffekseerPluginGraphicsUnity.h"
#include "../renderer/EffekseerRendererImplemented.h"
#include "../renderer/EffekseerRendererMaterialLoader.h"
#include "../renderer/EffekseerRendererModelLoader.h"
#include "../renderer/EffekseerRendererTextureLoader.h"
#include <algorithm>
#include <assert.h>

namespace EffekseerPlugin
{

GraphicsUnity::GraphicsUnity() {}

GraphicsUnity::~GraphicsUnity() {}

bool GraphicsUnity::Initialize(IUnityInterfaces* unityInterface) { return true; }

void GraphicsUnity::AfterReset(IUnityInterfaces* unityInterface) {}

void GraphicsUnity::Shutdown(IUnityInterfaces* unityInterface) {}

Effekseer::RefPtr<EffekseerRenderer::Renderer> GraphicsUnity::CreateRenderer(int squareMaxCount, bool reversedDepth)
{
	auto renderer = EffekseerRendererUnity::RendererImplemented::Create();
	if (renderer->Initialize(squareMaxCount))
	{
		return renderer;
	}
	else
	{
		return nullptr;
	}
}

void GraphicsUnity::SetBackGroundTextureToRenderer(EffekseerRenderer::Renderer* renderer, Effekseer::TextureRef backgroundTexture)
{
	renderer->SetBackground(backgroundTexture);
}

void GraphicsUnity::SetDepthTextureToRenderer(EffekseerRenderer::Renderer* renderer,
											  const Effekseer::Matrix44& projectionMatrix,
											  Effekseer::TextureRef depthTexture)
{
	if (depthTexture == nullptr)
	{
		renderer->SetDepth(nullptr, EffekseerRenderer::DepthReconstructionParameter{});
		return;
	}

	EffekseerRenderer::DepthReconstructionParameter param;
	param.DepthBufferScale = 1.0f;
	param.DepthBufferOffset = 0.0f;
	param.ProjectionMatrix33 = projectionMatrix.Values[2][2];
	param.ProjectionMatrix43 = projectionMatrix.Values[2][3];
	param.ProjectionMatrix34 = projectionMatrix.Values[3][2];
	param.ProjectionMatrix44 = projectionMatrix.Values[3][3];

	renderer->SetDepth(depthTexture, param);
}

void GraphicsUnity::SetExternalTexture(int renderId, ExternalTextureType type, void* texture)
{
	if (texture != nullptr)
	{
		renderSettings[renderId].externalTextures[static_cast<int>(type)] = Effekseer::MakeRefPtr<EffekseerRendererUnity::Texture>(texture);
	}
	else
	{
		renderSettings[renderId].externalTextures[static_cast<int>(type)].Reset();
	}
}

Effekseer::TextureLoaderRef GraphicsUnity::Create(TextureLoaderLoad load, TextureLoaderUnload unload)
{
	return Effekseer::MakeRefPtr<EffekseerRendererUnity::TextureLoader>(load, unload);
}

Effekseer::ModelLoaderRef GraphicsUnity::Create(ModelLoaderLoad load, ModelLoaderUnload unload)
{
	return Effekseer::MakeRefPtr<EffekseerRendererUnity::ModelLoader>(load, unload);
}

Effekseer::MaterialLoaderRef GraphicsUnity::Create(MaterialLoaderLoad load, MaterialLoaderUnload unload)
{
	return Effekseer::MakeRefPtr<EffekseerRendererUnity::MaterialLoader>(load, unload);
}

Effekseer::ProceduralModelGeneratorRef GraphicsUnity::Create(ProceduralModelGeneratorGenerate generate,
															 ProceduralModelGeneratorUngenerate ungenerate)
{
	return Effekseer::MakeRefPtr<EffekseerRendererUnity::ProceduralModelGenerator>(generate, ungenerate);
}

void GraphicsUnity::ShiftViewportForStereoSinglePass(bool isShift) {}

} // namespace EffekseerPlugin