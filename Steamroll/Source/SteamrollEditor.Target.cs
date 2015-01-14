// Copyright Anticto Estudi Binari, SL. All rights reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SteamrollEditorTarget : TargetRules
{
    public SteamrollEditorTarget(TargetInfo Target)
	{
		Type = TargetType.Editor;
	}

	//
	// TargetRules interface.
	//

	public override void SetupBinaries(
		TargetInfo Target,
		ref List<UEBuildBinaryConfiguration> OutBuildBinaryConfigurations,
		ref List<string> OutExtraModuleNames
		)
	{
		OutExtraModuleNames.Add("Steamroll");
	}
}
