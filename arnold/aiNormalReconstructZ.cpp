/*
 * aiNormalReconstructZ - Arnold Shader
 *
 * Reconstructs the Z (Blue) channel of a normal map from R and G channels
 * using the unit normal constraint: X^2 + Y^2 + Z^2 = 1
 */

#include <ai.h>
#include <cmath>

AI_SHADER_NODE_EXPORT_METHODS(NormalReconstructZMethods);

enum NormalReconstructZParams
{
    p_input
};

node_parameters
{
    AiParameterRGB("input", 0.5f, 0.5f, 1.0f);
}

node_initialize
{
}

node_update
{
}

node_finish
{
}

shader_evaluate
{
    // Get input color - this automatically evaluates connected shaders (textures)
    AtRGB input = AiShaderEvalParamRGB(p_input);

    float r = input.r;
    float g = input.g;

    // Convert from 0-1 range to -1 to 1 range
    float x = r * 2.0f - 1.0f;
    float y = g * 2.0f - 1.0f;

    // Calculate Z from unit normal constraint: x^2 + y^2 + z^2 = 1
    float dotProduct = x * x + y * y;
    if (dotProduct > 1.0f) dotProduct = 1.0f;
    float z = sqrtf(1.0f - dotProduct);

    // Normalize
    float len = sqrtf(x * x + y * y + z * z);
    if (len > 0.0f)
    {
        z = z / len;
    }

    // Convert Z back to 0-1 range
    float b = (z + 1.0f) * 0.5f;

    // Output reconstructed normal
    sg->out.RGB() = AtRGB(r, g, b);
}

node_loader
{
    if (i > 0)
        return false;

    node->methods = NormalReconstructZMethods;
    node->output_type = AI_TYPE_RGB;
    node->name = "normalReconstructZ";
    node->node_type = AI_NODE_SHADER;
    strcpy(node->version, AI_VERSION);
    return true;
}
