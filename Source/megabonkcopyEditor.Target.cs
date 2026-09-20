using UnrealBuildTool;

public class megabonkcopyEditorTarget : TargetRules
{
    public megabonkcopyEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        ExtraModuleNames.Add("megabonkcopy");
    }
}
