using UnityEngine;

namespace MegabonkCopy.Player
{
    [RequireComponent(typeof(Camera))]
    public sealed class ThirdPersonCamera : MonoBehaviour
    {
        [Header("Orbit")]
        public Vector3 pivotOffset = new Vector3(0f, 1.45f, 0f);
        [Min(1f)] public float distance = 6.5f;
        [Min(0.01f)] public float mouseSensitivity = 2.2f;
        public float minPitch = -30f;
        public float maxPitch = 68f;
        [Min(0f)] public float followSharpness = 18f;

        [Header("Collision")]
        [Min(0.01f)] public float collisionRadius = 0.25f;
        [Min(0f)] public float collisionPadding = 0.15f;
        [Min(0f)] public float minimumDistance = 0.6f;
        public LayerMask collisionLayers = ~0;

        [Header("Field of View")]
        [Range(30f, 100f)] public float normalFov = 67f;
        [Range(30f, 110f)] public float sprintFov = 73f;
        [Min(0f)] public float fovSharpness = 7f;

        private Transform _target;
        private ArcadePlayerController _player;
        private Camera _camera;
        private Vector3 _smoothedPivot;
        private float _yaw;
        private float _pitch = 18f;

        public void Initialize(Transform target, ArcadePlayerController player)
        {
            _target = target;
            _player = player;
            _smoothedPivot = target.position + pivotOffset;
            _yaw = target.eulerAngles.y;
        }

        private void Awake()
        {
            _camera = GetComponent<Camera>();
            _camera.fieldOfView = normalFov;
        }

        private void Start()
        {
            Cursor.lockState = CursorLockMode.Locked;
            Cursor.visible = false;
        }

        private void LateUpdate()
        {
            if (_target == null)
                return;

            _yaw += Input.GetAxisRaw("Mouse X") * mouseSensitivity;
            _pitch = Mathf.Clamp(_pitch - Input.GetAxisRaw("Mouse Y") * mouseSensitivity, minPitch, maxPitch);
            Quaternion orbit = Quaternion.Euler(_pitch, _yaw, 0f);

            float followT = 1f - Mathf.Exp(-followSharpness * Time.deltaTime);
            _smoothedPivot = Vector3.Lerp(_smoothedPivot, _target.position + pivotOffset, followT);
            Vector3 backward = orbit * Vector3.back;
            float resolvedDistance = distance;
            if (Physics.SphereCast(_smoothedPivot, collisionRadius, backward, out RaycastHit hit,
                    distance, collisionLayers, QueryTriggerInteraction.Ignore))
                resolvedDistance = Mathf.Max(minimumDistance, hit.distance - collisionPadding);

            transform.SetPositionAndRotation(_smoothedPivot + backward * resolvedDistance, orbit);
            float targetFov = _player != null && _player.IsSprinting ? sprintFov : normalFov;
            float fovT = 1f - Mathf.Exp(-fovSharpness * Time.deltaTime);
            _camera.fieldOfView = Mathf.Lerp(_camera.fieldOfView, targetFov, fovT);

            if (Input.GetKeyDown(KeyCode.Escape))
            {
                Cursor.lockState = CursorLockMode.None;
                Cursor.visible = true;
            }
            if (Input.GetMouseButtonDown(0))
            {
                Cursor.lockState = CursorLockMode.Locked;
                Cursor.visible = false;
            }
        }
    }
}

