
#include "EffekseerPluginGraphicsDX12.h"
#include "../unity/IUnityGraphics.h"
#include "../unity/IUnityGraphicsD3D12.h"
#include "../unity/IUnityInterface.h"
#include <algorithm>
#include <assert.h>

#include "../common/EffekseerPluginMaterial.h"

namespace EffekseerPlugin
{

bool RenderPassDX12::Initialize(IUnityInterfaces* unityInterface, Effekseer::Backend::GraphicsDeviceRef device)
{
	unityInterface_ = unityInterface;
	memoryPool_ = EffekseerRenderer::CreateSingleFrameMemoryPool(device);
	commandList_ = EffekseerRenderer::CreateCommandList(device, memoryPool_);
	return true;
}

void RenderPassDX12::Begin(RenderPass* backRenderPass)
{
	if (memoryPool_ != nullptr)
	{
		memoryPool_->NewFrame();
	}

	if (commandList_ != nullptr)
	{
		EffekseerRendererDX12::BeginCommandList(commandList_, nullptr);
	}
}

void RenderPassDX12::End()
{
	if (commandList_ != nullptr)
	{
		EffekseerRendererDX12::EndCommandList(commandList_);
	}
}

void RenderPassDX12::Execute()
{
	if (commandList_ != nullptr)
	{
		EffekseerRendererDX12::ExecuteCommandList(commandList_);
	}
}

class TextureLoaderDX12 : public TextureLoader
{
	std::map<Effekseer::TextureRef, void*> textureData2NativePtr;
	Effekseer::Backend::GraphicsDeviceRef graphicsDevice_;

public:
	TextureLoaderDX12(TextureLoaderLoad load,
					  TextureLoaderUnload unload,
					  Effekseer::Backend::GraphicsDeviceRef graphicsDevice)
		: TextureLoader(load, unload), graphicsDevice_(graphicsDevice)
	{
	}

	virtual ~TextureLoaderDX12() override = default;

	virtual Effekseer::TextureRef Load(const EFK_CHAR* path, Effekseer::TextureType textureType)
	{
		// Load from unity
		int32_t width, height, format;
		void* texturePtr = load((const char16_t*)path, &width, &height, &format);
		if (texturePtr == nullptr)
		{
			return nullptr;
		}

		// Convert
		ID3D12Resource* resource = reinterpret_cast<ID3D12Resource*>(texturePtr);
		auto backend = EffekseerRendererDX12::CreateTexture(graphicsDevice_, resource);

		auto textureDataPtr = Effekseer::MakeRefPtr<Effekseer::Texture>();
		textureDataPtr->SetBackend(backend);

		textureData2NativePtr[textureDataPtr] = texturePtr;

		return textureDataPtr;
	}

	virtual void Unload(Effekseer::TextureRef source)
	{
		if (source == nullptr)
		{
			return;
		}

		unload(source->GetPath().c_str(), textureData2NativePtr[source]);
		textureData2NativePtr.erase(source);
	}
};

GraphicsDX12::GraphicsDX12() {}

GraphicsDX12::~GraphicsDX12()
{
	assert(device_ == nullptr);
	assert(commandQueue_ == nullptr);
}

bool GraphicsDX12::Initialize(IUnityInterfaces* unityInterface)
{
	unityInterface_ = unityInterface;
	device_ = unityInterface->Get<IUnityGraphicsD3D12>()->GetDevice();
	commandQueue_ = unityInterface->Get<IUnityGraphicsD3D12>()->GetCommandQueue();
	const int swapCount = 2;

	MaterialEvent::Initialize();

	graphicsDevice_ = EffekseerRendererDX12::CreateGraphicsDevice(device_, commandQueue_, swapCount);

	ES_SAFE_ADDREF(device_);
	ES_SAFE_ADDREF(commandQueue_);

	return true;
}

void GraphicsDX12::AfterReset(IUnityInterfaces* unityInterface)
{
}

void GraphicsDX12::Shutdown(IUnityInterfaces* unityInterface)
{
	MaterialEvent::Terminate();
	graphicsDevice_.Reset();
	renderer_ = nullptr;
	ES_SAFE_RELEASE(device_);
	ES_SAFE_RELEASE(commandQueue_);
}

EffekseerRenderer::RendererRef GraphicsDX12::CreateRenderer(int squareMaxCount, bool reversedDepth)
{
	// TODO
	DXGI_FORMAT* renderTargetFormats;
	int32_t renderTargetCount;
	DXGI_FORMAT depthFormat;

	renderer_ = EffekseerRendererDX12::Create(graphicsDevice_, renderTargetFormats, renderTargetCount, depthFormat, reversedDepth, squareMaxCount);
	return renderer_;
}

void GraphicsDX12::SetExternalTexture(int renderId, ExternalTextureType type, void* texture)
{
	if (texture != nullptr)
	{
		ID3D12Resource* resource = reinterpret_cast<ID3D12Resource*>(texture);
		auto backend = EffekseerRendererDX12::CreateTexture(graphicsDevice_, resource);
		renderSettings[renderId].externalTextures[static_cast<int>(type)] = backend;	
	}
	else
	{
		renderSettings[renderId].externalTextures[static_cast<int>(type)] = nullptr;
	}
}

Effekseer::TextureLoaderRef GraphicsDX12::Create(TextureLoaderLoad load, TextureLoaderUnload unload)
{
	return Effekseer::MakeRefPtr<TextureLoaderDX12>(load, unload, graphicsDevice_);
}

Effekseer::ModelLoaderRef GraphicsDX12::Create(ModelLoaderLoad load, ModelLoaderUnload unload)
{
	if (renderer_ == nullptr)
		return nullptr;

	auto loader = Effekseer::MakeRefPtr<ModelLoader>(load, unload);
	auto internalLoader = EffekseerRenderer::CreateModelLoader(renderer_->GetGraphicsDevice(), loader->GetFileInterface());
	loader->SetInternalLoader(internalLoader);
	return loader;
}

Effekseer::MaterialLoaderRef GraphicsDX12::Create(MaterialLoaderLoad load, MaterialLoaderUnload unload)
{
	if (renderer_ == nullptr)
		return nullptr;

	auto loader = Effekseer::MakeRefPtr<MaterialLoader>(load, unload);
	auto internalLoader = renderer_->CreateMaterialLoader();
	auto holder = std::make_shared<MaterialLoaderHolder>(internalLoader);
	loader->SetInternalLoader(holder);
	return loader;
}

void GraphicsDX12::ShiftViewportForStereoSinglePass(bool isShift)
{
}

RenderPass* GraphicsDX12::CreateRenderPass()
{
	auto ret = new RenderPassDX12();
	ret->Initialize(unityInterface_, renderer_->GetGraphicsDevice());
	return ret;
}

void GraphicsDX12::SetRenderPath(EffekseerRenderer::Renderer* renderer, RenderPass* renderPath)
{
	if (renderPath != nullptr)
	{
		auto rt = static_cast<RenderPassDX12*>(renderPath);
		renderer_->SetCommandList(rt->GetCommandList());
	}
	else
	{
		renderer_->SetCommandList(nullptr);
	}
}

void GraphicsDX12::WaitFinish()
{
	if (renderer_ == nullptr)
	{
		return;
	}

	EffekseerRenderer::FlushAndWait(renderer_->GetGraphicsDevice());
}

} // namespace EffekseerPlugin