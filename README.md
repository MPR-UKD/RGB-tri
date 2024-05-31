# RGB-tri
RGBtri is a tool for the creation of color-coded maps of the diffusion components 
of triexponential diffusion-weighted imaging (DWI). 
The following triexponential modes is presumed:

S(b)/S<sub>0</sub> = F<sub>s</sub> e<sup>-bDs</sup> + F<sub>i</sub> e<sup>-bDi</sup> + F<sub>f</sub> e<sup>-Df</sup>.

RGBtri merges the diffusion fraction of slow, intermediate, and fast components int one rgb image. 

The inputs to RGBtri are maps of the fast (F<sub>f</sub>) and intermediate (F<sub>i</sub>) diffusion components. 
_(F<sub>f</sub> and F<sub>i</sub> should be scaled as a percentage so that their values range from 0 to 100. 
If Ff and Fi are between 0 and 1, use the "-f" option when running RGBtri.)_
The slow component (F<sub>s</sub>) is calculated internally by the equation 
F<sub>s</sub> = 1 - F<sub>i</sub> - F<sub>f</sub>.
The output of RGBtri is a color-coded parametric image, where F<sub>f</sub> uses the red, F<sub>i</sub> 
the green, and F<sub>s</sub> the blue color channel.

The syntax of RGBtri is quite simple: _"RGBtri -Ff file1.nii -Fi file2.nii -o output.nii"_.
Type _"RGBtri -h"_ for more details.
Some test data is available in the _"test-data"_ directory.
A Windows executable is provided in the _"win-binary"_ directory.
A Linux executable (Ubuntu 22.04.4 LTS) can be found in _"ubuntu-binary"_ directory.


![](assets/color-kidney1.png)

The algorithms presented here are described in: 
**"Presentation of microstructural diffusion components by color schemes in abdominal organs"** in Magnetic Resonance in Medicine.
Article DOI: 10.1002/mrm.30183

C++ code is provided to create RGB diffusion component maps. A cmake file has been added for compilation.
Since RGBtri software works with Nifti images, the only dependency is the Nifti package: https://github.com/NIFTI-Imaging/nifti_clib
