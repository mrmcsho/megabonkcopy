using UnrealBuildTool;

public class megabonkcopyTarget : TargetRules
{
    public megabonkcopyTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("megabonkcopy");
    }
}
