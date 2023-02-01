#include "ray_trace_model.hpp"
#include "../utils/utils.hpp"

#include <cstring>
#include <iostream>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "bvh.hpp"

namespace nugiEngine {
	EngineRayTraceModel::EngineRayTraceModel(EngineDevice &device, RayTraceModelData &datas) : engineDevice{device} {
		auto bvhData = this->createBvhData(datas);
		auto triangleData = this->createObjectData(datas);

		this->createBuffers(triangleData, bvhData);
	}

	EngineRayTraceModel::~EngineRayTraceModel() {}

	SphereData EngineRayTraceModel::createObjectData(const RayTraceModelData &data) {
		SphereData object;
		for (int i = 0; i < data.spheres.size(); i++) {
			object.spheres[i] = data.spheres[i];
		}

		return object;
	}

	BvhData EngineRayTraceModel::createBvhData(const RayTraceModelData &data) {
		std::vector<SphereBoundBox> objects;
		for (int i = 0; i < data.spheres.size(); i++) {
			Sphere t = data.spheres[i];
			objects.push_back({i, t});
		}

		auto bvhNodes = createBvh(objects);
		BvhData bvh{};

		for (int i = 0; i < bvhNodes.size(); i++) {
			bvh.bvhNodes[i] = bvhNodes[i];
		}

		return bvh;
	}

	void EngineRayTraceModel::createBuffers(SphereData &data, BvhData &bvh) {
		EngineBuffer objectStagingBuffer {
			this->engineDevice,
			sizeof(SphereData),
			1,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		objectStagingBuffer.map();
		objectStagingBuffer.writeToBuffer(&data);

		this->objectBuffer = std::make_shared<EngineBuffer>(
			this->engineDevice,
			sizeof(SphereData),
			1,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		this->objectBuffer->copyBuffer(objectStagingBuffer.getBuffer(), sizeof(SphereData));

		EngineBuffer bvhStagingBuffer {
			this->engineDevice,
			sizeof(BvhData),
			1,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		bvhStagingBuffer.map();
		bvhStagingBuffer.writeToBuffer(&bvh);

		this->bvhBuffer = std::make_shared<EngineBuffer>(
			this->engineDevice,
			sizeof(BvhData),
			1,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		this->bvhBuffer->copyBuffer(bvhStagingBuffer.getBuffer(), sizeof(BvhData));
	}

	std::unique_ptr<EngineRayTraceModel> EngineRayTraceModel::createModelFromFile(EngineDevice &device, const std::string &filePath) {
		RayTraceModelData modelData;
		modelData.loadModel(filePath);

		return std::make_unique<EngineRayTraceModel>(device, modelData);
	}

	void RayTraceModelData::loadModel(const std::string &filePath) {
		
	}
    
} // namespace nugiEngine
