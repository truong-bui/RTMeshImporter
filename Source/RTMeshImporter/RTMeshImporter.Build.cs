/************************************************************************************
 *																					*
 * Copyright (C) 2021 Truong Bui.													*
 * Website:	https://github.com/truong-bui/RTMeshImporter							*
 * Licensed under the MIT License. See 'LICENSE' file for full license information. *
 *																					*
 ************************************************************************************/

using System.IO;
using UnrealBuildTool;

public class RTMeshImporter : ModuleRules
{
	// return current directory (RTMeshImporter) path
	private string ModulePath
    {
		get { return ModuleDirectory; }
    }

	// return ThirdParty directory path
	private string ThirdPartyPath
    {
		get { return Path.GetFullPath(Path.Combine(ModulePath, "../ThirdParty/")); }
    }

	private string BinariesPath
    {
		get { return Path.GetFullPath(Path.Combine(ModulePath, "../../Binaries/")); }
    }

	public RTMeshImporter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange( new string[] { Path.Combine(ThirdPartyPath, "assimp/include") } );
				
		
		PrivateIncludePaths.AddRange( new string[] {} );			
		
		PublicDependencyModuleNames.AddRange( new string[] { "Core", "ProceduralMeshComponent" } );			
		
		PrivateDependencyModuleNames.AddRange( new string[] { "CoreUObject", "Engine", "Slate", "SlateCore", "UMG", "AppFramework" } );
		
		string PlatformDir = (Target.Platform == UnrealTargetPlatform.Win64) ? "Win64" : "Win32";
			string LibraryName = (Target.Platform == UnrealTargetPlatform.Win64) ? "assimp-vc142-mt.lib" : "assimp-vc141-mt.lib";
			string RuntimeLibName = (Target.Platform == UnrealTargetPlatform.Win64) ? "assimp-vc142-mt.dll" : "assimp-vc141-mt.dll";

			PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "assimp/lib", PlatformDir, LibraryName));

			RuntimeDependencies.Add(Path.Combine(BinariesPath, PlatformDir, RuntimeLibName));

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
