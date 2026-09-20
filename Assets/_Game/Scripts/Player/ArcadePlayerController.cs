using UnityEngine;

namespace MegabonkCopy.Player
{
    [RequireComponent(typeof(CharacterController))]
    public sealed class ArcadePlayerController : MonoBehaviour
    {
        [Header("Speed")]
        [Min(0f)] public float moveSpeed = 8f;
        [Min(0f)] public float sprintSpeed = 12.5f;
        [Min(0f)] public float groundAcceleration = 48f;
        [Min(0f)] public float groundDeceleration = 62f;
        [Min(0f)] public float airAcceleration = 18f;
        [Range(0f, 1f)] public float airControl = 0.65f;

        [Header("Jump and Gravity")]
        [Min(0.1f)] public float jumpHeight = 2.2f;
        [Min(0.1f)] public float gravity = 30f;
        [Min(1f)] public float maxFallSpeed = 38f;
        [Min(0f)] public float groundedStickForce = 4f;
        [Min(0f)] public float coyoteTime = 0.12f;
        [Min(0f)] public float jumpBufferTime = 0.12f;

        [Header("Steering")]
        [Min(0f)] public float rotationSpeed = 900f;

        public bool IsSprinting { get; private set; }
        public bool IsGrounded { get; private set; }
        public Vector3 PlanarVelocity => _planarVelocity;

        private CharacterController _controller;
        private Transform _cameraTransform;
        private Vector3 _planarVelocity;
        private float _verticalVelocity;
        private float _lastGroundedTime = float.NegativeInfinity;
        private float _lastJumpPressedTime = float.NegativeInfinity;

        public void SetCamera(Transform cameraTransform) => _cameraTransform = cameraTransform;

        private void Awake()
        {
            _controller = GetComponent<CharacterController>();
        }

        private void Update()
        {
            if (_cameraTransform == null && Camera.main != null)
                _cameraTransform = Camera.main.transform;

            IsGrounded = _controller.isGrounded;
            if (IsGrounded)
                _lastGroundedTime = Time.time;
            if (Input.GetButtonDown("Jump"))
                _lastJumpPressedTime = Time.time;

            Vector2 input = new Vector2(Input.GetAxisRaw("Horizontal"), Input.GetAxisRaw("Vertical"));
            input = Vector2.ClampMagnitude(input, 1f);
            Vector3 desiredDirection = CameraRelativeDirection(input);
            IsSprinting = Input.GetKey(KeyCode.LeftShift) && input.sqrMagnitude > 0.01f;
            float targetSpeed = IsSprinting ? sprintSpeed : moveSpeed;
            Vector3 targetVelocity = desiredDirection * targetSpeed;

            float acceleration;
            if (IsGrounded)
                acceleration = input.sqrMagnitude > 0.01f ? groundAcceleration : groundDeceleration;
            else
                acceleration = airAcceleration * airControl;

            _planarVelocity = Vector3.MoveTowards(_planarVelocity, targetVelocity, acceleration * Time.deltaTime);

            bool canJump = Time.time - _lastGroundedTime <= coyoteTime;
            bool bufferedJump = Time.time - _lastJumpPressedTime <= jumpBufferTime;
            if (canJump && bufferedJump)
            {
                _verticalVelocity = Mathf.Sqrt(2f * gravity * jumpHeight);
                _lastGroundedTime = float.NegativeInfinity;
                _lastJumpPressedTime = float.NegativeInfinity;
                IsGrounded = false;
            }
            else if (IsGrounded && _verticalVelocity < 0f)
            {
                _verticalVelocity = -groundedStickForce;
            }
            else
            {
                _verticalVelocity = Mathf.Max(_verticalVelocity - gravity * Time.deltaTime, -maxFallSpeed);
            }

            if (desiredDirection.sqrMagnitude > 0.001f)
            {
                Quaternion targetRotation = Quaternion.LookRotation(desiredDirection, Vector3.up);
                transform.rotation = Quaternion.RotateTowards(transform.rotation, targetRotation, rotationSpeed * Time.deltaTime);
            }

            CollisionFlags flags = _controller.Move((_planarVelocity + Vector3.up * _verticalVelocity) * Time.deltaTime);
            if ((flags & CollisionFlags.Above) != 0 && _verticalVelocity > 0f)
                _verticalVelocity = 0f;
        }

        private Vector3 CameraRelativeDirection(Vector2 input)
        {
            if (_cameraTransform == null)
                return new Vector3(input.x, 0f, input.y);

            Vector3 forward = Vector3.ProjectOnPlane(_cameraTransform.forward, Vector3.up).normalized;
            Vector3 right = Vector3.ProjectOnPlane(_cameraTransform.right, Vector3.up).normalized;
            return (forward * input.y + right * input.x).normalized * input.magnitude;
        }
    }
}

