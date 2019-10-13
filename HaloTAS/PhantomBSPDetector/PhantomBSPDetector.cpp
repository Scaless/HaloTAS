// PhantomBSPDetector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h> 
#include <iostream>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/matrix_decompose.hpp>
#include <gtx/string_cast.hpp>
#include <gtx/vector_angle.hpp>
#include <chrono>
#include <thread>
#include <filesystem>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

struct vertex {
	glm::vec3 pos;
	glm::vec3 normal;
};

bool obj_to_vertex_vector(const char* path, std::vector<vertex>& vec) {

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path);
	if (!warn.empty()) {
		std::cout << warn;
		return false;
	}
	if (!err.empty()) {
		std::cout << err;
		return false;
	}
	if (!ret) {
		return false;
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

				vertex vert{ glm::vec3(vx, vy, vz), glm::vec3(nx, ny, nz) };
				vec.push_back(vert);
			}
			index_offset += fv;
		}
	}

	return true;
}

bool SapienIsFocused(HWND hSapien) {
	if (GetForegroundWindow() == hSapien)
		return true;
	return false;
}

std::vector<vertex> mapVertexes{};
std::vector<glm::vec3> phantomPositions{};
glm::vec3 PreviousPhantomPosition;
glm::vec3* ADDR_PHANTOM_POSITION = reinterpret_cast<glm::vec3*>(0xE47AF0);
glm::vec3* ADDR_CAMERA_POSITION = reinterpret_cast<glm::vec3*>(0xDB46DC);
glm::vec3* ADDR_CAMERA_POSITION_ACTUAL = reinterpret_cast<glm::vec3*>(0xCA4634);
glm::vec3* ADDR_CAMERA_LOOKAT = reinterpret_cast<glm::vec3*>(0xDB46E8);

// Heron's Formula
float triangleArea(glm::vec3 vA, glm::vec3 vB, glm::vec3 vC) {
	float lA = glm::length(vA - vB);
	float lB = glm::length(vA - vC);
	float lC = glm::length(vC - vB);

	//float s = .5f * (lA + lB + lC);

	//float area = glm::sqrt(s * (s - lA)*(s - lB)*(s - lC));

	return .25f * glm::sqrt((lA+lB+lC)*(-lA+lB+lC)*(lA-lB+lC)*(lA+lB-lC));
}

int main()
{
	//auto hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	//SetConsoleTextAttribute(hConsole, 0);

	HWND hWnd = FindWindow(0, TEXT("Sapien"));
	if (hWnd == 0) {
		hWnd = FindWindow(0, TEXT("Sapien - [Game window]"));
		if (hWnd == 0) {
			std::cerr << "Sapien isn't open." << std::endl;
			return 1;
		}
	}
	DWORD pId;
	GetWindowThreadProcessId(hWnd, &pId);
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pId);
	if (hProc == 0) {
		std::cout << "Couldn't open process " << pId << ": " << GetLastError() << std::endl;
		return 1;
	}

	SIZE_T bytes_read = 0;
	if (!ReadProcessMemory(hProc, (LPVOID)ADDR_PHANTOM_POSITION, (LPVOID)&PreviousPhantomPosition, sizeof(PreviousPhantomPosition), &bytes_read)) {
		return 1;
	}

	std::string objName;
	std::cout << "Enter an OBJ to load: ";
	std::getline(std::cin, objName);

	if (!obj_to_vertex_vector(objName.c_str(), mapVertexes)) {
		std::cout << "There was a problem loading the obj. Closing." << std::endl;;
		return 1;
	}
	else {
		std::cout << "Loaded: " << objName << std::endl;
	}

	std::string outputFileName = objName + ".txt";
	std::ofstream outputFile;
	outputFile.open(outputFileName, std::ios::out | std::ios::app);
	outputFile << "===STARTING SCAN===" << std::endl;

	auto startTime = std::chrono::high_resolution_clock::now();
	int triCount = mapVertexes.size() / 3;
	int skippedTriangles = 0;
	std::cout << "Mesh has " << triCount << " triangles." << std::endl;
	// Begin Loop
	for (int i = 0; i < triCount; i++) {

		while (!SapienIsFocused(hWnd)) {
			std::cout << "Sapien is not focused, waiting..." << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(2));
		}

		// Report Progress
		if (i > 0 && i % 1000 == 0) {
			float done = (float)i / (float)triCount;

			auto now = std::chrono::high_resolution_clock::now();
			auto timeDiff = now - startTime;
			auto totalTime = (1.0f / done) * timeDiff;
			auto remaining = totalTime - timeDiff;

			auto hours = std::chrono::duration_cast<std::chrono::hours>(remaining);
			remaining -= hours;
			auto minutes = std::chrono::duration_cast<std::chrono::minutes>(remaining);
			remaining -= minutes;
			auto seconds = std::chrono::duration_cast<std::chrono::seconds>(remaining);

			std::cout << i << "/" << triCount << " (" << skippedTriangles << " skipped too small) (" << (1.0f - done) * 100.0f << "% remaining) | ETA "
				<< hours.count() << "h" << minutes.count() << "m" << seconds.count() << "s" << std::endl;
		}
		
		glm::vec3 normal = mapVertexes[i * 3].normal;
		glm::vec3 vA, vB, vC;
		vA = mapVertexes[i * 3].pos;
		vB = mapVertexes[i * 3 + 1].pos;
		vC = mapVertexes[i * 3 + 2].pos;

		float area = triangleArea(vA, vB, vC);
		float min_area = .1f;

		if (area < min_area) {
			skippedTriangles++;
			continue;
		}

		glm::vec3 center = glm::vec3(
			(vA.x + vB.x + vC.x) / 3.0f,
			(vA.y + vB.y + vC.y) / 3.0f,
			(vA.z + vB.z + vC.z) / 3.0f
		);

		glm::vec3 newCameraPos = center + (glm::normalize(normal) * .075f);
		
		glm::vec3 newCameraRot{0};

		glm::vec3 rotVecStartX = glm::vec3(-1, 0, 0);
		glm::vec3 rotVecTarget = glm::normalize(normal);
		auto yaw = glm::orientedAngle(rotVecStartX, rotVecTarget, glm::vec3(0, 0, 1));
		auto pitch = -glm::normalize(normal).z * (glm::pi<float>() / 2);

		newCameraRot.x = yaw;
		newCameraRot.y = pitch;

		SIZE_T bytes_written = 0;
		if (!WriteProcessMemory(hProc, (LPVOID)ADDR_CAMERA_LOOKAT, (LPCVOID)&newCameraRot, sizeof(newCameraRot), &bytes_written))
			std::cout << "Couldn't write process memory:" << GetLastError() << std::endl;
		if (!WriteProcessMemory(hProc, (LPVOID)ADDR_CAMERA_POSITION, (LPCVOID)&newCameraPos, sizeof(newCameraPos), &bytes_written))
			std::cout << "Couldn't write process memory:" << GetLastError() << std::endl;
		if (!WriteProcessMemory(hProc, (LPVOID)ADDR_CAMERA_POSITION_ACTUAL, (LPCVOID)& newCameraPos, sizeof(newCameraPos), &bytes_written))
			std::cout << "Couldn't write process memory:" << GetLastError() << std::endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(20));

		SIZE_T bytes_read = 0;
		glm::vec3 newPhantomPos;
		if (!ReadProcessMemory(hProc, (LPVOID)ADDR_PHANTOM_POSITION, (LPVOID)&newPhantomPos, sizeof(PreviousPhantomPosition), &bytes_read)) {
			return 1;
		}

		if (PreviousPhantomPosition != newPhantomPos) {
			std::cout << "FOUND PHANTOM AT: " << glm::to_string(newPhantomPos) << std::endl;
			
			outputFile << glm::to_string(newPhantomPos) << std::endl;

			phantomPositions.push_back(newPhantomPos);
			PreviousPhantomPosition = newPhantomPos;
		}
	}

	outputFile.close();

	std::cout << "SCAN COMPLETE" << std::endl;
	std::cout << "All Phantom Positions:" << std::endl;

	for (auto& pos : phantomPositions) {
		std::cout << glm::to_string(pos) << std::endl;
	}

}
