#include "render_d3d9.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "halo_constants.h"
#include <algorithm>

IDirect3DVertexDeclaration9* CUSTOMVERTEX::Decl = NULL;

void loadModelToVector(const char* path, std::vector<CUSTOMVERTEX> &vec) {

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path);
	if (!warn.empty()) {
		exit(1);
	}
	if (!err.empty()) {
		exit(1);
	}
	if (!ret) {
		exit(1);
	}

	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
				tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
				tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
				tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

				CUSTOMVERTEX vert{ D3DXVECTOR3( vx, vy, vz), D3DXVECTOR3(nx, ny, nz) };
				vec.push_back(vert);
			}
			index_offset += fv;
		}
	}
}

void render_d3d9::initialize(IDirect3DDevice9* device)
{
	initialized = true;

	std::vector<CUSTOMVERTEX> vertexes {};
	loadModelToVector("d40_full.obj", vertexes);

	// create the vertex and store the pointer into v_buffer, which is created globally
	device->CreateVertexBuffer(vertexes.size() * sizeof(CUSTOMVERTEX),
		0,
		0,
		D3DPOOL_MANAGED,
		&v_buffer,
		NULL);

	v_bufferSize = vertexes.size();

	VOID* pVoid;
	v_buffer->Lock(0, 0, (void**)& pVoid, 0);
	memcpy(pVoid, vertexes.data(), v_bufferSize * sizeof(CUSTOMVERTEX));
	v_buffer->Unlock();

	D3DVERTEXELEMENT9 VertexElements[] =
	{
		{0,0,D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0,12,D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		D3DDECL_END()
	};
	device->CreateVertexDeclaration(VertexElements, &CUSTOMVERTEX::Decl);
}

void render_d3d9::render(IDirect3DDevice9* device)
{
	if (!enabled) {
		return;
	}

	if (!initialized) {
		initialize(device);
	}

	glm::vec3 cam = *ADDR_CAMERA_POSITION;
	glm::vec3 look{ ADDR_CAMERA_LOOK_VECTOR[0], ADDR_CAMERA_LOOK_VECTOR[1], ADDR_CAMERA_LOOK_VECTOR[2] };
	look = cam + look;

	IDirect3DStateBlock9* pStateBlock = NULL;
	device->CreateStateBlock(D3DSBT_ALL, &pStateBlock);
	pStateBlock->Capture();

	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	device->SetRenderState(D3DRS_FOGCOLOR, 0);
	device->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
	device->SetRenderState(D3DRS_FOGSTART, 0);
	device->SetRenderState(D3DRS_FOGEND, 1);
	device->SetRenderState(D3DRS_FOGDENSITY, 1);
	device->SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
	device->SetRenderState(D3DRS_AMBIENT, 0);
	device->SetRenderState(D3DRS_COLORVERTEX, TRUE);
	device->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
	device->SetRenderState(D3DRS_CLIPPING, TRUE);
	device->SetRenderState(D3DRS_LIGHTING, TRUE);
	device->SetRenderState(D3DRS_LOCALVIEWER, TRUE);
	device->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
	device->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
	device->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
	device->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2);
	device->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
	device->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
	device->SetRenderState(D3DRS_POINTSIZE_MIN, 1);
	device->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
	device->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
	device->SetRenderState(D3DRS_POINTSCALE_A, 1);
	device->SetRenderState(D3DRS_POINTSCALE_B, 0);
	device->SetRenderState(D3DRS_POINTSCALE_C, 0);
	device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	device->SetRenderState(D3DRS_MULTISAMPLEMASK, 0xffffffff);
	device->SetRenderState(D3DRS_PATCHEDGESTYLE, D3DPATCHEDGE_DISCRETE);
	device->SetRenderState(D3DRS_POINTSIZE_MAX, 1);
	device->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_TWEENFACTOR, 0);
	device->SetRenderState(D3DRS_POSITIONDEGREE, D3DDEGREE_CUBIC);
	device->SetRenderState(D3DRS_NORMALDEGREE, D3DDEGREE_LINEAR);
	device->SetRenderState(D3DRS_MINTESSELLATIONLEVEL, 1);
	device->SetRenderState(D3DRS_MAXTESSELLATIONLEVEL, 1);
	device->SetRenderState(D3DRS_ADAPTIVETESS_X, 0);
	device->SetRenderState(D3DRS_ADAPTIVETESS_Y, 0);
	device->SetRenderState(D3DRS_ADAPTIVETESS_Z, 1);
	device->SetRenderState(D3DRS_ADAPTIVETESS_W, 0);
	device->SetRenderState(D3DRS_ENABLEADAPTIVETESSELLATION,FALSE);

	device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	device->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	device->SetRenderState(D3DRS_LASTPIXEL, TRUE);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	device->SetRenderState(D3DRS_ALPHAREF, 0);
	device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);
	device->SetRenderState(D3DRS_DITHERENABLE, FALSE);
	device->SetRenderState(D3DRS_FOGSTART, 0);
	device->SetRenderState(D3DRS_FOGEND, 1);
	device->SetRenderState(D3DRS_FOGDENSITY, 1);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_DEPTHBIAS, 0);
	device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	device->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	device->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
	device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
	device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	device->SetRenderState(D3DRS_STENCILREF, 0);
	device->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);
	device->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
	device->SetRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
	device->SetRenderState(D3DRS_WRAP0, 0);
	device->SetRenderState(D3DRS_WRAP1, 0);
	device->SetRenderState(D3DRS_WRAP2, 0);
	device->SetRenderState(D3DRS_WRAP3, 0);
	device->SetRenderState(D3DRS_WRAP4, 0);
	device->SetRenderState(D3DRS_WRAP5, 0);
	device->SetRenderState(D3DRS_WRAP6, 0);
	device->SetRenderState(D3DRS_WRAP7, 0);
	device->SetRenderState(D3DRS_WRAP8, 0);
	device->SetRenderState(D3DRS_WRAP9, 0);
	device->SetRenderState(D3DRS_WRAP10, 0);
	device->SetRenderState(D3DRS_WRAP11, 0);
	device->SetRenderState(D3DRS_WRAP12, 0);
	device->SetRenderState(D3DRS_WRAP13, 0);
	device->SetRenderState(D3DRS_WRAP14, 0);
	device->SetRenderState(D3DRS_WRAP15, 0);
	device->SetRenderState(D3DRS_LOCALVIEWER, TRUE);
	device->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
	device->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
	device->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
	device->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2);
	device->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0000000f);
	device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	device->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, 0);
	device->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE);
	device->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, FALSE);
	device->SetRenderState(D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_KEEP);
	device->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP);
	device->SetRenderState(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_KEEP);
	device->SetRenderState(D3DRS_CCW_STENCILFUNC, D3DCMP_ALWAYS);
	device->SetRenderState(D3DRS_COLORWRITEENABLE1, 0x0000000f);
	device->SetRenderState(D3DRS_COLORWRITEENABLE2, 0x0000000f);
	device->SetRenderState(D3DRS_COLORWRITEENABLE3, 0x0000000f);
	device->SetRenderState(D3DRS_BLENDFACTOR, 0xffffffff);
	device->SetRenderState(D3DRS_SRGBWRITEENABLE, 0);
	device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	device->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);
	device->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);

	device->SetTexture(0, NULL);
	device->SetTexture(1, NULL);
	device->SetTexture(2, NULL);
	device->SetTexture(3, NULL);
	device->SetVertexShader(NULL);
	device->SetPixelShader(NULL);
	
	device->SetRenderState(D3DRS_FILLMODE, fillMode);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	if (alphaModeColor) {
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
	}
	else {
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}

	device->SetRenderState(D3DRS_CULLMODE, cullMode);
	device->SetRenderState(D3DRS_ZENABLE, TRUE);
	device->SetRenderState(D3DRS_LIGHTING, TRUE);
	device->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(50, 50, 50));

	device->SetVertexDeclaration(CUSTOMVERTEX::Decl);


	D3DXMATRIX matWorld{};
	D3DXMatrixTranslation(&matWorld, 0,0,0);
	device->SetTransform(D3DTS_WORLD, &matWorld);

	float horizontalFovRadians = **ADDR_PTR_TO_CAMERA_HORIZONTAL_FIELD_OF_VIEW_IN_RADIANS;
	float verticalFov = horizontalFovRadians * (float)1080 / (float)1920;
	//verticalFov = std::clamp(verticalFov - .03f, 0.001f, glm::pi<float>()); // Have to offset by this to get correct ratio for 16:9, need to look into this further

	D3DXMATRIX matView;
	D3DXMatrixLookAtLH(&matView,
		&D3DXVECTOR3(cam.x, cam.y, cam.z),
		&D3DXVECTOR3(look.x, look.y, look.z),
		&D3DXVECTOR3(0.0f, 0.0f, 1.0f));

	//D3DXMATRIX matRotate;
	//D3DXMatrixRotationYawPitchRoll(&matRotate, 0, 0, *ADDR_CAMERA_ROLL);
	//D3DXMatrixMultiply(&matView, &matView, &matRotate);

	device->SetTransform(D3DTS_VIEW, &matView);

	D3DXMATRIX matProjection;

	D3DXMatrixPerspectiveFovLH(&matProjection,
		verticalFov,
		(FLOAT)1600 / (FLOAT)900,
		0.1f,
		cullDistance); 

	D3DXMATRIX matInvertX;
	D3DXMatrixReflect(&matInvertX, &D3DXPLANE(1, 0, 0, 0));

	D3DXMatrixMultiply(&matProjection, &matProjection, &matInvertX);

	device->SetTransform(D3DTS_PROJECTION, &matProjection);

	D3DLIGHT9 light;
	D3DMATERIAL9 material;

	ZeroMemory(&light, sizeof(light)); 
	light.Type = D3DLIGHT_POINT; 
	light.Diffuse = lightColor;
	light.Position = D3DXVECTOR3(cam.x, cam.y, cam.z);
	light.Range = 10.0f;
	light.Falloff = 1.0f;
	light.Attenuation0 = 1.0f;
	light.Attenuation1 = 0.0f;
	light.Attenuation2 = 0.0f;

	device->SetLight(0, &light);
	device->LightEnable(0, TRUE);

	ZeroMemory(&material, sizeof(D3DMATERIAL9));
	material.Diffuse = materialColor;
	material.Ambient = materialColor;
	device->SetMaterial(&material);

	device->SetStreamSource(0, v_buffer, 0, sizeof(CUSTOMVERTEX));
	device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, v_bufferSize);
	device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, v_bufferSize);

	pStateBlock->Apply();
	pStateBlock->Release();

}

void render_d3d9::SetEnabled(bool _enabled)
{
	enabled = _enabled;
}

void render_d3d9::SetFillMode(_D3DFILLMODE _mode)
{
	fillMode = _mode;
}

void render_d3d9::SetMaterialColor(D3DCOLORVALUE _materialColor)
{
	materialColor = _materialColor;
}

void render_d3d9::SetLightColor(D3DCOLORVALUE _lightColor)
{
	lightColor = _lightColor;
}

void render_d3d9::SetCullMode(D3DCULL _cullMode)
{
	cullMode = _cullMode;
}

void render_d3d9::ToggleTransparencyMode()
{
	alphaModeColor = !alphaModeColor;
}

void render_d3d9::SetCullDistance(float _cullDistance)
{
	cullDistance = _cullDistance;
}
