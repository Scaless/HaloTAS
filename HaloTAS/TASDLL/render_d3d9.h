#pragma once


#include <d3d9.h>
#include <d3dx9.h>
#include <atomic>
#include <filesystem>
#include <map>

struct D3DModel {
	LPDIRECT3DVERTEXBUFFER9 v_buffer{ NULL };
	UINT v_bufferSize{ 0 };
	bool active{ false };
};

struct CUSTOMVERTEX
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;

	static IDirect3DVertexDeclaration9* Decl;
};

class render_d3d9
{
#pragma region D3D9_Singleton
public:
	static render_d3d9& get() {
		static render_d3d9 instance;
		return instance;
	}
	render_d3d9(render_d3d9 const&) = delete;
	void operator=(render_d3d9 const&) = delete;
private:
	render_d3d9() = default;
#pragma endregion

private:
	bool initialized {false};
	void initialize(IDirect3DDevice9* device);

	void load_model(IDirect3DDevice9* device, std::filesystem::path path);
	void load_all_models(IDirect3DDevice9* device);
	
	void renderModel(IDirect3DDevice9* device, D3DModel model);

	bool enabled{ false };
	DWORD fillMode{ D3DFILL_SOLID };
	D3DCOLORVALUE materialColor{ 1,1,1,0.5f };
	D3DCOLORVALUE lightColor{ 0.5f, 0.5f, 0.5f, 0.2f };
	D3DCULL cullMode{ D3DCULL_CW };
	float cullDistance{ 250.0f };
	float fovOffset{ 0.0f };
	bool alphaModeColor{ true };

	std::map<std::string, D3DModel> models {};

public:
	void render(IDirect3DDevice9* device);

	// Functionality Toggles
	void SetEnabled(bool _enabled);
	void SetFillMode(_D3DFILLMODE _mode);
	void SetMaterialColor(D3DCOLORVALUE _materialColor);
	void SetLightColor(D3DCOLORVALUE _lightColor);
	void SetCullMode(D3DCULL _cullMode);
	void SetFoVOffset(float _offset);
	void ToggleTransparencyMode();
	void SetCullDistance(float _cullDistance);

	//// Returns a list of loaded model names
	//std::vector<std::string> ModelNames();
	//// Sets if model is visible or not
	//void SetModelVisibility(std::string modelName, bool enabled);

	std::map<std::string, D3DModel>& Models();

};

