#pragma once

//#define PHX_MINIMAL_BUILD

#define PHX_TEXGEN_IMAGE

#define PHX_MESH_CUBE
#define PHX_MESH_SPHERE
#define PHX_MESH_PLANE
#define PHX_MESH_CONE
#define PHX_MESH_CYLINDER
#define PHX_MESH_ARC
#define PHX_MESH_LINE
#define PHX_MESH_SPLINE
#define PHX_MESH_LOFT
#define PHX_MESH_COPY
#define PHX_MESH_CREATEGEOSPHERE
#define PHX_MESH_CREATETREE
#define PHX_MESH_CREATETREELEAVES
#define PHX_MESH_CREATETEXT
#define PHX_MESH_CREATEMARCHINGMESH
#define PHX_MESH_SCATTER
#define PHX_MESH_LOADSTOREDMESH
#define PHX_MESH_LOADSTOREDMINIMESH
#define PHX_MESH_MERGE
#define PHX_MESH_CALCULATETINT
#define PHX_MESH_CALCULATETINTSHAPE
#define PHX_MESH_MAPXFORM
#define PHX_MESH_BEVEL
#define PHX_MESH_REPLICATE
#define PHX_MESH_SMOOTH
#define PHX_MESH_NORMALDEFORM
#define PHX_MESH_CSG
#define PHX_MESH_GREEBLE
#define PHX_MESH_INVERT
#define PHX_MESH_SAVEPOS2
#define PHX_MESH_UV_HAS_CLIPPING

#define PHX_OBJ_MODEL
#define PHX_OBJ_LIGHT
#define PHX_OBJ_CAMEYE
#define PHX_OBJ_DUMMY
#define PHX_OBJ_SUBSCENE
#define PHX_OBJ_EMITTER
#define PHX_OBJ_EMITTERCPU
#define PHX_OBJ_PARTICLEDRAG
#define PHX_OBJ_PARTICLEGRAVITY
#define PHX_OBJ_PARTICLETURBULENCE
#define PHX_OBJ_PARTICLEVORTEX
#define PHX_OBJ_LOGICOBJECT

#define PHX_HAS_STANDARD_PARTICLES
#define PHX_HAS_MESH_PARTICLES
#define PHX_HAS_SUBSCENE_PARTICLES
#define PHX_HAS_PARTICLE_SORTING

#define PHX_EVENT_ENDDEMO
#define PHX_EVENT_RENDERDEMO
#define PHX_EVENT_SHADERTOY
#define PHX_EVENT_RENDERSCENE
#define PHX_EVENT_PARTICLECALC
#define PHX_EVENT_CAMSHAKE
#define PHX_EVENT_CAMOVERRIDE

#define SPLINE_INTERPOLATION_CONSTANT
#define SPLINE_INTERPOLATION_LINEAR
#define SPLINE_INTERPOLATION_CUBIC
#define SPLINE_INTERPOLATION_BEZIER
#define SPLINE_INTERPOLATION_SLERP
#define SPLINE_INTERPOLATION_SQUAD

#define SPLINE_WAVEFORM_SIN
#define SPLINE_WAVEFORM_SQUARE
#define SPLINE_WAVEFORM_TRIANGLE
#define SPLINE_WAVEFORM_SAWTOOTH
#define SPLINE_WAVEFORM_NOISE
#define SPLINE_WAVEFORM_HASADDITIVE
#define SPLINE_HASZEROKEYEXPORT

#define HAS_TECH_PARAMS

//manual config part follows cranked to the max:
//#define PHX_SPLINE_WAVEFORM_SUPPORT

#define PHX_ARBARO_HAVE_HELIX_SUPPORT
#define PHX_ARBARO_HAVE_PRUNING
#define PHX_ARBARO_HAVE_FAN_LEAVES
#define PHX_ARBARO_HAVE_NEGATIVE_TRUNK_ROTANGLE

#define PHX_ARBARO_HAS_SHAPE_CONICAL
#define PHX_ARBARO_HAS_SHAPE_SPHERICAL
#define PHX_ARBARO_HAS_SHAPE_HEMISPHERICAL
#define PHX_ARBARO_HAS_SHAPE_CYLINDRICAL
#define PHX_ARBARO_HAS_SHAPE_TAPERED_CYLINDRICAL
#define PHX_ARBARO_HAS_SHAPE_FLAME
#define PHX_ARBARO_HAS_SHAPE_INVERSE_CONICAL
#define PHX_ARBARO_HAS_SHAPE_TEND_FLAME

#define PHX_SCATTER_HAS_VERTEX_SUPPORT
#define PHX_SCATTER_HAS_EDGE_SUPPORT
#define PHX_SCATTER_HAS_POLY_SUPPORT
#define PHX_SCATTER_HAS_POLY_MORE_THAN_1_INSTANCE_SUPPORT
#define PHX_SCATTER_HAS_ORIENTATION_ORIGINAL
#define PHX_SCATTER_HAS_ORIENTATION_NORMAL
#define PHX_SCATTER_HAS_ORIENTATION_NORMALROTATE
#define PHX_SCATTER_HAS_ORIENTATION_FULLROTATE


//#define PHX_QUATERNION_HAS_QUADRATIC
#define PHX_HAS_NON_DEMO_RELATIVE_RENDERTARGETS
#define PHX_HAS_SCENE_OBJECT_HIERARCHIES

#define LTC1
#define LTC2

#define SETUPBOX_HAS_SOCIAL

//#define PHX_VOLUMETRIC_RENDERTARGETS