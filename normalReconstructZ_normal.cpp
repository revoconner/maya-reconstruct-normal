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

    virtual MStatus compute(const MPlug& plug, MDataBlock& block);
    static void* creator() { return new NormalReconstructZ(); }
    static MStatus initialize();

    static MTypeId id;
    static MObject aColor;
    static MObject aOutColor;
};


#define MAKE_INPUT(attr)						\
    CHECK_MSTATUS(attr.setKeyable(true));  		\
	CHECK_MSTATUS(attr.setStorable(true));		\
    CHECK_MSTATUS(attr.setReadable(true)); 		\
	CHECK_MSTATUS(attr.setWritable(true));

#define MAKE_OUTPUT(attr)						\
    CHECK_MSTATUS(attr.setKeyable(false)); 		\
	CHECK_MSTATUS(attr.setStorable(false));		\
    CHECK_MSTATUS(attr.setReadable(true)); 		\
	CHECK_MSTATUS(attr.setWritable(false));


// Attributes
MTypeId NormalReconstructZ::id(0x00001234);
MObject NormalReconstructZ::aColor;
MObject NormalReconstructZ::aOutColor;

MStatus NormalReconstructZ::initialize()
{
    MFnNumericAttribute nAttr;

    // Create a single color input (RGB)
	aColor = nAttr.createColor("inputColor", "ic");
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setDefault(1.f, 1.f, 1.f));

    // Output RGB
	// Create output attributes
    aOutColor = nAttr.createColor("outColor", "oc");
	MAKE_OUTPUT(nAttr);
    addAttribute(aOutColor);

    CHECK_MSTATUS(addAttribute(aColor));
    CHECK_MSTATUS(addAttribute(aOutColor));
    CHECK_MSTATUS(attributeAffects(aColor,    aOutColor));

    return MS::kSuccess;
}

MStatus NormalReconstructZ::compute(const MPlug& plug, MDataBlock& block)
{
    // outColor or individual R, G, B channel
    if((plug != aOutColor) && (plug.parent() != aOutColor))
        return MS::kUnknownParameter;

    MFloatVector& inputColor = block.inputValue(aColor).asFloatVector();
    
    // Debug input values
    MString msg;
    msg = MString("Input values - R: ") + inputColor[0] + " G: " + inputColor[1] + " B: " + inputColor[2];
    MGlobal::displayInfo(msg);

    float r = inputColor[0];
    float g = inputColor[1];

    // Convert to -1 to 1 range
    float x = r * 2.0f - 1.0f;
    float y = g * 2.0f - 1.0f;

    // Debug converted values
    msg = MString("Converted to -1,1 - X: ") + x + " Y: " + y;
    MGlobal::displayInfo(msg);

    // Calculate Z
    float dotProduct = x * x + y * y;
    if (dotProduct > 1.0f) dotProduct = 1.0f;
    float z = sqrt(1.0f - dotProduct);

    // Debug Z calculation
    msg = MString("Dot product: ") + dotProduct + " Z before norm: " + z;
    MGlobal::displayInfo(msg);

    // Normalize
    float length = sqrt(x * x + y * y + z * z);
    if (length > 0.0f) {
        x = x / length;
        y = y / length;
        z = z / length;
    }

    // Convert Z back to 0-1 range
    float b = (z + 1.0f) * 0.5f;

    // Debug final values
    msg = MString("Final values - R: ") + r + " G: " + g + " B: " + b;
    MGlobal::displayInfo(msg);

    // Create the result color with original R,G and reconstructed B
    MFloatVector resultColor(r, g, b);

    // Set the output
    MDataHandle outColorHandle = block.outputValue(aOutColor);
    MFloatVector& outColor = outColorHandle.asFloatVector();
    outColor = resultColor;
    outColorHandle.setClean();

    return MS::kSuccess;
}

MStatus initializePlugin(MObject obj)
{
    MFnPlugin plugin(obj, "Rev", "1.0", "Any");

    MString classification("utility/texture");
    
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