# Maya hypershade - Reconstruction node for packed normal map textures
### Two nodes for Maya compatible shader and for Arnold

## Screenshot Preview
![preview](https://github.com/user-attachments/assets/dad7e213-6d82-4bbd-8943-3e94a8158fdd)

## How to install (2025 - Windows only)
1. Download the zip from the release tab
2. Unzip it.
3. Use the `install_built.bat` file to copy the files to their correct folders for Maya and Arnold. </br>
Or copy manually if you have not installed maya in the default intall location.
4. Releases are for Windows only.

## Build it

1. Clone the git repo (use git or download as a zip)
2. Run the script `install_build_tools.bat` to install MSVC build library. **Skip if you have Visual studio 2022 installed, this will give an error if you do not have Winget package manager installed in Windows**
3. Run the `build_plugins.bat` script to build it.
4. Then run the `install_plugins.bat` to copy the built binary to their respective places.</br>
Or copy manually if you have not installed maya in the default intall location.

### If you are not on Windows you may have to create your own build files, the scripts should still work.

## Generative AI use
I was stuck in this project for the better part of a year, maybe even more. Today I just wanted to go back and fix it, so I asked Claude (Anthropic) to see if it can fix it, [Closed Unmerged Pull Request](https://github.com/revoconner/maya-reconstruct-normal/pull/1)

That did not work unfortunately so I asked it to just create a research of whatever it found, when I fed it all I had asked in the Autodesk forums last year. That was very helpful, because I got no replies from people in the forum. 

**The reasearch file is present as a markdown file in this branch, feel free to use it to make your own shader or maya texture based plugins**
