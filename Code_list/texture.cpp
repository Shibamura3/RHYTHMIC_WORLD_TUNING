//テクスチャ管理
//texture.cpp

#include "texture.h"
#include "direct3d.h"
#include <string>
#include "DirectXTex.h"
#include "WICTextureLoader11.h"
using namespace DirectX;

static constexpr int TEXTURE_MAX = 256;

struct Texture {
	std::wstring fliename;
	ID3D11ShaderResourceView* pTextures;
	unsigned int width;
	unsigned int height;
};

static Texture g_Textures[TEXTURE_MAX]{};
static int g_SetTextureIndex = - 1;

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;


void Texture_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext){
	for (Texture& t : g_Textures) {
		t.pTextures = nullptr;
	}

	g_SetTextureIndex = -1;

	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;


}


void Texture_Finalize(void){
	Texture_AllRelease();
}

int Texture_Load(const wchar_t* pFilename){
	//すでに読み込んだファイルは読み込まない
	for (int i = 0;i < TEXTURE_MAX;i++) {
		if (g_Textures[i].fliename == pFilename) {
			return i;
		}

	}

	//空いている管理領域を探す
	for (int i = 0;i < TEXTURE_MAX;i++) {
		if (g_Textures[i].pTextures)continue;//使用中なら戻る


		//テクスチャの読み込み
		TexMetadata metadata;
		ScratchImage image;

		//ファイルの読み込みをしてmetadataとimageを作る
		//metadataはrgbの数値の塊、機械語
		
		HRESULT hr = LoadFromWICFile(pFilename, WIC_FLAGS_NONE, &metadata, image);

		if (FAILED(hr)) {
			MessageBoxW(nullptr, L"テクスチャの読み込みに失敗しました", pFilename, MB_OK | MB_ICONERROR);
			return -1;
		}
		
		g_Textures[i].fliename = pFilename;
		g_Textures[i].width = (unsigned int) metadata.width;
		g_Textures[i].height = (unsigned int) metadata.height;

		hr = CreateShaderResourceView(g_pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Textures[i].pTextures);
		
		return i;
	}

	return -1;
}

void Texture_AllRelease(){
	for (Texture& t : g_Textures) {
		t.fliename.clear();
		SAFE_RELEASE(t.pTextures);
	}
}

void Texture_SetTexture(int texid){
	if (texid < 0) return ;
	g_SetTextureIndex = texid;

	//テクスチャ設定
	g_pContext->PSSetShaderResources(0, 1, &g_Textures[texid].pTextures);
}

unsigned int Texture_Width(int texid){
	if (texid < 0) return (unsigned int) 0;

	return g_Textures[texid].width;
}

unsigned int Texture_Height(int texid){
	if (texid < 0) return (unsigned int) 0;

	return g_Textures[texid].height;
}
