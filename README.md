# RTMeshImporter

An Unreal Engine 5.7 plugin that enables runtime importing of 40+ 3D file formats using the [Assimp](https://github.com/assimp/assimp) (Open Asset Import Library) v6.0.4.

## Features

- Import 3D meshes at runtime from 40+ file formats (FBX, OBJ, glTF, COLLADA, STL, and more)
- Full PBR material pipeline with support for Diffuse, Normal, Metallic, Specular, Roughness, Emissive, and Ambient Occlusion textures
- Handles both external file textures and embedded textures
- UV transform controls (rotation, scale, offset)
- Save and load imported meshes to/from compressed files
- Blueprint-callable file dialogs for 3D files and textures
- Custom vertex transforms on import
- Built on UE5's `ProceduralMeshComponent` (no third-party mesh component plugin required)

## Requirements

- Unreal Engine 5.7
- ProceduralMeshComponent plugin (built-in, enabled by default)

## Supported Platforms

| Platform | Architecture | Status |
|----------|-------------|--------|
| Windows  | x64         | Supported |
| Windows  | x86         | Supported |
| Linux    | x64         | Supported |
| macOS    | ARM64       | Supported |

## Installation

1. Copy the `RTMeshImporter` folder into your project's `Plugins/` directory.
2. Regenerate project files and build.
3. Enable the plugin in your project settings if not already enabled.

## Quick Start

### Blueprint Usage

1. Add a `URTMeshImporterComponent` to any actor in your level.
2. Call `ImportMesh` with a file path to load a 3D model:
   - Returns an `ARTMeshActor` that you can manipulate in-game.
3. Or use `Open3DFileDialog` to let the user pick a file at runtime.

### C++ Usage

```cpp
// Get the importer component
URTMeshImporterComponent* Importer = FindComponentByClass<URTMeshImporterComponent>();

// Import a mesh
ARTMeshActor* MeshActor = Importer->ImportMesh(TEXT("C:/Models/my_model.fbx"));

// Import with custom vertex transform
FTransform CustomTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector(0.01f, 0.01f, 0.01f));
ARTMeshActor* ScaledMesh = Importer->ImportMeshWithTransform(TEXT("C:/Models/my_model.fbx"), CustomTransform);
```

### Saving & Loading

```cpp
// Save all RTMeshActors in the level
Importer->SaveRTMeshesToFile(TEXT("MySave.dat"));

// Load previously saved meshes
Importer->LoadRTMeshesFromFile(TEXT("MySave.dat"));
```

### Changing Textures at Runtime

```cpp
ARTMeshActor* MeshActor = /* your imported mesh */;
UProceduralMeshComponent* Section = /* target section */;
MeshActor->ChangeSectionTexture(Section, FName("DiffuseTexture"), TEXT("C:/Textures/new_diffuse.png"));
```

## Architecture

```
RTMeshImporter/
  Source/RTMeshImporter/
    Public/
      RTMeshImporterComponent.h   - Main importer component (attach to actors)
      RTMeshActor.h               - Actor holding imported mesh sections
      RTSectionActor.h            - Lightweight single-section actor
      RTMeshData.h                - Data structures for mesh/material/texture info
      RTColorPicker.h             - UMG color picker widget
    Private/
      RTMeshImporterComponent.cpp - Assimp parsing, file dialogs, save/load
      RTMeshActor.cpp             - Mesh section creation, material setup, serialization
      RTSectionActor.cpp          - Section actor implementation
      RTColorPicker.cpp           - Color picker implementation
  Content/
    Materials/Master/             - Master PBR material and instances
    Textures/                     - Default placeholder textures
```

### Key Classes

| Class | Description |
|-------|-------------|
| `URTMeshImporterComponent` | Main entry point. Attach to an actor, call `ImportMesh()` to load 3D files. Provides file dialogs and save/load. |
| `ARTMeshActor` | Spawned actor holding imported geometry. Contains a map of `UProceduralMeshComponent` sections (one per material). |
| `FRTMaterialInfo` | Struct carrying material properties and texture info from Assimp to the mesh actor. |
| `ARTSectionActor` | Lightweight wrapper for a single `UProceduralMeshComponent`. |
| `URTColorPicker` | UMG widget for runtime color selection via Slate's `SColorPicker`. |

### Import Flow

1. `URTMeshImporterComponent::ImportMesh()` reads the file via Assimp
2. `ProcessNode()` recursively traverses the Assimp scene graph
3. For each mesh node: vertices, normals, UVs, tangents, and vertex colors are extracted
4. Textures are resolved (embedded or from disk)
5. `ARTMeshActor::DrawMeshSection()` creates a `UProceduralMeshComponent` per material and applies PBR materials

## Material Parameters

The master material (`M_PBR_Runtime_Master`) exposes these parameters:

| Parameter | Type | Description |
|-----------|------|-------------|
| `BaseColor` | Vector | Base color when no diffuse texture |
| `DiffuseTexture` | Texture | Diffuse/albedo map |
| `NormalTexture` | Texture | Normal map |
| `MetallicTexture` | Texture | Metallic map |
| `SpecularTexture` | Texture | Specular map |
| `RoughnessTexture` | Texture | Roughness map |
| `EmissiveTexture` | Texture | Emissive map |
| `AmbientTexture` | Texture | Ambient occlusion map |
| `RotationAngle` | Scalar | UV rotation angle |
| `UOffset` / `VOffset` | Scalar | UV offset |
| `UScale` / `VScale` | Scalar | UV scale |

Each texture slot has a corresponding `Has*Texture` scalar toggle (0 or 1).

## License

Licensed under the MIT License. See [LICENSE](LICENSE) for details.
