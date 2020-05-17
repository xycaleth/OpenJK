#include "tr_vertex_formats.h"

constexpr std::array<VkVertexInputAttributeDescription, 3>
    SingleTextureVertexFormat::vertexAttributes;
constexpr size_t SingleTextureVertexFormat::vertexSize;

constexpr std::array<VkVertexInputAttributeDescription, 4>
    MultiTextureVertexFormat::vertexAttributes;
constexpr size_t MultiTextureVertexFormat::vertexSize;