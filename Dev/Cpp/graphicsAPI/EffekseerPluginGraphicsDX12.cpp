
#include "EffekseerPluginGraphicsDX12.h"
#include "../unity/IUnityGraphics.h"
#include "../unity/IUnityGraphicsD3D12.h"
#include "../unity/IUnityInterface.h"
#include <algorithm>
#include <assert.h>

#include "../common/EffekseerPluginMaterial.h"

namespace EffekseerPlugin
{

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

		ES_SAFE_RELEASE(srv);

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

void GraphicsDX12::SetBackGroundTextureToRenderer(EffekseerRenderer::Renderer* renderer, void* backgroundTexture)
{
	((EffekseerRendererDX12::Renderer*)renderer)->SetBackground((ID3D11ShaderResourceView*)backgroundTexture);
}

void GraphicsDX12::SetDepthTextureToRenderer(EffekseerRenderer::Renderer* renderer,
											 const Effekseer::Matrix44& projectionMatrix,
											 void* depthTexture)
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

	auto srv = static_cast<ID3D11ShaderResourceView*>(depthTexture);

	auto texture = EffekseerRendererDX11::CreateTexture(graphicsDevice_, srv, nullptr, nullptr);
	renderer->SetDepth(texture, param);
}

void GraphicsDX12::SetExternalTexture(int renderId, ExternalTextureType type, void* texture)
{
	HRESULT hr;

	// create ID3D11ShaderResourceView because a texture type is ID3D11Texture2D from Unity on DX11
	ID3D11Texture2D* textureDX11 = (ID3D11Texture2D*)texture;
	ID3D11ShaderResourceView* srv = (ID3D11ShaderResourceView*)renderSettings[renderId].externalTextures[static_cast<int>(type)];

	if (srv != nullptr)
	{
		ID3D11Resource* res = nullptr;
		srv->GetResource(&res);
		if (res != texture)
		{
			// if texture is not same, delete it
			srv->Release();
			srv = nullptr;
			renderSettings[renderId].externalTextures[static_cast<int>(type)] = nullptr;
		}
		ES_SAFE_RELEASE(res);
	}

	if (srv == nullptr && texture != nullptr)
	{
		D3D11_TEXTURE2D_DESC texDesc;
		textureDX11->GetDesc(&texDesc);

		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		// adjust format
		switch (texDesc.Format)
		{
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
			desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			break;
		case DXGI_FORMAT_R16_TYPELESS:
			desc.Format = DXGI_FORMAT_R16_FLOAT;
			break;
		default:
			desc.Format = texDesc.Format;
			break;
		}
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MostDetailedMip = 0;
		desc.Texture2D.MipLevels = texDesc.MipLevels;
		hr = d3d11Device->CreateShaderResourceView(textureDX11, &desc, &srv);
		if (SUCCEEDED(hr))
		{
			renderSettings[renderId].externalTextures[static_cast<int>(type)] = srv;
		}
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
	// TODO
}

} // namespace EffekseerPlugin