using UnrealBuildTool;

public class megabonkcopyTarget : TargetRules
{
    public megabonkcopyTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        ExtraModuleNames.Add("megabonkcopy");
    }
}
