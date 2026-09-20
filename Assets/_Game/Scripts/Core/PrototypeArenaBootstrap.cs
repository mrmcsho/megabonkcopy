using MegabonkCopy.Player;
using UnityEngine;

namespace MegabonkCopy.Core
{
    public sealed class PrototypeArenaBootstrap : MonoBehaviour
    {
        [Header("Movement Feel")]
        public float moveSpeed = 8f;
        public float sprintSpeed = 12.5f;
        public float groundAcceleration = 48f;
        public float groundDeceleration = 62f;
        public float airAcceleration = 18f;
        [Range(0f, 1f)] public float airControl = 0.65f;
        public float jumpHeight = 2.2f;
        public float gravity = 30f;
        public float maxFallSpeed = 38f;
        public float rotationSpeed = 900f;

        [Header("Camera Feel")]
        public float cameraDistance = 6.5f;
        public float mouseSensitivity = 2.2f;
        public float followSharpness = 18f;
        public float normalFov = 67f;
        public float sprintFov = 73f;

        private Material _groundMaterial;
        private Material _obstacleMaterial;

        private void Awake()
        {
            QualitySettings.vSyncCount = 0;
            Application.targetFrameRate = 120;
            BuildLighting();
            BuildArena();
            BuildPlayerAndCamera();
        }

        private void BuildLighting()
        {
            RenderSettings.ambientLight = new Color(0.42f, 0.46f, 0.52f);
            GameObject sun = new GameObject("Sun");
            Light light = sun.AddComponent<Light>();
            light.type = LightType.Directional;
            light.intensity = 1.2f;
            light.color = new Color(1f, 0.95f, 0.86f);
            sun.transform.rotation = Quaternion.Euler(48f, -32f, 0f);
        }

        private void BuildArena()
        {
            Shader shader = Shader.Find("Universal Render Pipeline/Lit") ?? Shader.Find("Standard");
            _groundMaterial = new Material(shader) { color = new Color(0.22f, 0.27f, 0.3f) };
            _obstacleMaterial = new Material(shader) { color = new Color(0.38f, 0.46f, 0.52f) };

            CreateBlock("Arena Floor", new Vector3(0f, -0.5f, 0f), new Vector3(120f, 1f, 120f), _groundMaterial);

            CreateBlock("North Platform", new Vector3(0f, 1f, 22f), new Vector3(18f, 2f, 12f), _obstacleMaterial);
            CreateRamp("North Ramp", new Vector3(0f, 0.25f, 13.5f), new Vector3(8f, 0.75f, 8f), -10.6f);
            CreateBlock("West Platform", new Vector3(-24f, 2f, -8f), new Vector3(14f, 4f, 14f), _obstacleMaterial);
            CreateRamp("West Ramp", new Vector3(-13.5f, 0.85f, -8f), new Vector3(10f, 1f, 7f), 17f, 90f);
            CreateBlock("East Ledge", new Vector3(25f, 0.75f, 10f), new Vector3(17f, 1.5f, 7f), _obstacleMaterial);

            Vector3[] pillars = { new(-15f, 2f, 18f), new(16f, 2f, -18f), new(33f, 2f, -24f), new(-34f, 2f, -28f) };
            for (int i = 0; i < pillars.Length; i++)
                CreateBlock($"Pillar {i + 1}", pillars[i], new Vector3(3f, 4f, 3f), _obstacleMaterial);

            Vector3[] crates = { new(8f, 1f, 9f), new(11f, 1f, 10f), new(-8f, 1f, -13f), new(-5.5f, 0.75f, -14f) };
            for (int i = 0; i < crates.Length; i++)
                CreateBlock($"Crate {i + 1}", crates[i], Vector3.one * (i == 3 ? 1.5f : 2f), _obstacleMaterial);

            CreateRamp("South Hill", new Vector3(2f, 1.4f, -28f), new Vector3(18f, 3f, 13f), 12f);
        }

        private void BuildPlayerAndCamera()
        {
            GameObject player = GameObject.CreatePrimitive(PrimitiveType.Capsule);
            player.name = "Player";
            player.transform.position = new Vector3(0f, 1.1f, 0f);
            Destroy(player.GetComponent<CapsuleCollider>());
            CharacterController controller = player.AddComponent<CharacterController>();
            controller.height = 2f;
            controller.radius = 0.5f;
            controller.center = Vector3.zero;
            controller.slopeLimit = 52f;
            controller.stepOffset = 0.35f;
            controller.skinWidth = 0.06f;

            ArcadePlayerController movement = player.AddComponent<ArcadePlayerController>();
            movement.moveSpeed = moveSpeed;
            movement.sprintSpeed = sprintSpeed;
            movement.groundAcceleration = groundAcceleration;
            movement.groundDeceleration = groundDeceleration;
            movement.airAcceleration = airAcceleration;
            movement.airControl = airControl;
            movement.jumpHeight = jumpHeight;
            movement.gravity = gravity;
            movement.maxFallSpeed = maxFallSpeed;
            movement.rotationSpeed = rotationSpeed;

            Material playerMaterial = new Material(_obstacleMaterial) { color = new Color(0.12f, 0.75f, 0.92f) };
            player.GetComponent<Renderer>().sharedMaterial = playerMaterial;

            GameObject cameraObject = new GameObject("Third Person Camera");
            cameraObject.tag = "MainCamera";
            Camera cameraComponent = cameraObject.AddComponent<Camera>();
            cameraComponent.nearClipPlane = 0.08f;
            cameraComponent.farClipPlane = 180f;
            cameraObject.AddComponent<AudioListener>();
            ThirdPersonCamera cameraRig = cameraObject.AddComponent<ThirdPersonCamera>();
            cameraRig.distance = cameraDistance;
            cameraRig.mouseSensitivity = mouseSensitivity;
            cameraRig.followSharpness = followSharpness;
            cameraRig.normalFov = normalFov;
            cameraRig.sprintFov = sprintFov;
            cameraRig.Initialize(player.transform, movement);
            movement.SetCamera(cameraObject.transform);
        }

        private GameObject CreateBlock(string objectName, Vector3 position, Vector3 scale, Material material)
        {
            GameObject block = GameObject.CreatePrimitive(PrimitiveType.Cube);
            block.name = objectName;
            block.transform.SetPositionAndRotation(position, Quaternion.identity);
            block.transform.localScale = scale;
            block.GetComponent<Renderer>().sharedMaterial = material;
            return block;
        }

        private void CreateRamp(string objectName, Vector3 position, Vector3 scale, float xAngle, float yAngle = 0f)
        {
            GameObject ramp = CreateBlock(objectName, position, scale, _obstacleMaterial);
            ramp.transform.rotation = Quaternion.Euler(xAngle, yAngle, 0f);
        }
    }
}

