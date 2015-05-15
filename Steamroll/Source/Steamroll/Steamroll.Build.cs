// Copyright Anticto Estudi Binari, SL. All rights reserved.

using UnrealBuildTool;

public class Steamroll : ModuleRules
{
    public Steamroll(TargetInfo Target)
	{
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "RHI", "RenderCore", "ShaderCore", "UMG", "Slate", "SlateCore" });
	}
}
