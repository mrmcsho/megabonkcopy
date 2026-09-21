using UnrealBuildTool;

public class megabonkcopyEditorTarget : TargetRules
{
    public megabonkcopyEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("megabonkcopy");
    }
}
