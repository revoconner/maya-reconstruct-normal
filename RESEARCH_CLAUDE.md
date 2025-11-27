# Normal Reconstruct Z - Maya Plugin Development Notes

## Goal
Create a Maya plugin that takes the R and G channels of a normal map and reconstructs the B (Z) channel using the unit normal constraint: `X^2 + Y^2 + Z^2 = 1`

## The Core Problem
When connecting a file texture's `outColor` to a utility node's color input in Maya, the `compute()` method receives `(0, 0, 0)` instead of the actual texture values. This is because Maya's dependency graph evaluation doesn't automatically sample textures at UV coordinates for utility nodes.

## Approaches Tried

### 1. Simple Utility Node with Color Input (FAILED)
- Created `MPxNode` with `inputColor` (float3) attribute
- Connected file texture's `outColor` to `inputColor`
- **Result**: Always received `(0, 0, 0)` - textures aren't sampled

### 2. Added UV Coordinates (FAILED)
- Added `uvCoord` and `uvFilterSize` attributes like texture nodes have
- Connected `place2dTexture.outUV` to node's `uvCoord`
- **Result**: Still `(0, 0, 0)` - UV coords received but texture not sampled

### 3. MRenderUtil::sampleShadingNetwork (FAILED)
- Used `MRenderUtil::sampleShadingNetwork()` in compute to explicitly sample connected texture
- **Result**: Didn't work - this function needs proper render context

### 4. Texture Node with MImage (PARTIAL)
- Changed to texture node that takes `fileName` string input
- Used `MImage::readFromFile()` to load texture
- Sampled pixels directly at UV coordinates
- **Result**: Showed `0.5, 0, 0.5`

### 5. MPxShadingNodeOverride for VP2 (WORKING)
- Per forum post, need `MPxShadingNodeOverride` for VP2 viewport to work
- Fragment receives `inputColor` and processes it
- Fragment graph connects upstream texture fragment to our fragment automatically
- **Status**: WORKING

## Key Insights from Research

1. **Maya's DG evaluation doesn't sample textures** - When you connect a file texture to a utility node, the utility node's compute() just gets (0,0,0) because there's no automatic texture sampling

2. **VP2 works differently** - The fragment shader system composes fragments together, so texture sampling happens in the GPU shader pipeline

3. **Forum solution** - User on Autodesk forums said they had to implement `MPxShadingNodeOverride` for it to work (not elegant but necessary)

4. **File node approach** - Maya's built-in `file` node works because it samples textures itself using `MImage` - it doesn't receive color from another node


```

## References
- https://forums.autodesk.com/t5/maya-programming-forum/openmaya-plugin-use-texture-as-color-input/td-p/9392499
- https://help.autodesk.com/cloudhelp/2017/ENU/Maya-SDK/files/GUID-585F5656-4069-4D82-B9BB-3D1AB2D0DFE6.htm (Shading Node Overrides)
- Maya DevKit examples: fileTexture, checkerShader, brickShader
