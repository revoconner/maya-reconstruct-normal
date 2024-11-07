import math
import maya.api.OpenMaya as om2
import maya.OpenMaya as om
import maya.OpenMayaMPx as ommpx
import maya.OpenMayaRender as omr

# Node definition
nodeName = "normalMapZReconstruction"
nodeTypeId = om.MTypeId(0x1234567)  # Replace with your unique ID

class NormalMapZReconstruction(ommpx.MPxNode):
    # Input attributes
    textureInput = None
    
    # Output attributes
    outputColor = None
    
    def __init__(self):
        ommpx.MPxNode.__init__(self)
    
    def compute(self, plug, dataBlock):
        if plug == NormalMapZReconstruction.outputColor:
            inputDataHandle = dataBlock.inputValue(NormalMapZReconstruction.textureInput)
            inputColor = inputDataHandle.asFloat3()
            
            # Extract R and G components (X and Y of normal)
            x = inputColor[0] * 2.0 - 1.0
            y = inputColor[1] * 2.0 - 1.0
            
            # Calculate Z component
            z = math.sqrt(max(0, 1.0 - (x*x + y*y)))
            
            # Convert back to 0-1 range
            z = z * 0.5 + 0.5
            
            outputHandle = dataBlock.outputValue(NormalMapZReconstruction.outputColor)
            outputHandle.set3Float(inputColor[0], inputColor[1], z)
            dataBlock.setClean(plug)
            
        return True

# Viewport 2.0 Override
class NormalMapZReconstructionOverride(omr.MHWRender.MPxShadingNodeOverride):
    def __init__(self, obj):
        omr.MHWRender.MPxShadingNodeOverride.__init__(self, obj)
        self.fObject = obj

    @staticmethod
    def creator(obj):
        return NormalMapZReconstructionOverride(obj)

    def supportedDrawAPIs(self):
        # Support both GL and DX
        return omr.MHWRender.MRenderer.kAllDevices

    def fragmentName(self):
        # Return the name of your fragment shader
        return "NormalMapZReconstruction"

    def getCustomMappings(self, mappings):
        # Create mappings between node attributes and fragment parameters
        mappings.append(omr.MHWRender.MAttributeParameterMapping(
            "normalMapInput",  # parameter name in fragment
            "textureInput",    # attribute name in Maya node
            True,             # allow connections
            True              # allow parameter rename
        ))
        mappings.append(omr.MHWRender.MAttributeParameterMapping(
            "output",         # parameter name in fragment
            "outputColor",    # attribute name in Maya node
            True,            # allow connections
            True             # allow parameter rename
        ))

    def updateDG(self):
        # Nothing needed for DG update
        pass

    def updateShader(self, shader, mappings):
        # Nothing needed for shader update - values automatically mapped
        pass

def initializePlugin(mobject):
    mplugin = ommpx.MFnPlugin(mobject, "AAA", "1.0")
    try:
        # Register the node
        mplugin.registerNode(
            nodeName,
            nodeTypeId,
            lambda: ommpx.asMPxPtr(NormalMapZReconstruction()),
            initialize,
            ommpx.MPxNode.kDependNode
        )
        
        # Register the shader override
        omr.MDrawRegistry.registerShadingNodeOverrideCreator(
            "drawdb/shader/surface/" + nodeName,
            nodeTypeId,
            NormalMapZReconstructionOverride.creator
        )
        
        # Register the fragment (HLSL shader code)
        fragment = '''
            <fragment uiName="NormalMapZReconstruction" name="NormalMapZReconstruction" type="plumbing" class="ShadeFragment" version="1.0">
                <description><![CDATA[Reconstructs normal map Z channel]]></description>
                <properties>
                    <float3 name="normalMapInput" semantic="Color"/>
                    <float3 name="output" semantic="Color"/>
                </properties>
                <values>
                    <float3 name="normalMapInput" value="0.5,0.5,0.5"/>
                    <float3 name="output" value="0,0,0"/>
                </values>
                <implementation>
                <implementation render="OGSRenderer" language="HLSL" lang_version="11.000000">
                    <function_name val="NormalMapZReconstruction" />
                    <source><![CDATA[
                        float3 NormalMapZReconstruction(float3 normalMapInput)
                        {
                            float2 xy = normalMapInput.xy * 2.0 - 1.0;
                            float z = sqrt(max(0, 1.0 - dot(xy, xy)));
                            return float3(normalMapInput.xy, z * 0.5 + 0.5);
                        }
                    ]]></source>
                </implementation>
                <implementation render="OGSRenderer" language="GLSL" lang_version="3.0">
                    <function_name val="NormalMapZReconstruction" />
                    <source><![CDATA[
                        vec3 NormalMapZReconstruction(vec3 normalMapInput)
                        {
                            vec2 xy = normalMapInput.xy * 2.0 - 1.0;
                            float z = sqrt(max(0.0, 1.0 - dot(xy, xy)));
                            return vec3(normalMapInput.xy, z * 0.5 + 0.5);
                        }
                    ]]></source>
                </implementation>
                </implementation>
            </fragment>
        '''
        
        fragmentMgr = omr.MRenderer.getFragmentManager()
        if not fragmentMgr.hasFragment("NormalMapZReconstruction"):
            fragmentMgr.addShadeFragment(fragment)
            
    except:
        raise

def initialize(nodeFn):
    # Initialize attributes
    nAttr = om.MFnNumericAttribute()
    
    # Input color attribute (texture input)
    NormalMapZReconstruction.textureInput = nAttr.createColor("textureInput", "in")
    nAttr.setStorable(True)
    nAttr.setKeyable(True)
    
    # Output color attribute
    NormalMapZReconstruction.outputColor = nAttr.createColor("outputColor", "out")
    nAttr.setStorable(False)
    nAttr.setKeyable(False)
    nAttr.setWritable(False)
    
    # Add attributes
    NormalMapZReconstruction.addAttribute(NormalMapZReconstruction.textureInput)
    NormalMapZReconstruction.addAttribute(NormalMapZReconstruction.outputColor)
    
    # Set dependencies
    NormalMapZReconstruction.attributeAffects(
        NormalMapZReconstruction.textureInput,
        NormalMapZReconstruction.outputColor
    )
    
    return True

def uninitializePlugin(mobject):
    mplugin = ommpx.MFnPlugin(mobject)
    try:
        # Deregister the shader override
        omr.MDrawRegistry.deregisterShadingNodeOverrideCreator(
            "drawdb/shader/surface/" + nodeName,
            nodeTypeId
        )
        
        # Deregister the node
        mplugin.deregisterNode(nodeTypeId)
        
        # Remove the fragment
        fragmentMgr = omr.MRenderer.getFragmentManager()
        if fragmentMgr.hasFragment("NormalMapZReconstruction"):
            fragmentMgr.removeFragment("NormalMapZReconstruction")
    except:
        raise
