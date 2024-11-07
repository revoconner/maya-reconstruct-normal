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
    static MObject aColor;
    static MObject aUVCoord;
    static MObject aFilterSize;
    static MObject aOutColor;
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
MObject NormalReconstructZ::aColor;
MObject NormalReconstructZ::aUVCoord;
MObject NormalReconstructZ::aFilterSize;
MObject NormalReconstructZ::aOutColor;

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

    aColor = nAttr.createColor("inputColor", "ic");
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setDefault(1.0f, 1.0f, 1.0f));
    CHECK_MSTATUS(nAttr.setUsedAsColor(true));
    CHECK_MSTATUS(nAttr.setAffectsAppearance(true));

    MObject uCoord = nAttr.create("uCoord", "u", MFnNumericData::kFloat);
    MObject vCoord = nAttr.create("vCoord", "v", MFnNumericData::kFloat);
    aUVCoord = nAttr.create("uvCoord", "uv", uCoord, vCoord);
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setHidden(true));

    MObject filterX = nAttr.create("uvFilterSizeX", "fsx", MFnNumericData::kFloat);
    MObject filterY = nAttr.create("uvFilterSizeY", "fsy", MFnNumericData::kFloat);
    aFilterSize = nAttr.create("uvFilterSize", "fs", filterX, filterY);
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setHidden(true));

    aOutColor = nAttr.createColor("outColor", "oc");
    MAKE_OUTPUT(nAttr);
    CHECK_MSTATUS(nAttr.setUsedAsColor(true));
    CHECK_MSTATUS(nAttr.setAffectsAppearance(true));

    CHECK_MSTATUS(addAttribute(aColor));
    CHECK_MSTATUS(addAttribute(aUVCoord));
    CHECK_MSTATUS(addAttribute(aFilterSize));
    CHECK_MSTATUS(addAttribute(aOutColor));

    CHECK_MSTATUS(attributeAffects(aColor, aOutColor));
    CHECK_MSTATUS(attributeAffects(aUVCoord, aOutColor));
    CHECK_MSTATUS(attributeAffects(aFilterSize, aOutColor));

    return MS::kSuccess;
}

MStatus NormalReconstructZ::compute(const MPlug& plug, MDataBlock& block)
{
    if((plug != aOutColor) && (plug.parent() != aOutColor))
        return MS::kUnknownParameter;

    float2& uv = block.inputValue(aUVCoord).asFloat2();
    float2& filterSize = block.inputValue(aFilterSize).asFloat2();
    MFloatVector& inputColor = block.inputValue(aColor).asFloatVector();
    
    // Debug input values
    MString msg;
    msg = MString("Input values - R: ") + inputColor[0] + " G: " + inputColor[1] + " B: " + inputColor[2];
    MGlobal::displayInfo(msg);
    
    msg = MString("UV values - U: ") + uv[0] + " V: " + uv[1];
    MGlobal::displayInfo(msg);

    float r = inputColor[0];
    float g = inputColor[1];

    float x = r * 2.0f - 1.0f;
    float y = g * 2.0f - 1.0f;

    float dotProduct = x * x + y * y;
    if (dotProduct > 1.0f) dotProduct = 1.0f;
    float z = sqrt(1.0f - dotProduct);

    float length = sqrt(x * x + y * y + z * z);
    if (length > 0.0f) {
        x = x / length;
        y = y / length;
        z = z / length;
    }

    float b = (z + 1.0f) * 0.5f;

    msg = MString("Output values - R: ") + r + " G: " + g + " B: " + b;
    MGlobal::displayInfo(msg);

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
    <description>Normal reconstruction Z fragment</description>
    <properties>
        <float3 name="inputColor" semantic="mayaColor"/>
        <float2 name="uvCoord" semantic="mayaUvCoordSemantic" flags="varyingInputParam" />
        <float2 name="uvFilterSize" />
    </properties>
    <outputs>
        <float3 name="outColor" />
    </outputs>
    <implementation>
        <implementation render="OGSRenderer" language="GLSL" lang_version="3.0">
            <function_name val="normalReconstructZ" />
            <source><![CDATA[
            vec3 normalReconstructZ(vec3 color, vec2 uv, vec2 filterSize) {
                float r = clamp(color.r, 0.0, 1.0);
                float g = clamp(color.g, 0.0, 1.0);
                
                float x = r * 2.0 - 1.0;
                float y = g * 2.0 - 1.0;
                
                float dotProduct = min(x * x + y * y, 1.0);
                float z = sqrt(1.0 - dotProduct);
                
                float length = sqrt(x * x + y * y + z * z);
                if (length > 0.0) {
                    x /= length;
                    y /= length;
                    z /= length;
                }
                
                float b = (z + 1.0) * 0.5;
                return vec3(r, g, b);
            }
            ]]></source>
        </implementation>
        <implementation render="OGSRenderer" language="HLSL" lang_version="11.0">
            <function_name val="normalReconstructZ" />
            <source><![CDATA[
            float3 normalReconstructZ(float3 color, float2 uv, float2 filterSize) {
                float r = clamp(color.x, 0.0, 1.0);
                float g = clamp(color.y, 0.0, 1.0);
                
                float x = r * 2.0f - 1.0f;
                float y = g * 2.0f - 1.0f;
                
                float dotProduct = min(x * x + y * y, 1.0f);
                float z = sqrt(1.0f - dotProduct);
                
                float length = sqrt(x * x + y * y + z * z);
                if (length > 0.0f) {
                    x /= length;
                    y /= length;
                    z /= length;
                }
                
                float b = (z + 1.0f) * 0.5f;
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
            if (!fragmentMgr->hasFragment(sFragmentName))
            {
                if (sFragmentName == fragmentMgr->addShadeFragmentFromBuffer(sFragmentBody, false))
                {
                    fFragmentName = sFragmentName;
                }
            }
            else
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
    mappings.append(MHWRender::MAttributeParameterMapping(
        "inputColor",       // Parameter name in fragment
        "inputColor",       // Maya attribute name
        true,              // Allow connection
        true              // Allow parameter renaming
    ));
    
    mappings.append(MHWRender::MAttributeParameterMapping(
        "uvCoord",         // Parameter name in fragment
        "uvCoord",         // Maya attribute name
        true,             // Allow connection
        false            // Don't rename UV coordinates
    ));
    
    mappings.append(MHWRender::MAttributeParameterMapping(
        "uvFilterSize",    // Parameter name in fragment
        "uvFilterSize",    // Maya attribute name
        true,             // Allow connection
        false            // Don't rename filter size
    ));
}

static const MString sRegistrantId("normalReconstructZPlugin");

MStatus initializePlugin(MObject obj)
{
    MFnPlugin plugin(obj, "rev", "1.0", "Any");

    const MString UserClassify("texture/2d:drawdb/shader/texture/2d/normalReconstructZ");
    
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
            "drawdb/shader/texture/2d/normalReconstructZ",
            sRegistrantId,
            NormalReconstructZOverride::creator);
    }

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MFnPlugin plugin(obj);
    
    MHWRender::MDrawRegistry::deregisterShadingNodeOverrideCreator(
        "drawdb/shader/texture/2d/normalReconstructZ",
        sRegistrantId);
    
    plugin.deregisterNode(NormalReconstructZ::id);
    
    return MS::kSuccess;
}
