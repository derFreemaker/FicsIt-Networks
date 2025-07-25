﻿using UnrealBuildTool;

public class FicsItNetworksMicrocontroller : ModuleRules
{
    public FicsItNetworksMicrocontroller(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;
        bWarningsAsErrors = true;
        bEnableExceptions = true;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "SML",
                "FactoryGame",
                "FicsItNetworksCircuit",
                "FicsItNetworksComputer",
                "FicsItNetworksLua",
                "FicsItReflection",
                "FicsItLogLibrary",
                "Eris"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
            }
        );
    }
}