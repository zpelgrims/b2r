Copyright 2017 Pixar

Example implementation code for the Pixar tech memo titled "Geometry into Shading".  This tech memo is currently available at http://graphics.pixar.com/library/BumpRoughness/paper.pdf.

Licensing information can be found in the license.txt file.

At this time, Pixar has no plans to provide support or receive contributions to the example implementation code.

--------------------------------------------------

Notes from zeno:

- A special 6-channel image needs to be generated as the input to this shader, can be easily done with maketx:
    - e.g: '/home/cactus/Arnold-7.0.0.0-linux/bin/maketx' /home/cactus/b2r/scenes/36_scratches_smudges_fingerprints_specs.tif --bumpslopes

- In a broken state because:
    - Partial derivatives: Dx(variable), Dy(variable) don't seem supported in Arnold for anything but Dx(P)... If this is solved we can make this run the way it's supposed to be.


'/home/cactus/Arnold-7.0.0.0-linux/bin/oslc' src/PxrBump2RoughnessMake.osl -o dist/PxrBump2RoughnessMake.oso
'/home/cactus/Arnold-7.0.0.0-linux/bin/oslc' src/PxrBump2RoughnessRead.osl -o dist/PxrBump2RoughnessRead.oso
'/Applications/Autodesk/Arnold/mtoa/2020/bin/oslc' src/PxrBump2RoughnessRead.osl -o dist/PxrBump2RoughnessRead.oso 


