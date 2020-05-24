/*
===========================================================================
Copyright (C) 2009, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/
#include "tr_gpu.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include <set>
#include <vector>

#include "qcommon/q_shared.h"
#include "tr_local.h"

namespace {
bool AllExtensionsSupported(
	const VkPhysicalDevice physicalDevice,
	const std::vector<const char *>& requiredExtensions)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(
		physicalDevice,
		nullptr,
		&extensionCount,
		nullptr);

	std::vector<VkExtensionProperties> deviceExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(
		physicalDevice,
		nullptr,
		&extensionCount,
		deviceExtensions.data());

	std::set<std::string> requiredExtensionSet(
		std::begin(requiredExtensions), std::end(requiredExtensions));

	for (const auto& extension : deviceExtensions)
	{
		requiredExtensionSet.erase(extension.extensionName);
	}

	return requiredExtensionSet.empty();
}

int PickPhysicalDeviceIndex(
	const std::vector<VkPhysicalDevice>& physicalDevices,
	const std::vector<const char *>& requiredExtensions)
{
	int bestDeviceIndex = -1;
	int bestScore = 0;
	for (int i = 0; i < physicalDevices.size(); ++i)
	{
		VkPhysicalDevice physicalDevice = physicalDevices[i];

		if (!AllExtensionsSupported(physicalDevice, requiredExtensions))
		{
			continue;
		}

		VkPhysicalDeviceProperties properties = {};
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);

		int score = 0;
		switch (properties.deviceType)
		{
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				score = 3;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				score = 4;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				score = 2;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				score = 1;
				break;
			default:
				break;
		}

		if (bestDeviceIndex == -1 || score > bestScore)
		{
			bestDeviceIndex = i;
			bestScore = score;
		}
	}

	return bestDeviceIndex;
}

void InitializeDeviceQueue(VkDevice device, GpuQueue& queue)
{
	vkGetDeviceQueue(device, queue.queueFamily, 0, &queue.queue);
}

const char *VkFormatToString(const VkFormat format)
{
	switch (format)
	{
		case VK_FORMAT_UNDEFINED:
			return "UNDEFINED";
		case VK_FORMAT_R4G4_UNORM_PACK8:
			return "R4G4_UNORM_PACK8";
		case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
			return "R4G4B4A4_UNORM_PACK16";
		case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
			return "B4G4R4A4_UNORM_PACK16";
		case VK_FORMAT_R5G6B5_UNORM_PACK16:
			return "R5G6B5_UNORM_PACK16";
		case VK_FORMAT_B5G6R5_UNORM_PACK16:
			return "B5G6R5_UNORM_PACK16";
		case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
			return "R5G5B5A1_UNORM_PACK16";
		case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
			return "B5G5R5A1_UNORM_PACK16";
		case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
			return "A1R5G5B5_UNORM_PACK16";
		case VK_FORMAT_R8_UNORM:
			return "R8_UNORM";
		case VK_FORMAT_R8_SNORM:
			return "R8_SNORM";
		case VK_FORMAT_R8_USCALED:
			return "R8_USCALED";
		case VK_FORMAT_R8_SSCALED:
			return "R8_SSCALED";
		case VK_FORMAT_R8_UINT:
			return "R8_UINT";
		case VK_FORMAT_R8_SINT:
			return "R8_SINT";
		case VK_FORMAT_R8_SRGB:
			return "R8_SRGB";
		case VK_FORMAT_R8G8_UNORM:
			return "R8G8_UNORM";
		case VK_FORMAT_R8G8_SNORM:
			return "R8G8_SNORM";
		case VK_FORMAT_R8G8_USCALED:
			return "R8G8_USCALED";
		case VK_FORMAT_R8G8_SSCALED:
			return "R8G8_SSCALED";
		case VK_FORMAT_R8G8_UINT:
			return "R8G8_UINT";
		case VK_FORMAT_R8G8_SINT:
			return "R8G8_SINT";
		case VK_FORMAT_R8G8_SRGB:
			return "R8G8_SRGB";
		case VK_FORMAT_R8G8B8_UNORM:
			return "R8G8B8_UNORM";
		case VK_FORMAT_R8G8B8_SNORM:
			return "R8G8B8_SNORM";
		case VK_FORMAT_R8G8B8_USCALED:
			return "R8G8B8_USCALED";
		case VK_FORMAT_R8G8B8_SSCALED:
			return "R8G8B8_SSCALED";
		case VK_FORMAT_R8G8B8_UINT:
			return "R8G8B8_UINT";
		case VK_FORMAT_R8G8B8_SINT:
			return "R8G8B8_SINT";
		case VK_FORMAT_R8G8B8_SRGB:
			return "R8G8B8_SRGB";
		case VK_FORMAT_B8G8R8_UNORM:
			return "B8G8R8_UNORM";
		case VK_FORMAT_B8G8R8_SNORM:
			return "B8G8R8_SNORM";
		case VK_FORMAT_B8G8R8_USCALED:
			return "B8G8R8_USCALED";
		case VK_FORMAT_B8G8R8_SSCALED:
			return "B8G8R8_SSCALED";
		case VK_FORMAT_B8G8R8_UINT:
			return "B8G8R8_UINT";
		case VK_FORMAT_B8G8R8_SINT:
			return "B8G8R8_SINT";
		case VK_FORMAT_B8G8R8_SRGB:
			return "B8G8R8_SRGB";
		case VK_FORMAT_R8G8B8A8_UNORM:
			return "R8G8B8A8_UNORM";
		case VK_FORMAT_R8G8B8A8_SNORM:
			return "R8G8B8A8_SNORM";
		case VK_FORMAT_R8G8B8A8_USCALED:
			return "R8G8B8A8_USCALED";
		case VK_FORMAT_R8G8B8A8_SSCALED:
			return "R8G8B8A8_SSCALED";
		case VK_FORMAT_R8G8B8A8_UINT:
			return "R8G8B8A8_UINT";
		case VK_FORMAT_R8G8B8A8_SINT:
			return "R8G8B8A8_SINT";
		case VK_FORMAT_R8G8B8A8_SRGB:
			return "R8G8B8A8_SRGB";
		case VK_FORMAT_B8G8R8A8_UNORM:
			return "B8G8R8A8_UNORM";
		case VK_FORMAT_B8G8R8A8_SNORM:
			return "B8G8R8A8_SNORM";
		case VK_FORMAT_B8G8R8A8_USCALED:
			return "B8G8R8A8_USCALED";
		case VK_FORMAT_B8G8R8A8_SSCALED:
			return "B8G8R8A8_SSCALED";
		case VK_FORMAT_B8G8R8A8_UINT:
			return "B8G8R8A8_UINT";
		case VK_FORMAT_B8G8R8A8_SINT:
			return "B8G8R8A8_SINT";
		case VK_FORMAT_B8G8R8A8_SRGB:
			return "B8G8R8A8_SRGB";
		case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
			return "A8B8G8R8_UNORM_PACK32";
		case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
			return "A8B8G8R8_SNORM_PACK32";
		case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
			return "A8B8G8R8_USCALED_PACK32";
		case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
			return "A8B8G8R8_SSCALED_PACK32";
		case VK_FORMAT_A8B8G8R8_UINT_PACK32:
			return "A8B8G8R8_UINT_PACK32";
		case VK_FORMAT_A8B8G8R8_SINT_PACK32:
			return "A8B8G8R8_SINT_PACK32";
		case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
			return "A8B8G8R8_SRGB_PACK32";
		case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
			return "A2R10G10B10_UNORM_PACK32";
		case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
			return "A2R10G10B10_SNORM_PACK32";
		case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
			return "A2R10G10B10_USCALED_PACK32";
		case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
			return "A2R10G10B10_SSCALED_PACK32";
		case VK_FORMAT_A2R10G10B10_UINT_PACK32:
			return "A2R10G10B10_UINT_PACK32";
		case VK_FORMAT_A2R10G10B10_SINT_PACK32:
			return "A2R10G10B10_SINT_PACK32";
		case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
			return "A2B10G10R10_UNORM_PACK32";
		case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
			return "A2B10G10R10_SNORM_PACK32";
		case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
			return "A2B10G10R10_USCALED_PACK32";
		case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
			return "A2B10G10R10_SSCALED_PACK32";
		case VK_FORMAT_A2B10G10R10_UINT_PACK32:
			return "A2B10G10R10_UINT_PACK32";
		case VK_FORMAT_A2B10G10R10_SINT_PACK32:
			return "A2B10G10R10_SINT_PACK32";
		case VK_FORMAT_R16_UNORM:
			return "R16_UNORM";
		case VK_FORMAT_R16_SNORM:
			return "R16_SNORM";
		case VK_FORMAT_R16_USCALED:
			return "R16_USCALED";
		case VK_FORMAT_R16_SSCALED:
			return "R16_SSCALED";
		case VK_FORMAT_R16_UINT:
			return "R16_UINT";
		case VK_FORMAT_R16_SINT:
			return "R16_SINT";
		case VK_FORMAT_R16_SFLOAT:
			return "R16_SFLOAT";
		case VK_FORMAT_R16G16_UNORM:
			return "R16G16_UNORM";
		case VK_FORMAT_R16G16_SNORM:
			return "R16G16_SNORM";
		case VK_FORMAT_R16G16_USCALED:
			return "R16G16_USCALED";
		case VK_FORMAT_R16G16_SSCALED:
			return "R16G16_SSCALED";
		case VK_FORMAT_R16G16_UINT:
			return "R16G16_UINT";
		case VK_FORMAT_R16G16_SINT:
			return "R16G16_SINT";
		case VK_FORMAT_R16G16_SFLOAT:
			return "R16G16_SFLOAT";
		case VK_FORMAT_R16G16B16_UNORM:
			return "R16G16B16_UNORM";
		case VK_FORMAT_R16G16B16_SNORM:
			return "R16G16B16_SNORM";
		case VK_FORMAT_R16G16B16_USCALED:
			return "R16G16B16_USCALED";
		case VK_FORMAT_R16G16B16_SSCALED:
			return "R16G16B16_SSCALED";
		case VK_FORMAT_R16G16B16_UINT:
			return "R16G16B16_UINT";
		case VK_FORMAT_R16G16B16_SINT:
			return "R16G16B16_SINT";
		case VK_FORMAT_R16G16B16_SFLOAT:
			return "R16G16B16_SFLOAT";
		case VK_FORMAT_R16G16B16A16_UNORM:
			return "R16G16B16A16_UNORM";
		case VK_FORMAT_R16G16B16A16_SNORM:
			return "R16G16B16A16_SNORM";
		case VK_FORMAT_R16G16B16A16_USCALED:
			return "R16G16B16A16_USCALED";
		case VK_FORMAT_R16G16B16A16_SSCALED:
			return "R16G16B16A16_SSCALED";
		case VK_FORMAT_R16G16B16A16_UINT:
			return "R16G16B16A16_UINT";
		case VK_FORMAT_R16G16B16A16_SINT:
			return "R16G16B16A16_SINT";
		case VK_FORMAT_R16G16B16A16_SFLOAT:
			return "R16G16B16A16_SFLOAT";
		case VK_FORMAT_R32_UINT:
			return "R32_UINT";
		case VK_FORMAT_R32_SINT:
			return "R32_SINT";
		case VK_FORMAT_R32_SFLOAT:
			return "R32_SFLOAT";
		case VK_FORMAT_R32G32_UINT:
			return "R32G32_UINT";
		case VK_FORMAT_R32G32_SINT:
			return "R32G32_SINT";
		case VK_FORMAT_R32G32_SFLOAT:
			return "R32G32_SFLOAT";
		case VK_FORMAT_R32G32B32_UINT:
			return "R32G32B32_UINT";
		case VK_FORMAT_R32G32B32_SINT:
			return "R32G32B32_SINT";
		case VK_FORMAT_R32G32B32_SFLOAT:
			return "R32G32B32_SFLOAT";
		case VK_FORMAT_R32G32B32A32_UINT:
			return "R32G32B32A32_UINT";
		case VK_FORMAT_R32G32B32A32_SINT:
			return "R32G32B32A32_SINT";
		case VK_FORMAT_R32G32B32A32_SFLOAT:
			return "R32G32B32A32_SFLOAT";
		case VK_FORMAT_R64_UINT:
			return "R64_UINT";
		case VK_FORMAT_R64_SINT:
			return "R64_SINT";
		case VK_FORMAT_R64_SFLOAT:
			return "R64_SFLOAT";
		case VK_FORMAT_R64G64_UINT:
			return "R64G64_UINT";
		case VK_FORMAT_R64G64_SINT:
			return "R64G64_SINT";
		case VK_FORMAT_R64G64_SFLOAT:
			return "R64G64_SFLOAT";
		case VK_FORMAT_R64G64B64_UINT:
			return "R64G64B64_UINT";
		case VK_FORMAT_R64G64B64_SINT:
			return "R64G64B64_SINT";
		case VK_FORMAT_R64G64B64_SFLOAT:
			return "R64G64B64_SFLOAT";
		case VK_FORMAT_R64G64B64A64_UINT:
			return "R64G64B64A64_UINT";
		case VK_FORMAT_R64G64B64A64_SINT:
			return "R64G64B64A64_SINT";
		case VK_FORMAT_R64G64B64A64_SFLOAT:
			return "R64G64B64A64_SFLOAT";
		case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
			return "B10G11R11_UFLOAT_PACK32";
		case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
			return "E5B9G9R9_UFLOAT_PACK32";
		case VK_FORMAT_D16_UNORM:
			return "D16_UNORM";
		case VK_FORMAT_X8_D24_UNORM_PACK32:
			return "X8_D24_UNORM_PACK32";
		case VK_FORMAT_D32_SFLOAT:
			return "D32_SFLOAT";
		case VK_FORMAT_S8_UINT:
			return "S8_UINT";
		case VK_FORMAT_D16_UNORM_S8_UINT:
			return "D16_UNORM_S8_UINT";
		case VK_FORMAT_D24_UNORM_S8_UINT:
			return "D24_UNORM_S8_UINT";
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return "D32_SFLOAT_S8_UINT";
		case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
			return "BC1_RGB_UNORM_BLOCK";
		case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
			return "BC1_RGB_SRGB_BLOCK";
		case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
			return "BC1_RGBA_UNORM_BLOCK";
		case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
			return "BC1_RGBA_SRGB_BLOCK";
		case VK_FORMAT_BC2_UNORM_BLOCK:
			return "BC2_UNORM_BLOCK";
		case VK_FORMAT_BC2_SRGB_BLOCK:
			return "BC2_SRGB_BLOCK";
		case VK_FORMAT_BC3_UNORM_BLOCK:
			return "BC3_UNORM_BLOCK";
		case VK_FORMAT_BC3_SRGB_BLOCK:
			return "BC3_SRGB_BLOCK";
		case VK_FORMAT_BC4_UNORM_BLOCK:
			return "BC4_UNORM_BLOCK";
		case VK_FORMAT_BC4_SNORM_BLOCK:
			return "BC4_SNORM_BLOCK";
		case VK_FORMAT_BC5_UNORM_BLOCK:
			return "BC5_UNORM_BLOCK";
		case VK_FORMAT_BC5_SNORM_BLOCK:
			return "BC5_SNORM_BLOCK";
		case VK_FORMAT_BC6H_UFLOAT_BLOCK:
			return "BC6H_UFLOAT_BLOCK";
		case VK_FORMAT_BC6H_SFLOAT_BLOCK:
			return "BC6H_SFLOAT_BLOCK";
		case VK_FORMAT_BC7_UNORM_BLOCK:
			return "BC7_UNORM_BLOCK";
		case VK_FORMAT_BC7_SRGB_BLOCK:
			return "BC7_SRGB_BLOCK";
		case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
			return "ETC2_R8G8B8_UNORM_BLOCK";
		case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
			return "ETC2_R8G8B8_SRGB_BLOCK";
		case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
			return "ETC2_R8G8B8A1_UNORM_BLOCK";
		case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
			return "ETC2_R8G8B8A1_SRGB_BLOCK";
		case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
			return "ETC2_R8G8B8A8_UNORM_BLOCK";
		case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
			return "ETC2_R8G8B8A8_SRGB_BLOCK";
		case VK_FORMAT_EAC_R11_UNORM_BLOCK:
			return "EAC_R11_UNORM_BLOCK";
		case VK_FORMAT_EAC_R11_SNORM_BLOCK:
			return "EAC_R11_SNORM_BLOCK";
		case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
			return "EAC_R11G11_UNORM_BLOCK";
		case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
			return "EAC_R11G11_SNORM_BLOCK";
		case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
			return "ASTC_4x4_UNORM_BLOCK";
		case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
			return "ASTC_4x4_SRGB_BLOCK";
		case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
			return "ASTC_5x4_UNORM_BLOCK";
		case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
			return "ASTC_5x4_SRGB_BLOCK";
		case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
			return "ASTC_5x5_UNORM_BLOCK";
		case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
			return "ASTC_5x5_SRGB_BLOCK";
		case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
			return "ASTC_6x5_UNORM_BLOCK";
		case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
			return "ASTC_6x5_SRGB_BLOCK";
		case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
			return "ASTC_6x6_UNORM_BLOCK";
		case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
			return "ASTC_6x6_SRGB_BLOCK";
		case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
			return "ASTC_8x5_UNORM_BLOCK";
		case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
			return "ASTC_8x5_SRGB_BLOCK";
		case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
			return "ASTC_8x6_UNORM_BLOCK";
		case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
			return "ASTC_8x6_SRGB_BLOCK";
		case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
			return "ASTC_8x8_UNORM_BLOCK";
		case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
			return "ASTC_8x8_SRGB_BLOCK";
		case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
			return "ASTC_10x5_UNORM_BLOCK";
		case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
			return "ASTC_10x5_SRGB_BLOCK";
		case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
			return "ASTC_10x6_UNORM_BLOCK";
		case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
			return "ASTC_10x6_SRGB_BLOCK";
		case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
			return "ASTC_10x8_UNORM_BLOCK";
		case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
			return "ASTC_10x8_SRGB_BLOCK";
		case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
			return "ASTC_10x10_UNORM_BLOCK";
		case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
			return "ASTC_10x10_SRGB_BLOCK";
		case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
			return "ASTC_12x10_UNORM_BLOCK";
		case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
			return "ASTC_12x10_SRGB_BLOCK";
		case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
			return "ASTC_12x12_UNORM_BLOCK";
		case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
			return "ASTC_12x12_SRGB_BLOCK";
		case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
			return "PVRTC1_2BPP_UNORM_BLOCK_IMG";
		case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
			return "PVRTC1_4BPP_UNORM_BLOCK_IMG";
		case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
			return "PVRTC2_2BPP_UNORM_BLOCK_IMG";
		case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
			return "PVRTC2_4BPP_UNORM_BLOCK_IMG";
		case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
			return "PVRTC1_2BPP_SRGB_BLOCK_IMG";
		case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
			return "PVRTC1_4BPP_SRGB_BLOCK_IMG";
		case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
			return "PVRTC2_2BPP_SRGB_BLOCK_IMG";
		case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
			return "PVRTC2_4BPP_SRGB_BLOCK_IMG";
		default:
			return "UNKNOWN";
	}
}

const char *VkPresentModeToString(const VkPresentModeKHR presentMode)
{
	switch (presentMode)
	{
		case VK_PRESENT_MODE_IMMEDIATE_KHR:
			return "Immediate";
		case VK_PRESENT_MODE_MAILBOX_KHR:
			return "Mailbox";
		case VK_PRESENT_MODE_FIFO_KHR:
			return "FIFO";
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
			return "Relaxed";
		default:
			return "Unknown";
	}
}

VkBool32 VKAPI_PTR VulkanDebugMessageCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
	void *userData)
{
	const char *color = "";
	switch (messageSeverity)
	{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			color = S_COLOR_CYAN;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			color = S_COLOR_WHITE;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			color = S_COLOR_YELLOW;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			color = S_COLOR_RED;
			break;
		default:
			break;
	}

	char reason[64] = {};
	if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
		Q_strcat(reason, sizeof(reason), "[General]");

	if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
		Q_strcat(reason, sizeof(reason), "[Validation]");

	if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
		Q_strcat(reason, sizeof(reason), "[Performance]");

	Com_Printf(
		"%s[VULKAN DEBUG]%s[%s]: %s\n",
		color,
		reason,
		callbackData->pMessageIdName,
		callbackData->pMessage);

	return VK_FALSE;
}

VkPresentModeKHR PickBestPresentMode(
	const std::vector<VkPresentModeKHR>& presentModes)
{
	bool mailboxAvailable = false;
	bool fifoRelaxedAvailable = false;
	for (auto mode : presentModes)
	{
		switch (mode)
		{
			case VK_PRESENT_MODE_MAILBOX_KHR:
				mailboxAvailable = true;
				break;
			case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
				fifoRelaxedAvailable = true;
				break;
			default:
				break;
		}
	}

	// Immediate and FIFO are most similar to the non-vsync and vsync settings
	// in the original renderer. We also give the option to choose from FIFO
	// Relaxed and Mailbox modes too.
	switch (ri.Cvar_VariableIntegerValue("r_swapInterval"))
	{
		case 1:
			return VK_PRESENT_MODE_FIFO_KHR;
		case 2:
			if (fifoRelaxedAvailable)
			{
				return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
			}
		// fall-through
		case 3:
			if (mailboxAvailable)
			{
				return VK_PRESENT_MODE_MAILBOX_KHR;
			}
		// fall-through
		default:
			return VK_PRESENT_MODE_IMMEDIATE_KHR;
	}
}

void ConfigureRenderer(
	glconfig_t& config,
	VkPhysicalDeviceFeatures& requiredFeatures,
	const VkPhysicalDeviceFeatures& features,
	const VkPhysicalDeviceProperties& properties)
{
	requiredFeatures = {};

	Com_Printf("Configuring physical device features\n");

	// Extensions from original renderer that we have by default in Vulkan
	config.textureEnvAddAvailable = qtrue;

	const VkPhysicalDeviceLimits& limits = properties.limits;
	config.maxTextureSize = limits.maxImageDimension2D;

	Com_Printf("...Max 2D image dimension = %d\n", config.maxTextureSize);

	if (features.samplerAnisotropy)
	{
		Com_Printf("...Sampler anisotropy available\n");
		config.maxTextureFilterAnisotropy = limits.maxSamplerAnisotropy;

		ri.Cvar_SetValue(
			"r_ext_texture_filter_anisotropic_avail",
			config.maxTextureFilterAnisotropy);
		if (r_ext_texture_filter_anisotropic->value >
				config.maxTextureFilterAnisotropy)
		{
			ri.Cvar_SetValue(
				"r_ext_texture_filter_anisotropic_avail",
				config.maxTextureFilterAnisotropy);
		}

		requiredFeatures.samplerAnisotropy = VK_TRUE;
	}
	else
	{
		Com_Printf ("...Sampler anisotropy is not available\n");
		ri.Cvar_Set("r_ext_texture_filter_anisotropic_avail", "0");
	}
}

void ReportInstanceLayers()
{
	uint32_t maxSupportedDriverApiVersion = 0;
	vkEnumerateInstanceVersion(&maxSupportedDriverApiVersion);

	Com_Printf(
		"Max Driver Vulkan Version: %d.%d.%d\n",
		VK_VERSION_MAJOR(maxSupportedDriverApiVersion),
		VK_VERSION_MINOR(maxSupportedDriverApiVersion),
		VK_VERSION_PATCH(maxSupportedDriverApiVersion));

	uint32_t instanceLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);

	std::vector<VkLayerProperties> instanceLayers(instanceLayerCount);
	vkEnumerateInstanceLayerProperties(
		&instanceLayerCount, instanceLayers.data());

	Com_Printf("%d instance layers available\n", instanceLayerCount);
	if (instanceLayerCount > 0)
	{
		Com_Printf("...");
		for (const auto& layer : instanceLayers)
		{
			Com_Printf("%s ", layer.layerName);
		}
		Com_Printf("\n");
	}
}

void ReportInstanceExtensions()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(
		nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(
		nullptr, &extensionCount, extensions.data());

	Com_Printf("%d instance extensions available\n", extensionCount);
	if (extensionCount > 0)
	{
		Com_Printf("...");
		for (const auto& extension : extensions)
		{
			Com_Printf("%s ", extension.extensionName);
		}
		Com_Printf("\n");
	}
}

void SetDebugUtilsMessengerDefaults(
	VkDebugUtilsMessengerCreateInfoEXT& debugUtilsMessengerCreateInfo)
{
	debugUtilsMessengerCreateInfo.sType =
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugUtilsMessengerCreateInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugUtilsMessengerCreateInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugUtilsMessengerCreateInfo.pfnUserCallback = VulkanDebugMessageCallback;
}

VkInstance CreateInstance()
{
	uint32_t requiredExtensionCount = 0;
	ri.VK_GetInstanceExtensions(&requiredExtensionCount, nullptr);

	std::vector<const char *> requiredLayers;
	if (r_debugApi->integer)
	{
		requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
	}

	std::vector<const char *> requiredExtensions(requiredExtensionCount);
	ri.VK_GetInstanceExtensions(
		&requiredExtensionCount, requiredExtensions.data());
	if (r_debugApi->integer)
	{
		requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	Com_Printf(
		"%d instance extensions will be used\n", requiredExtensionCount);
	for (const auto& extension : requiredExtensions)
	{
		Com_Printf("...%s\n", extension);
	}

	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = "Strawberry Renderer";
	applicationInfo.apiVersion = VK_API_VERSION_1_0;
	applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	applicationInfo.pEngineName = "OpenJK";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledExtensionCount = requiredExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();
	instanceCreateInfo.enabledLayerCount = requiredLayers.size();
	instanceCreateInfo.ppEnabledLayerNames = requiredLayers.data();

	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {};
	if (r_debugApi->integer)
	{
		SetDebugUtilsMessengerDefaults(debugUtilsMessengerCreateInfo);
		instanceCreateInfo.pNext = &debugUtilsMessengerCreateInfo;
	}

	VkInstance instance;
	VkResult result = vkCreateInstance(
		&instanceCreateInfo, nullptr, &instance);
	if (result != VK_SUCCESS)
	{
		Com_Error(ERR_FATAL, "Failed to create Vulkan instance\n");
	}

	return instance;
}

void CreateDescriptorSetLayouts(GpuContext& context)
{
	// single texture
	{
		VkDescriptorSetLayoutBinding bindings[1] = {};
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = {};
		setLayoutCreateInfo.sType =
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setLayoutCreateInfo.bindingCount = 1;
		setLayoutCreateInfo.pBindings = bindings;

		if (vkCreateDescriptorSetLayout(
				context.device,
				&setLayoutCreateInfo,
				nullptr,
				&context.descriptorSetLayouts[DESCRIPTOR_SET_SINGLE_TEXTURE]) != VK_SUCCESS)
		{
			Com_Error(
				ERR_FATAL,
				"Failed to create single-texture descriptor set layout");
		}
	}
	
	{
		VkDescriptorSetLayoutBinding bindings[2] = {};
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[1].binding = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[1].descriptorCount = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = {};
		setLayoutCreateInfo.sType =
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setLayoutCreateInfo.bindingCount = 2;
		setLayoutCreateInfo.pBindings = bindings;

		if (vkCreateDescriptorSetLayout(
				context.device,
				&setLayoutCreateInfo,
				nullptr,
				&context.descriptorSetLayouts[DESCRIPTOR_SET_MULTI_TEXTURE]) != VK_SUCCESS)
		{
			Com_Error(
				ERR_FATAL,
				"Failed to create multi-texture descriptor set layout");
		}
	}
}

void CreatePipelineLayouts(GpuContext& context)
{
	VkPushConstantRange pushConstants = {};
	pushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstants.offset = 0;
	pushConstants.size = sizeof(gpuMatrices_t);

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; 
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstants;

	for (int i = 0; i < DESCRIPTOR_SET_COUNT; ++i)
	{
		VkDescriptorSetLayout layout = context.descriptorSetLayouts[i];
		pipelineLayoutCreateInfo.pSetLayouts = &layout;

		if (vkCreatePipelineLayout(
				context.device,
				&pipelineLayoutCreateInfo,
				nullptr,
				&context.pipelineLayouts[i]) != VK_SUCCESS)
		{
			Com_Error(ERR_FATAL, "Failed to create pipeline layout");
		}
	}
}

VkCullModeFlags GetVkCullMode(uint32_t stateBits2)
{
	switch (stateBits2 & GLS2_CULLMODE_BITS)
	{
		case GLS2_CULLMODE_FRONT: return VK_CULL_MODE_BACK_BIT;
		case GLS2_CULLMODE_BACK: return VK_CULL_MODE_FRONT_BIT;
		case GLS2_CULLMODE_NONE: return VK_CULL_MODE_NONE;
        default: return VK_CULL_MODE_FRONT_BIT;
    }
}

VkBlendFactor GetVkSrcBlendFactor(uint32_t stateBits)
{
	switch (stateBits & GLS_SRCBLEND_BITS)
	{
		case GLS_SRCBLEND_ZERO:
			return VK_BLEND_FACTOR_ZERO;
		case GLS_SRCBLEND_ONE:
			return VK_BLEND_FACTOR_ONE;
		case GLS_SRCBLEND_DST_COLOR:
			return VK_BLEND_FACTOR_DST_COLOR;
		case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case GLS_SRCBLEND_SRC_ALPHA:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case GLS_SRCBLEND_DST_ALPHA:
			return VK_BLEND_FACTOR_DST_ALPHA;
		case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case GLS_SRCBLEND_ALPHA_SATURATE:
			return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		default:
			return VK_BLEND_FACTOR_ONE;
	}
}

VkBlendFactor GetVkDstBlendFactor(uint32_t stateBits)
{
	switch ( stateBits & GLS_DSTBLEND_BITS )
	{
		case GLS_DSTBLEND_ZERO:
			return VK_BLEND_FACTOR_ZERO;
		case GLS_DSTBLEND_ONE:
			return VK_BLEND_FACTOR_ONE;
		case GLS_DSTBLEND_SRC_COLOR:
			return VK_BLEND_FACTOR_SRC_COLOR;
		case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case GLS_DSTBLEND_SRC_ALPHA:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case GLS_DSTBLEND_DST_ALPHA:
			return VK_BLEND_FACTOR_DST_ALPHA;
		case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		default:
			return VK_BLEND_FACTOR_ZERO;
	}
}

VkFormat PickDepthStencilFormat(VkPhysicalDevice physicalDevice)
{
	const std::array<VkFormat, 2> depthStencilFormatCandidates = {
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT_S8_UINT
	};

	for (auto format : depthStencilFormatCandidates)
	{
		VkFormatProperties formatProperties = {};
		vkGetPhysicalDeviceFormatProperties(
			physicalDevice,
			format,
			&formatProperties);

		if (formatProperties.optimalTilingFeatures &
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			return format;
		}
	}

	return VK_FORMAT_UNDEFINED;
}
}

bool operator==(const RenderState& lhs, const RenderState& rhs)
{
    return (
        (lhs.vertexShader == rhs.vertexShader) &&
        (lhs.fragmentShader == rhs.fragmentShader) &&
        (lhs.vertexAttributes == rhs.vertexAttributes) &&
        (lhs.vertexAttributeCount == rhs.vertexAttributeCount) &&
        (lhs.stateBits == rhs.stateBits) &&
        (lhs.stateBits2 == rhs.stateBits2)
    );
}

void GpuContextInit(GpuContext& context)
{
	ReportInstanceLayers();

	ReportInstanceExtensions();

	gpuContext.instance = CreateInstance();

	//
	// debug utils
	//
	if (r_debugApi->integer)
	{
		auto vkCreateDebugUtilsMessengerEXT =
			(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
				context.instance, "vkCreateDebugUtilsMessengerEXT");

		VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo;
		SetDebugUtilsMessengerDefaults(debugUtilsMessengerCreateInfo);
		if (vkCreateDebugUtilsMessengerEXT(
				context.instance,
				&debugUtilsMessengerCreateInfo,
				nullptr,
				&context.debugUtilsMessenger) != VK_SUCCESS)
		{
			Com_Printf(
				S_COLOR_YELLOW
				"Unable to initialise Vulkan debug utils\n");
		}
	}

	//
	// physical devices
	//
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(
		context.instance, &physicalDeviceCount, nullptr);

	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(
		context.instance, &physicalDeviceCount, physicalDevices.data());

	std::vector<VkPhysicalDeviceProperties> physicalDeviceProperties;
	physicalDeviceProperties.reserve(physicalDeviceCount);

	Com_Printf("%d GPU physical devices found\n", physicalDeviceCount);
	for (const auto& physicalDevice : physicalDevices)
	{
		VkPhysicalDeviceProperties properties = {};
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		physicalDeviceProperties.push_back(properties);

		const char *deviceType = nullptr;
		switch (properties.deviceType)
		{
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				deviceType = "Integrated GPU";
				break;
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				deviceType = "Discrete GPU";
				break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				deviceType = "Virtual GPU";
				break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				deviceType = "CPU";
				break;
			default:
				deviceType = "Unknown";
				break;
		}

		Com_Printf(
			"...%s '%s', "
			"version id 0x%08x, "
			"vendor id 0x%08x, "
			"driver version %d, "
			"max supported Vulkan version %d.%d.%d\n",
			deviceType,
			properties.deviceName,
			properties.deviceID,
			properties.vendorID,
			properties.driverVersion,
			VK_VERSION_MAJOR(properties.apiVersion),
			VK_VERSION_MINOR(properties.apiVersion),
			VK_VERSION_PATCH(properties.apiVersion));
	}

	const std::vector<const char *> requiredDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	const int bestPhysicalDeviceIndex =
		PickPhysicalDeviceIndex(physicalDevices, requiredDeviceExtensions);
	context.physicalDevice = physicalDevices[bestPhysicalDeviceIndex];
	Q_strncpyz(
		context.physicalDeviceName,
		physicalDeviceProperties[bestPhysicalDeviceIndex].deviceName,
		sizeof(context.physicalDeviceName));

	Com_Printf(
		"Best physical device: %s\n", context.physicalDeviceName);

	VkPhysicalDeviceFeatures deviceFeatures = {};
	vkGetPhysicalDeviceFeatures(
		context.physicalDevice, &deviceFeatures);

	VkPhysicalDeviceFeatures enableDeviceFeatures = {};
	ConfigureRenderer(
		glConfig,
		enableDeviceFeatures,
		deviceFeatures,
		physicalDeviceProperties[bestPhysicalDeviceIndex]);

	//
	// physical device extensions
	//
	uint32_t deviceExtensionCount = 0;
	vkEnumerateDeviceExtensionProperties(
		context.physicalDevice,
		nullptr,
		&deviceExtensionCount,
		nullptr);

	std::vector<VkExtensionProperties> deviceExtensions(
		deviceExtensionCount);
	vkEnumerateDeviceExtensionProperties(
		context.physicalDevice,
		nullptr,
		&deviceExtensionCount,
		deviceExtensions.data());

	Com_Printf("%d device extensions available\n", deviceExtensionCount);
	if (deviceExtensionCount > 0)
	{
		Com_Printf("...");
		for (const auto& extension : deviceExtensions)
		{
			Com_Printf("%s ", extension.extensionName);
		}
		Com_Printf("\n");
	}

	//
	// window surface
	//
	if (!ri.VK_CreateWindowSurface(
			context.instance,
			(void **)&context.windowSurface))
	{
		Com_Error(ERR_FATAL, "Failed to create window surface");
	}

	//
	// queue families
	//
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(
		context.physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyProperties(
		queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(
		context.physicalDevice,
		&queueFamilyCount,
		queueFamilyProperties.data());

	Com_Printf(
		"%d queue families available for '%s'\n",
		queueFamilyCount,
		context.physicalDeviceName);
	for (int i = 0; i < queueFamilyCount; ++i)
	{
		const auto& properties = queueFamilyProperties[i];

		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			context.physicalDevice,
			i,
			context.windowSurface,
			&presentSupport);

		char queueCaps[64] = {};
		if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			Q_strcat(queueCaps, sizeof(queueCaps), "graphics ");

		if (properties.queueFlags & VK_QUEUE_COMPUTE_BIT)
			Q_strcat(queueCaps, sizeof(queueCaps), "compute ");

		if (properties.queueFlags & VK_QUEUE_TRANSFER_BIT)
			Q_strcat(queueCaps, sizeof(queueCaps), "transfer ");

		if (presentSupport)
			Q_strcat(queueCaps, sizeof(queueCaps), "present ");

		Com_Printf(
			"...%d: %d queues, supports %s\n",
			i,
			properties.queueCount,
			queueCaps);
	}

	for (int i = 0; i < queueFamilyCount; ++i)
	{
		const auto& properties = queueFamilyProperties[i];

		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			context.physicalDevice,
			i,
			context.windowSurface,
			&presentSupport);

		if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			context.graphicsQueue.queueFamily = i;
		}

		if (properties.queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			context.computeQueue.queueFamily = i;
		}

		if (properties.queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			context.transferQueue.queueFamily = i;
		}

		if (presentSupport)
		{
			context.presentQueue.queueFamily = i;
		}

		if (context.graphicsQueue.queueFamily != -1 &&
			context.computeQueue.queueFamily != -1 &&
			context.transferQueue.queueFamily != -1 &&
			context.presentQueue.queueFamily != 1)
		{
			break;
		}
	}

	//
	// logical device
	//
	const std::set<uint32_t> queueFamilies = {
		context.graphicsQueue.queueFamily,
		context.computeQueue.queueFamily,
		context.transferQueue.queueFamily,
		context.presentQueue.queueFamily
	};

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.reserve(queueFamilies.size());

	const float queuePriority = 1.0f;
	for (const auto queueFamilyIndex : queueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.emplace_back(queueCreateInfo);
	}

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
	deviceCreateInfo.pEnabledFeatures = &enableDeviceFeatures;
	deviceCreateInfo.ppEnabledExtensionNames =
		requiredDeviceExtensions.data();
	deviceCreateInfo.enabledExtensionCount =
		requiredDeviceExtensions.size();

	if (vkCreateDevice(
			context.physicalDevice,
			&deviceCreateInfo,
			nullptr,
			&context.device) != VK_SUCCESS)
	{
		Com_Error(ERR_FATAL, "Failed to create logical device");
	}

	InitializeDeviceQueue(
		context.device, context.graphicsQueue);
	InitializeDeviceQueue(
		context.device, context.computeQueue);
	InitializeDeviceQueue(
		context.device, context.transferQueue);
	InitializeDeviceQueue(
		context.device, context.presentQueue);

	Com_Printf("Vulkan context initialized\n");

	//
	// memory allocator
	//
	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.physicalDevice = context.physicalDevice;
	allocatorCreateInfo.device = context.device;

	vmaCreateAllocator(&allocatorCreateInfo, &context.allocator);

	//
	// swap chain
	//
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		context.physicalDevice,
		context.windowSurface,
		&surfaceCapabilities);

	uint32_t swapchainImageCount = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0)
	{
		swapchainImageCount = std::max(
			swapchainImageCount,
			surfaceCapabilities.maxImageCount);
	}


	uint32_t surfaceFormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		context.physicalDevice,
		context.windowSurface,
		&surfaceFormatCount,
		nullptr);

	std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		context.physicalDevice,
		context.windowSurface,
		&surfaceFormatCount,
		surfaceFormats.data());

	Com_Printf(
		"%d window surface formats available\n", surfaceFormatCount);
	if (surfaceFormatCount > 0)
	{
		Com_Printf("...");
		for (const auto& format : surfaceFormats)
		{
			Com_Printf("%s ", VkFormatToString(format.format));
		}
		Com_Printf("\n");
	}

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		context.physicalDevice,
		context.windowSurface,
		&presentModeCount,
		nullptr);

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		context.physicalDevice,
		context.windowSurface,
		&presentModeCount,
		presentModes.data());

	Com_Printf(
		"%d window present modes available\n", presentModeCount);
	if (presentModeCount > 0)
	{
		Com_Printf("...");
		for (const auto& mode : presentModes)
		{
			Com_Printf("%s ", VkPresentModeToString(mode));
		}
		Com_Printf("\n");
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType =
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = context.windowSurface;
	swapchainCreateInfo.minImageCount = swapchainImageCount;
	swapchainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	swapchainCreateInfo.imageColorSpace =
		VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage =
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
		VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;

	const std::array<uint32_t, 2> swapchainQueueFamilyIndices = {
		context.graphicsQueue.queueFamily,
		context.presentQueue.queueFamily
	};

	if (context.graphicsQueue.queueFamily ==
		context.presentQueue.queueFamily)
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	else
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount =
			swapchainQueueFamilyIndices.size();
		swapchainCreateInfo.pQueueFamilyIndices =
			swapchainQueueFamilyIndices.data();
	}
	swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = PickBestPresentMode(presentModes);
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(
			context.device,
			&swapchainCreateInfo,
			nullptr,
			&context.swapchain.swapchain) != VK_SUCCESS)
	{
		Com_Error(ERR_FATAL, "Failed to create swapchain");
	}

	context.swapchain.surfaceFormat = VK_FORMAT_B8G8R8A8_UNORM;
	context.swapchain.width = surfaceCapabilities.currentExtent.width;
	context.swapchain.height = surfaceCapabilities.currentExtent.height;

	context.depthStencilFormat =
		PickDepthStencilFormat(context.physicalDevice);
	if (context.depthStencilFormat == VK_FORMAT_UNDEFINED)
	{
		Com_Error(ERR_FATAL, "Failed to find valid depth/stencil format");
	}
	else
	{
		Com_Printf(
			"Using %s for depth-stencil format\n",
			VkFormatToString(context.depthStencilFormat));
	}

	//
	// Render pass
	//
	std::array<VkAttachmentDescription, 2> attachments = {};
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].format = context.swapchain.surfaceFormat;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].format = context.depthStencilFormat;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthStencilAttachmentRef = {};
	depthStencilAttachmentRef.attachment = 1;
	depthStencilAttachmentRef.layout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentRef;
	subpassDescription.pDepthStencilAttachment = &depthStencilAttachmentRef;

	VkSubpassDependency subpassDependency = {};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstAccessMask =
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = attachments.size();
	renderPassCreateInfo.pAttachments = attachments.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &subpassDependency;

	if (vkCreateRenderPass(
			context.device,
			&renderPassCreateInfo,
			nullptr,
			&context.renderPass) != VK_SUCCESS)
	{
		Com_Printf(S_COLOR_RED "Failed to create render pass\n");
	}

	//
	// global descriptor pool
	//
	{
		VkDescriptorPoolSize poolSizes[1] = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[0].descriptorCount = 4096;

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
		descriptorPoolCreateInfo.sType =
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.maxSets = 4096;
		descriptorPoolCreateInfo.poolSizeCount = 1;
		descriptorPoolCreateInfo.pPoolSizes = poolSizes;

		if (vkCreateDescriptorPool(
				gpuContext.device,
				&descriptorPoolCreateInfo,
				nullptr,
				&gpuContext.globalDescriptorPool) != VK_SUCCESS)
		{
			Com_Error(ERR_FATAL, "Failed to create global descriptor pool");
		}
	}

	//
	// command buffer pool
	//
	{
		VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
		cmdPoolCreateInfo.sType =
			VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolCreateInfo.flags =
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		cmdPoolCreateInfo.queueFamilyIndex =
			context.transferQueue.queueFamily;

		if (vkCreateCommandPool(
				context.device,
				&cmdPoolCreateInfo,
				nullptr,
				&context.transferCommandPool) != VK_SUCCESS)
		{
			Com_Error(ERR_FATAL, "Failed to create transfer command pool");
		}
	}

	{
		VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
		cmdPoolCreateInfo.sType =
			VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolCreateInfo.flags =
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		cmdPoolCreateInfo.queueFamilyIndex =
			context.graphicsQueue.queueFamily;

		if (vkCreateCommandPool(
				context.device,
				&cmdPoolCreateInfo,
				nullptr,
				&context.gfxCommandPool) != VK_SUCCESS)
		{
			Com_Error(ERR_FATAL, "Failed to create graphics command pool");
		}
	}

	//
	// swapchain resources
	//
	swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(
		context.device,
		context.swapchain.swapchain,
		&swapchainImageCount,
		nullptr);

	std::vector<VkImage> swapchainImages(swapchainImageCount);
	vkGetSwapchainImagesKHR(
		context.device,
		context.swapchain.swapchain,
		&swapchainImageCount,
		swapchainImages.data());

	// STRAWB: Organise this better? Each render pass should maybe have its own
	// struct containing the images it contains and not be initialized in
	// here...
	context.depthImage = CreateImageHandle(
		context.allocator,
		surfaceCapabilities.currentExtent.width,
		surfaceCapabilities.currentExtent.height,
		context.depthStencilFormat,
		1,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		context.depthImageAllocation);
	context.depthImageView = CreateImageView(
		context.depthImage,
		context.depthStencilFormat,
		VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
		1);

	auto& swapchainResources = context.swapchain.resources;
	swapchainResources.resize(swapchainImageCount);

	for (uint32_t i = 0; i < swapchainImageCount; ++i)
	{
		GpuSwapchainResources& resources = swapchainResources[i];

		resources.image = swapchainImages[i];
		resources.imageView = CreateImageView(
			swapchainImages[i], 
			context.swapchain.surfaceFormat,
			VK_IMAGE_ASPECT_COLOR_BIT,
			1);

		// framebuffer
		std::array<VkImageView, 2> attachments = {
			resources.imageView,
			context.depthImageView
		};

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType =
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO; 
		framebufferCreateInfo.renderPass = context.renderPass;
		framebufferCreateInfo.attachmentCount = attachments.size();
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width =
			surfaceCapabilities.currentExtent.width;
		framebufferCreateInfo.height =
			surfaceCapabilities.currentExtent.height;
		framebufferCreateInfo.layers = 1;

		if (vkCreateFramebuffer(
				context.device,
				&framebufferCreateInfo,
				nullptr,
				&resources.framebuffer) != VK_SUCCESS)
		{
			Com_Printf(S_COLOR_RED "Failed to create framebuffer\n");
		}

		// graphics command buffer
		VkCommandBufferAllocateInfo cmdBufferAllocateInfo = {};
		cmdBufferAllocateInfo.sType =
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocateInfo.commandPool = context.gfxCommandPool;
		cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAllocateInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(
				context.device,
				&cmdBufferAllocateInfo,
				&resources.gfxCommandBuffer) != VK_SUCCESS)
		{
			Com_Printf(S_COLOR_RED "Failed to allocate command buffer\n");
			return;
		}

		// Descriptor pool
		VkDescriptorPoolSize poolSizes[1] = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[0].descriptorCount = 2048;

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
		descriptorPoolCreateInfo.sType =
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.maxSets = 2048;
		descriptorPoolCreateInfo.poolSizeCount = 1;
		descriptorPoolCreateInfo.pPoolSizes = poolSizes;

		if (vkCreateDescriptorPool(
				gpuContext.device,
				&descriptorPoolCreateInfo,
				nullptr,
				&resources.descriptorPool) != VK_SUCCESS)
		{
			Com_Error(ERR_FATAL, "Failed to create descriptor pool");
		}

		VkBufferCreateInfo indexBufferCreateInfo = {};
		indexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		indexBufferCreateInfo.size = 1 * 1024 * 1024;
		indexBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		VkBufferCreateInfo vertexBufferCreateInfo = {};
		vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferCreateInfo.size = 4 * 1024 * 1024;
		vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

		VmaAllocationCreateInfo bufferAllocCreateInfo = {};
		bufferAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		if (vmaCreateBuffer(
				context.allocator, 
				&indexBufferCreateInfo,
				&bufferAllocCreateInfo,
				&resources.indexBuffer,
				&resources.indexBufferAllocation,
				nullptr) != VK_SUCCESS)
		{
			Com_Error(ERR_FATAL, "Failed to create index buffer");
		}

		if (vmaMapMemory(
				context.allocator,
				resources.indexBufferAllocation,
				&resources.indexBufferBase) != VK_SUCCESS)
		{
			Com_Error(ERR_FATAL, "Failed to map index buffer into memory");
		}
		resources.indexBufferData = resources.indexBufferBase;

		if (vmaCreateBuffer(
				context.allocator, 
				&vertexBufferCreateInfo,
				&bufferAllocCreateInfo,
				&resources.vertexBuffer,
				&resources.vertexBufferAllocation,
				nullptr) != VK_SUCCESS)
		{
			Com_Error(ERR_FATAL, "Failed to create vertex buffer");
		}

		if (vmaMapMemory(
				context.allocator,
				resources.vertexBufferAllocation,
				&resources.vertexBufferBase) != VK_SUCCESS)
		{
			Com_Error(ERR_FATAL, "Failed to map vertex buffer into memory");
		}
		resources.vertexBufferData = resources.vertexBufferBase;
	}

	for (auto& resources : context.frameResources)
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(
				context.device,
				&semaphoreCreateInfo,
				nullptr,
				&resources.renderFinishedSemaphore) != VK_SUCCESS)
		{
			Com_Printf(S_COLOR_RED "Failed to create semaphore\n");
		}

		if (vkCreateSemaphore(
				context.device,
				&semaphoreCreateInfo,
				nullptr,
				&resources.imageAvailableSemaphore) != VK_SUCCESS)
		{
			Com_Printf(S_COLOR_RED "Failed to create semaphore\n");
		}

		// fence
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateFence(
				context.device,
				&fenceCreateInfo,
				nullptr,
				&resources.frameExecutedFence) != VK_SUCCESS)
		{
			Com_Printf(S_COLOR_RED "Failed to create fence\n");
		}
	}

	CreateDescriptorSetLayouts(gpuContext);
	CreatePipelineLayouts(gpuContext);
}

void GpuContextPreShutdown(GpuContext& context)
{
	vkDeviceWaitIdle(context.device);
}

void GpuContextShutdown(GpuContext& context)
{
	for (auto pipelineLayout : context.pipelineLayouts)
	{
		vkDestroyPipelineLayout(context.device, pipelineLayout, nullptr);
	}

	for (auto layout : context.descriptorSetLayouts)
	{
		vkDestroyDescriptorSetLayout(context.device, layout, nullptr);
	}

	vkDestroyCommandPool(context.device, context.gfxCommandPool, nullptr);
	vkDestroyCommandPool(context.device, context.transferCommandPool, nullptr);
	vkDestroyRenderPass(context.device, context.renderPass, nullptr); 

	for (auto iter : context.graphicsPipelines)
	{
		vkDestroyPipeline(context.device, iter.second, nullptr);
	}

	for (auto& swapchainResources : context.swapchain.resources)
	{
		vmaUnmapMemory(
			context.allocator,
			swapchainResources.vertexBufferAllocation);

		vmaFreeMemory(
			context.allocator,
			swapchainResources.vertexBufferAllocation);

		vkDestroyBuffer(
			context.device,
			swapchainResources.vertexBuffer,
			nullptr);

		vmaUnmapMemory(
			context.allocator,
			swapchainResources.indexBufferAllocation);

		vmaFreeMemory(
			context.allocator,
			swapchainResources.indexBufferAllocation);

		vkDestroyBuffer(
			context.device,
			swapchainResources.indexBuffer,
			nullptr);

		vkDestroyDescriptorPool(
			context.device,
			swapchainResources.descriptorPool,
			nullptr);

		vkDestroyImageView(
			context.device,
			swapchainResources.imageView,
			nullptr);

		vkDestroyFramebuffer(
			context.device,
			swapchainResources.framebuffer,
			nullptr);
	}

	vkDestroyDescriptorPool(
		context.device, context.globalDescriptorPool, nullptr);
	vkDestroyImageView(context.device, context.depthImageView, nullptr);
	vkDestroyImage(context.device, context.depthImage, nullptr);
	vmaFreeMemory(context.allocator, context.depthImageAllocation);

	for (auto& frameResources : context.frameResources)
	{
		vkDestroySemaphore(
			context.device,
			frameResources.imageAvailableSemaphore,
			nullptr);

		vkDestroySemaphore(
			context.device,
			frameResources.renderFinishedSemaphore,
			nullptr);

		vkDestroyFence(
			context.device,
			frameResources.frameExecutedFence,
			nullptr);
	}

	vmaDestroyAllocator(context.allocator);

	vkDestroySwapchainKHR(
		context.device, context.swapchain.swapchain, nullptr);
	vkDestroyDevice(context.device, nullptr); 

	if (context.debugUtilsMessenger != VK_NULL_HANDLE)
	{
		auto vkDestroyDebugUtilsMessengerEXT =
			(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
				context.instance, "vkDestroyDebugUtilsMessengerEXT");
		vkDestroyDebugUtilsMessengerEXT(
			context.instance, context.debugUtilsMessenger, nullptr);
	}
	vkDestroySurfaceKHR(context.instance, context.windowSurface, nullptr);
	vkDestroyInstance(context.instance, nullptr);
}

VkShaderModule GpuCreateShaderModuleFromFile(
	GpuContext& context, const char *filePath)
{
	void *code = nullptr;
	uint32_t codeSizeInBytes = ri.FS_ReadFile((char *)filePath, &code);
	if (codeSizeInBytes < 0 || !code)
	{
		Com_Error(ERR_FATAL, "Failed to load shader module from file '%s'", filePath);
	}

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = codeSizeInBytes;
	createInfo.pCode = static_cast<const uint32_t *>(code);

	VkShaderModule module;
	if (vkCreateShaderModule(
			context.device,
			&createInfo,
			nullptr,
			&module) != VK_SUCCESS)
	{
		ri.FS_FreeFile(code);
		Com_Error(ERR_FATAL, "Failed to create shader module");
	}

	ri.FS_FreeFile(code);

	return module;
}

void GpuDestroyShaderModule(GpuContext& context, VkShaderModule shaderModule)
{
	vkDestroyShaderModule(context.device, shaderModule, nullptr);
}

VkPipeline GpuGetGraphicsPipelineForRenderState(
	GpuContext& context,
	const RenderState& renderState)
{
	const auto iter = context.graphicsPipelines.find(renderState);
	if (iter != std::end(context.graphicsPipelines))
	{
		return iter->second;
	}

	std::array<VkPipelineShaderStageCreateInfo, 2> stageCreateInfos = {};
	stageCreateInfos[0].sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageCreateInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stageCreateInfos[0].module = renderState.vertexShader;
	stageCreateInfos[0].pName = "main";
	stageCreateInfos[1].sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageCreateInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stageCreateInfos[1].module = renderState.fragmentShader;
	stageCreateInfos[1].pName = "main";

	std::array<VkVertexInputBindingDescription, 1> vertexBindings = {};
	vertexBindings[0].binding = 0;
	vertexBindings[0].stride = renderState.vertexSize;
	vertexBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkPipelineVertexInputStateCreateInfo viCreateInfo = {};
	viCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	viCreateInfo.vertexBindingDescriptionCount = vertexBindings.size();
	viCreateInfo.pVertexBindingDescriptions = vertexBindings.data();
	viCreateInfo.vertexAttributeDescriptionCount = renderState.vertexAttributeCount;
	viCreateInfo.pVertexAttributeDescriptions = renderState.vertexAttributes;

	VkPipelineInputAssemblyStateCreateInfo iaCreateInfo = {};
	iaCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	iaCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineViewportStateCreateInfo viewportCreateInfo = {};
	viewportCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportCreateInfo.viewportCount = 1;
	viewportCreateInfo.pViewports = nullptr;
	viewportCreateInfo.scissorCount = 1;
	viewportCreateInfo.pScissors = nullptr;

	VkPipelineRasterizationStateCreateInfo rsCreateInfo = {};
	rsCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rsCreateInfo.polygonMode =
		(renderState.stateBits & GLS_POLYMODE_LINE)
			? VK_POLYGON_MODE_LINE
			: VK_POLYGON_MODE_FILL;
	rsCreateInfo.cullMode = GetVkCullMode(renderState.stateBits2);
	rsCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rsCreateInfo.depthBiasEnable =
		(renderState.stateBits2 & GLS2_POLYGONOFFSET) ? VK_TRUE : VK_FALSE;
	rsCreateInfo.depthBiasConstantFactor = r_offsetUnits->value;
	rsCreateInfo.depthBiasClamp = 0.0f;
	rsCreateInfo.depthBiasSlopeFactor = r_offsetFactor->value;
	rsCreateInfo.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo msCreateInfo = {};
	msCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	msCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	msCreateInfo.sampleShadingEnable = VK_FALSE;
	msCreateInfo.minSampleShading = 0.0f;
	msCreateInfo.pSampleMask = nullptr;
	msCreateInfo.alphaToCoverageEnable = VK_FALSE;
	msCreateInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo dsCreateInfo = {};
	dsCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	dsCreateInfo.depthTestEnable =
		!(renderState.stateBits & GLS_DEPTHTEST_DISABLE);
	dsCreateInfo.depthWriteEnable =
		(renderState.stateBits & GLS_DEPTHMASK_TRUE) ? VK_TRUE : VK_FALSE;
	dsCreateInfo.depthCompareOp =
		(renderState.stateBits & GLS_DEPTHFUNC_EQUAL)
			? VK_COMPARE_OP_EQUAL
			: VK_COMPARE_OP_LESS_OR_EQUAL;
	dsCreateInfo.minDepthBounds = 0.0f;
	dsCreateInfo.maxDepthBounds = 1.0f;

	VkPipelineColorBlendAttachmentState cbAttachment = {};
	cbAttachment.blendEnable =
		(renderState.stateBits & (GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS))
		? VK_TRUE
		: VK_FALSE;
	cbAttachment.srcColorBlendFactor =
		GetVkSrcBlendFactor(renderState.stateBits);
	cbAttachment.dstColorBlendFactor =
		GetVkDstBlendFactor(renderState.stateBits);
	cbAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	cbAttachment.srcAlphaBlendFactor =
		GetVkSrcBlendFactor(renderState.stateBits);
	cbAttachment.dstAlphaBlendFactor =
		GetVkDstBlendFactor(renderState.stateBits);
	cbAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	cbAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo cbCreateInfo = {};
	cbCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cbCreateInfo.attachmentCount = 1;
	cbCreateInfo.pAttachments = &cbAttachment;

	const std::array<VkDynamicState, 2> states = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = states.size();
	dynamicStateCreateInfo.pDynamicStates = states.data();

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.stageCount = stageCreateInfos.size();
	pipelineCreateInfo.pStages = stageCreateInfos.data();
	pipelineCreateInfo.pVertexInputState = &viCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &iaCreateInfo;
	pipelineCreateInfo.pViewportState = &viewportCreateInfo;
	pipelineCreateInfo.pRasterizationState = &rsCreateInfo;
	pipelineCreateInfo.pMultisampleState = &msCreateInfo;
	pipelineCreateInfo.pDepthStencilState = &dsCreateInfo;
	pipelineCreateInfo.pColorBlendState = &cbCreateInfo;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.layout = renderState.pipelineLayout;
	pipelineCreateInfo.renderPass = context.renderPass;
	pipelineCreateInfo.subpass = 0;

	VkPipeline graphicsPipeline;
	if (vkCreateGraphicsPipelines(
			context.device,
			VK_NULL_HANDLE,
			1,
			&pipelineCreateInfo,
			nullptr,
			&graphicsPipeline) != VK_SUCCESS)
	{
		Com_Error(ERR_FATAL, "Failed to create graphics pipeline");
	}

	context.graphicsPipelines.insert(
		std::make_pair(renderState, graphicsPipeline));

	return graphicsPipeline;
}

VkDescriptorSet GpuAllocateDescriptorSet(GpuContext& context, DescriptorSetId descriptorSetLayoutId)
{
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType =
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO; 
	descriptorSetAllocateInfo.descriptorPool =
		context.globalDescriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts =
		&gpuContext.descriptorSetLayouts[descriptorSetLayoutId];

	VkDescriptorSet descriptorSet;
	if (vkAllocateDescriptorSets(
			gpuContext.device,
			&descriptorSetAllocateInfo,
			&descriptorSet) != VK_SUCCESS)
	{
		Com_Error(ERR_FATAL, "Failed to create descriptor set");
	}
	return descriptorSet;
}

VkDescriptorSet GpuCreateDescriptorSet(GpuContext& context, const image_t* image)
{
	if (image == nullptr)
	{
		return VK_NULL_HANDLE;
	}

	VkDescriptorSet descriptorSet = GpuAllocateDescriptorSet(
		context,
		DESCRIPTOR_SET_SINGLE_TEXTURE);

	VkDescriptorImageInfo descriptorImageInfo = {};
	descriptorImageInfo.sampler = image->sampler;
	descriptorImageInfo.imageView = image->imageView;
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.pImageInfo = &descriptorImageInfo;
	vkUpdateDescriptorSets(gpuContext.device, 1, &descriptorWrite, 0, nullptr);

	context.descriptorSets.push_back(descriptorSet);

	return descriptorSet;
}

VkDescriptorSet GpuCreateMultitextureDescriptorSet(GpuContext& context, const shaderStage_t *stage)
{
	VkDescriptorSet descriptorSet = GpuAllocateDescriptorSet(
		context,
		DESCRIPTOR_SET_MULTI_TEXTURE);

	const image_t *image0 = stage->bundle[0].image;
	if (stage->bundle[0].numImageAnimations > 1)
	{
		image0 = ((image_t**)(stage->bundle[0].image))[0];
	}

	const image_t *image1 = stage->bundle[0].image;
	if (stage->bundle[1].numImageAnimations > 1)
	{
		image1 = ((image_t**)(stage->bundle[1].image))[0];
	}

	if (image0 == nullptr || image1 == nullptr)
	{
		return VK_NULL_HANDLE;
	}

	std::array<VkDescriptorImageInfo, 2> descriptorImageInfos = {};
	descriptorImageInfos[0].sampler = image0->sampler;
	descriptorImageInfos[0].imageView = image0->imageView;
	descriptorImageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	descriptorImageInfos[1].sampler = image1->sampler;
	descriptorImageInfos[1].imageView = image1->imageView;
	descriptorImageInfos[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorCount = descriptorImageInfos.size();
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.pImageInfo = descriptorImageInfos.data();

	vkUpdateDescriptorSets(gpuContext.device, 1, &descriptorWrite, 0, nullptr);

	context.descriptorSets.push_back(descriptorSet);

	return descriptorSet;
}

VkDeviceSize GetBufferOffset(const void* base, const void* pointer)
{
    return static_cast<const char*>(pointer) - static_cast<const char*>(base);
}