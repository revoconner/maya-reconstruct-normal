#include <maya/MFnNumericAttribute.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MTypeId.h>
#include <maya/MPxNode.h>
#include <maya/MVector.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFloatVector.h>
#include <maya/MGlobal.h>
#include <cmath>

class NormalReconstructZ : public MPxNode
{
public:
    NormalReconstructZ() {}
    virtual ~NormalReconstructZ() {}

    virtual MStatus compute(const MPlug& plug, MDataBlock& data);
    static void* creator() { return new NormalReconstructZ(); }
    static MStatus initialize();

    static MTypeId id;
    static MObject input;
    static MObject output;
};

MTypeId NormalReconstructZ::id(0x00001234);
MObject NormalReconstructZ::input;
MObject NormalReconstructZ::output;

#define MAKE_INPUT(attr)                     \
    CHECK_MSTATUS(attr.setKeyable(true));   \
    CHECK_MSTATUS(attr.setStorable(true));   \
    CHECK_MSTATUS(attr.setReadable(true));   \
    CHECK_MSTATUS(attr.setWritable(true));

#define MAKE_OUTPUT(attr)                    \
    CHECK_MSTATUS(attr.setKeyable(false));   \
    CHECK_MSTATUS(attr.setStorable(false));  \
    CHECK_MSTATUS(attr.setReadable(true));   \
    CHECK_MSTATUS(attr.setWritable(false));

MStatus NormalReconstructZ::initialize()
{
    MFnNumericAttribute nAttr;

    // Create input attributes - using createColor like the contrast shader
    input = nAttr.createColor("inputColor", "ic");
    MAKE_INPUT(nAttr);

    // Create output attributes
    output = nAttr.createColor("outColor", "oc");
    MAKE_OUTPUT(nAttr);

    // Add attributes
    CHECK_MSTATUS(addAttribute(input));
    CHECK_MSTATUS(addAttribute(output));

    // Set dependencies
    CHECK_MSTATUS(attributeAffects(input, output));

    return MS::kSuccess;
}

MStatus NormalReconstructZ::compute(const MPlug& plug, MDataBlock& data)
{
    // outColor or individual R, G, B channel
    if((plug != output) && (plug.parent() != output))
        return MS::kUnknownParameter;

    // Get input color using FloatVector like contrast shader
    MFloatVector resultColor;
    MFloatVector& col = data.inputValue(input).asFloatVector();
    
    MGlobal::displayInfo(MString("Input color values - R: ") + col[0] + 
                        " G: " + col[1] + " B: " + col[2]);

    // Convert to -1 to 1 range
    float x = col[0] * 2.0f - 1.0f;
    float y = col[1] * 2.0f - 1.0f;

    // Calculate Z
    float dotProduct = x * x + y * y;
    if (dotProduct > 1.0f) dotProduct = 1.0f;
    float z = sqrt(1.0f - dotProduct);

    // Normalize
    float length = sqrt(x * x + y * y + z * z);
    if (length > 0.0f) {
        x = x / length;
        y = y / length;
        z = z / length;
    }

    // Convert Z back to 0-1 range
    float b = (z + 1.0f) * 0.5f;

    // Set the result color using original R,G and computed B
    resultColor[0] = col[0];  // Preserve original R
    resultColor[1] = col[1];  // Preserve original G
    resultColor[2] = b;       // Set computed B

    // Set output color attribute like contrast shader
    MDataHandle outColorHandle = data.outputValue(output);
    MFloatVector& outColor = outColorHandle.asFloatVector();
    outColor = resultColor;
    outColorHandle.setClean();

    return MS::kSuccess;
}

MStatus initializePlugin(MObject obj)
{
    MFnPlugin plugin(obj, "Your Name", "1.0", "Any");

    MString classification("utility/color");
    
    MStatus status = plugin.registerNode(
        "normalReconstructZ", 
        NormalReconstructZ::id,
        NormalReconstructZ::creator,
        NormalReconstructZ::initialize,
        MPxNode::kDependNode,
        &classification);

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MFnPlugin plugin(obj);
    plugin.deregisterNode(NormalReconstructZ::id);
    return MS::kSuccess;
}