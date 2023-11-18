# apEx (Another Programming EXperiment) demotool

This repo contains the apEx demotool used by [Conspiracy](https://conspiracy.hu/) and others between the years 2014 and 2023.
The full list of releases created with it can be found [here](https://demozoo.org/productions/tagged/apex/).

## Credits

- BoyC - tool and engine code
- Gargaj - synth code
- Zoom - UI, testing, UX and primary target audience

## Contents of the repo
- **apEx**: Main tool code
  - **apEx**: Tool executable project
  - **LibCTiny**: Tiny CRT library for use in final releases
  - **Libraries**: Third-party libraries
  - **MinimalPlayer**: Main release executable project
  - **MVX**: Synth library
  - **Phoenix**: The core of the 64k engine
  - **Phoenix_Tool**: Wrapper code for the Phoenix engine to be used by the tool
  - **ThirdParty**: D3DX headers and libraries, because nowadays it's a headache to install the latast DX11 SDK
  - **Utils**: Build tools (NASM and executable compressors)
- **Bedrock**: Base code libraries, including the UI
  - **BaseLib**: Basic runtime classes
  - **CoRE2**: Conspiracy Rendering Engine 2 (This was supposed to be a nextgen engine for the MMO [Perpetuum](http://www.perpetuum-online.com/) in 2010 but it never materialized. Used as a D3D access layer by the UI.)
  - **UtilLib**: Utility functions library
  - **Whiteboard**: UI library
- **Projects**: Project files for the Conspiracy releases created with apEx:
  - Clean Slate
  - Darkness Lay Your Eyes Upon Me
  - Offscreen Colonies _(note: some scenes are now slightly broken, use build 424 to view)_
  - One of These Days The Sky's Gonna Break
  - Supermode
  - Universal Sequence _(note: deadlocks this tool build on load, use build 424 to view)_
  - Vessel
  - When Silence Dims the Stars Above _(note: some scenes are now broken, use build 424 to view)_

## Acknowledgements and third-party libraries
- D3DX by Microsoft (https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dx)
- JSON++ by Hong Jiang (https://github.com/hjiang/jsonxx)
- RapidXML by Marcin Kalicinski (https://rapidxml.sourceforge.net/)
- STB Image and Vorbis libraries by Sean Barrett (https://nothings.org)
- Open Asset Import Library by the ASSIMP dev team (https://www.assimp.org)
- V2 Synthesizer System by Tammo "kb" Hinrichs (http://www.1337haxorz.de/products.html)
- WaveSabre Synthesizer by Jake "Ferris" Taylor (https://github.com/logicomacorp/WaveSabre)
- A C++ port of Arbaro by Wolfram Diestel (https://arbaro.sourceforge.net/)
- .kkrunchy by Fabian "ryg" Giesen (https://github.com/farbrausch/fr_public/tree/master/kkrunchy)
- .rekkrunchy by Ralph "revivalizer" Brorsen (https://github.com/revivalizer/rekkrunchy)
- OpenSSL by The OpenSSL Project (https://www.openssl.org/)
- Miniz by Rich Geldreich (https://github.com/richgel999/miniz)
- NvTriStrip by NVIDIA (https://github.com/turbulenz/NvTriStrip)
- A heavily modified version of the UE HLSL AST code by Epic Games (for shader minification)
- ProFont by Carl Osterwald, Stephen C. Gilardi, Andrew Welch (https://tobiasjung.name/profont/) (Love you guys)

These software are available under their respective licenses.

## Release notes
apEx was in development between summer 2010 and spring 2021, releases range from spring 2014 to Spring 2023.
The tool and all projects have been retargeted to VS 2022, meaning there are a whole bunch of warnings during build, and more importantly 
the 64k engine won't build as small as for the original releases, because it was optimized for VS 2010 (also a pain to install nowadays)

We have included all the project files for the Conspiracy releases, but as you can imagine over the years breaking changes crept in and
some of the projects won't look as they originally did. Frankly we're amazed how good most of them still held up.

The minimalplayer will only compile if the appropriate minimal release binaries are generated for it. There's an automatic compile
function in the tool somewhere that does all that, but it needs a bit configuration to work; we did 99% of the work for you, 
so you can work a bit for a release ;)

We're supplying 2 binary releases as well, the very last build ever (build 500) and an earlier one (build 424) that can load the now 
broken older releases well (namely Offscreen Colonies, When Silence Dims the Stars Above and Universal Sequence). With Clean Slate we 
really didn't care anymore for breaking changes and the like ;)

## License
See LICENSE.txt for the license. A bit of explanation is in order as we know this may be a bit unconventional. The point of this release 
is to satisfy curiosity and provide a possibility to learn from in the spirit of the demoscene and to allow tinkering and new releases 
in a way that doesn't make it into the commercial space. After a lengthy discussion with Netpoet (on another project) we found that the 
CC-NC license is the closest to what we have in mind, even though it's not a specific software license.

## Contact
Please note that all this is provided "as-is", and releasing this means that from this point forward, we consider this obsolete,
and we will not be providing any degree of support for it.

However, we're more than happy to help if you want to learn more so that you can make your own; mail us your questions 
to crew@conspiracy.hu or find us on one of the many social networks.

Enjoy!

Hugs,

BoyC, Gargaj and Zoom

http://www.conspiracy.hu
