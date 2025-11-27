#include <maya/MFnNumericAttribute.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MTypeId.h>
#include <maya/MPxNode.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFloatVector.h>
#include <maya/MPxShadingNodeOverride.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MFragmentManager.h>
#include <maya/MDrawRegistry.h>
#include <cmath>

class NormalReconstructZ : public MPxNode
{
public:
    NormalReconstructZ() {}
    ~NormalReconstructZ() override {}

    MStatus compute(const MPlug& plug, MDataBlock& block) override;
    SchedulingType schedulingType() const override { return SchedulingType::kParallel; }

    static void* creator() { return new NormalReconstructZ(); }
    static MStatus initialize();

    static MTypeId id;
    static MObject aInputColor;
    static MObject aInputColorR;
    static MObject aInputColorG;
    static MObject aInputColorB;
    static MObject aOutColor;
    static MObject aOutColorR;
    static MObject aOutColorG;
    static MObject aOutColorB;
};

class NormalReconstructZOverride : public MHWRender::MPxShadingNodeOverride
{
public:
    static MHWRender::MPxShadingNodeOverride* creator(const MObject& obj)
    {
        return new NormalReconstructZOverride(obj);
    }

    NormalReconstructZOverride(const MObject& obj);
    ~NormalReconstructZOverride() override {}

    MHWRender::DrawAPI supportedDrawAPIs() const override;
    MString fragmentName() const override;
    void getCustomMappings(MHWRender::MAttributeParameterMappingList& mappings) override;

private:
    MString fFragmentName;
};

// Static member initialization
MTypeId NormalReconstructZ::id(0x00001234);
MObject NormalReconstructZ::aInputColor;
MObject NormalReconstructZ::aInputColorR;
MObject NormalReconstructZ::aInputColorG;
MObject NormalReconstructZ::aInputColorB;
MObject NormalReconstructZ::aOutColor;
MObject NormalReconstructZ::aOutColorR;
MObject NormalReconstructZ::aOutColorG;
MObject NormalReconstructZ::aOutColorB;

#define MAKE_INPUT(attr)                         \
    CHECK_MSTATUS(attr.setKeyable(true));       \
    CHECK_MSTATUS(attr.setStorable(true));      \
    CHECK_MSTATUS(attr.setReadable(true));      \
    CHECK_MSTATUS(attr.setWritable(true));

#define MAKE_OUTPUT(attr)                        \
    CHECK_MSTATUS(attr.setKeyable(false));      \
    CHECK_MSTATUS(attr.setStorable(false));     \
    CHECK_MSTATUS(attr.setReadable(true));      \
    CHECK_MSTATUS(attr.setWritable(false));

MStatus NormalReconstructZ::initialize()
{
    MFnNumericAttribute nAttr;

    // Input color with child attributes for proper connection
    aInputColorR = nAttr.create("inputColorR", "icr", MFnNumericData::kFloat, 0.5f);
    aInputColorG = nAttr.create("inputColorG", "icg", MFnNumericData::kFloat, 0.5f);
    aInputColorB = nAttr.create("inputColorB", "icb", MFnNumericData::kFloat, 1.0f);
    aInputColor = nAttr.create("inputColor", "ic", aInputColorR, aInputColorG, aInputColorB);
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setUsedAsColor(true));

    // Output color with child attributes
    aOutColorR = nAttr.create("outColorR", "ocr", MFnNumericData::kFloat, 0.5f);
    aOutColorG = nAttr.create("outColorG", "ocg", MFnNumericData::kFloat, 0.5f);
    aOutColorB = nAttr.create("outColorB", "ocb", MFnNumericData::kFloat, 1.0f);
    aOutColor = nAttr.create("outColor", "oc", aOutColorR, aOutColorG, aOutColorB);
    MAKE_OUTPUT(nAttr);
    CHECK_MSTATUS(nAttr.setUsedAsColor(true));

    CHECK_MSTATUS(addAttribute(aInputColor));
    CHECK_MSTATUS(addAttribute(aOutColor));

    CHECK_MSTATUS(attributeAffects(aInputColor, aOutColor));
    CHECK_MSTATUS(attributeAffects(aInputColorR, aOutColor));
    CHECK_MSTATUS(attributeAffects(aInputColorG, aOutColor));
    CHECK_MSTATUS(attributeAffects(aInputColorB, aOutColor));
    CHECK_MSTATUS(attributeAffects(aInputColorR, aOutColorR));
    CHECK_MSTATUS(attributeAffects(aInputColorG, aOutColorG));
    CHECK_MSTATUS(attributeAffects(aInputColorB, aOutColorB));

    return MS::kSuccess;
}

MStatus NormalReconstructZ::compute(const MPlug& plug, MDataBlock& block)
{
    if ((plug != aOutColor) && (plug.parent() != aOutColor) &&
        (plug != aOutColorR) && (plug != aOutColorG) && (plug != aOutColorB))
    {
        return MS::kUnknownParameter;
    }

    MFloatVector& inputColor = block.inputValue(aInputColor).asFloatVector();

    float r = inputColor[0];
    float g = inputColor[1];

    // Convert from 0-1 range to -1 to 1 range
    float x = r * 2.0f - 1.0f;
    float y = g * 2.0f - 1.0f;

    // Calculate Z from X and Y (unit normal constraint)
    float dotProduct = x * x + y * y;
    if (dotProduct > 1.0f) dotProduct = 1.0f;
    float z = sqrtf(1.0f - dotProduct);

    // Normalize
    float len = sqrtf(x * x + y * y + z * z);
    if (len > 0.0f) {
        z = z / len;
    }

    // Convert Z back to 0-1 range
    float b = (z + 1.0f) * 0.5f;

    MDataHandle outColorHandle = block.outputValue(aOutColor);
    MFloatVector& outColor = outColorHandle.asFloatVector();
    outColor = MFloatVector(r, g, b);
    outColorHandle.setClean();

    return MS::kSuccess;
}

NormalReconstructZOverride::NormalReconstructZOverride(const MObject& obj)
    : MPxShadingNodeOverride(obj)
    , fFragmentName("")
{
    static const MString sFragmentName("normalReconstructZFragment");
    static const char* sFragmentBody = R"(
<fragment uiName="normalReconstructZFragment" name="normalReconstructZFragment" type="plumbing" class="ShadeFragment" version="1.0">
    <description>Reconstructs Z channel of normal map from RG</description>
    <properties>
        <float3 name="inputColor" />
    </properties>
    <outputs>
        <float3 name="outColor" />
    </outputs>
    <implementation>
        <implementation render="OGSRenderer" language="GLSL" lang_version="3.0">
            <function_name val="normalReconstructZ" />
            <source><![CDATA[
vec3 normalReconstructZ(vec3 inputColor) {
    float r = inputColor.r;
    float g = inputColor.g;

    float x = r * 2.0 - 1.0;
    float y = g * 2.0 - 1.0;

    float dotProduct = min(x * x + y * y, 1.0);
    float z = sqrt(1.0 - dotProduct);

    float len = sqrt(x * x + y * y + z * z);
    if (len > 0.0) {
        z /= len;
    }

    float b = (z + 1.0) * 0.5;
    return vec3(r, g, b);
}
            ]]></source>
        </implementation>
        <implementation render="OGSRenderer" language="HLSL" lang_version="11.0">
            <function_name val="normalReconstructZ" />
            <source><![CDATA[
float3 normalReconstructZ(float3 inputColor) {
    float r = inputColor.x;
    float g = inputColor.y;

    float x = r * 2.0 - 1.0;
    float y = g * 2.0 - 1.0;

    float dotProduct = min(x * x + y * y, 1.0);
    float z = sqrt(1.0 - dotProduct);

    float len = sqrt(x * x + y * y + z * z);
    if (len > 0.0) {
        z /= len;
    }

    float b = (z + 1.0) * 0.5;
    return float3(r, g, b);
}
            ]]></source>
        </implementation>
    </implementation>
</fragment>
)";

    MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
    if (theRenderer)
    {
        MHWRender::MFragmentManager* fragmentMgr = theRenderer->getFragmentManager();
        if (fragmentMgr)
        {
            // Remove old fragment if exists to get fresh version
            if (fragmentMgr->hasFragment(sFragmentName))
            {
                fragmentMgr->removeFragment(sFragmentName);
            }

            if (sFragmentName == fragmentMgr->addShadeFragmentFromBuffer(sFragmentBody, false))
            {
                fFragmentName = sFragmentName;
            }
        }
    }
}

MHWRender::DrawAPI NormalReconstructZOverride::supportedDrawAPIs() const
{
    return MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile;
}

MString NormalReconstructZOverride::fragmentName() const
{
    return fFragmentName;
}

void NormalReconstructZOverride::getCustomMappings(
    MHWRender::MAttributeParameterMappingList& mappings)
{
    MHWRender::MAttributeParameterMapping inputMapping(
        "inputColor",    // fragment parameter name
        "inputColor",    // node attribute name
        true,            // allow connection
        true             // allow renaming
    );
    mappings.append(inputMapping);
}

static const MString sRegistrantId("normalReconstructZPlugin");

MStatus initializePlugin(MObject obj)
{
    MFnPlugin plugin(obj, "rev", "1.0", "Any");

    const MString UserClassify("utility/color:drawdb/shader/utility/normalReconstructZ");

    MStatus status = plugin.registerNode(
        "normalReconstructZ",
        NormalReconstructZ::id,
        NormalReconstructZ::creator,
        NormalReconstructZ::initialize,
        MPxNode::kDependNode,
        &UserClassify);

    if (status == MS::kSuccess)
    {
        status = MHWRender::MDrawRegistry::registerShadingNodeOverrideCreator(
            "drawdb/shader/utility/normalReconstructZ",
            sRegistrantId,
            NormalReconstructZOverride::creator);
    }

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MFnPlugin plugin(obj);

    MHWRender::MDrawRegistry::deregisterShadingNodeOverrideCreator(
        "drawdb/shader/utility/normalReconstructZ",
        sRegistrantId);

    plugin.deregisterNode(NormalReconstructZ::id);

    return MS::kSuccess;
}
