#if UNITY_EDITOR
using UnityEditor;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;

namespace MegabonkCopy.Editor
{
    [InitializeOnLoad]
    internal static class UrpProjectSetup
    {
        private const string PipelinePath = "Assets/_Game/Settings/PrototypeURP.asset";
        private const string RendererPath = "Assets/_Game/Settings/PrototypeRenderer.asset";

        static UrpProjectSetup()
        {
            EditorApplication.delayCall += EnsurePipeline;
        }

        private static void EnsurePipeline()
        {
            UniversalRenderPipelineAsset pipeline = AssetDatabase.LoadAssetAtPath<UniversalRenderPipelineAsset>(PipelinePath);
            if (pipeline == null)
            {
                EnsureFolder("Assets/_Game", "Settings");
                UniversalRendererData renderer = ScriptableObject.CreateInstance<UniversalRendererData>();
                renderer.name = "PrototypeRenderer";
                AssetDatabase.CreateAsset(renderer, RendererPath);
                pipeline = UniversalRenderPipelineAsset.Create(renderer);
                pipeline.name = "PrototypeURP";
                AssetDatabase.CreateAsset(pipeline, PipelinePath);
                AssetDatabase.SaveAssets();
            }

            if (GraphicsSettings.defaultRenderPipeline != pipeline)
                GraphicsSettings.defaultRenderPipeline = pipeline;
            if (QualitySettings.renderPipeline != pipeline)
                QualitySettings.renderPipeline = pipeline;
        }

        private static void EnsureFolder(string parent, string child)
        {
            string path = parent + "/" + child;
            if (!AssetDatabase.IsValidFolder(path))
                AssetDatabase.CreateFolder(parent, child);
        }
    }
}
#endif
