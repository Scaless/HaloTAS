#pragma once


#include <d3d9.h>
#include <d3dx9.h>
#include <atomic>


struct CUSTOMVERTEX
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DCOLOR color;

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

	bool enabled{ false };
	DWORD fillMode{ D3DFILL_SOLID };

	LPDIRECT3DVERTEXBUFFER9 v_buffer {NULL};
	UINT v_bufferSize{ 0 };

public:
	void render(IDirect3DDevice9* device);

	// Functionality Toggles
	void Enable();
	void Disable();
	void SetFillModeWireframe();
	void SetFillModeSolid();

};

