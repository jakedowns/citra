/*
 * Copyright 2023 (c) Leia Inc.  All rights reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Leia Inc. and its suppliers, if any.  The
 * intellectual and technical concepts contained herein are
 * proprietary to Leia Inc. and its suppliers and may be covered
 * by U.S. and Foreign Patents, patents in process, and are
 * protected by trade secret or copyright law.  Dissemination of
 * this information or reproduction of this materials strictly
 * forbidden unless prior written permission is obtained from
 * Leia Inc.
 */
using System;
using System.Runtime.InteropServices;

namespace Leia
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public class Config
    {
        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 2)]
        public float[] dotPitchInMM;
        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 2)]
        public Int32[] panelResolution;
        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 2)]
        public Int32[] numViews;
        public Int32 sharpeningKernelXSize;
        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 18)]
        public float[] sharpeningKernelX;
        public Int32 sharpeningKernelYSize;
        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 18)]
        public float[] sharpeningKernelY;
        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 2)]
        public Int32[] viewResolution;
        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 2)]
        public Int32[] displaySizeInMm;
        public float act_gamma;
        public float act_beta;
        public float act_singleTapCoef;
        public float systemDisparityPercent;
        public float systemDisparityPixels;
        public float cameraCenterX;
        public float cameraCenterY;
        public float cameraCenterZ;
        public float cameraThetaX;
        public float cameraThetaY;
        public float cameraThetaZ;
        public float centerViewNumber;
        public float convergence;
        public float n;
        public float theta;
        public float s;
        public float d_over_n;
        public float p_over_du;
        public float p_over_dv;
        public Int32 colorInversion;
        public Int32 colorSlant;
        public Int32 cameraWidth;
        public Int32 cameraHeight;
        public Int32 cameraFps;
        public float cameraBinningFactor;
        public float facePredictAlphaX;
        public float facePredictAlphaY;
        public float facePredictAlphaZ;
        public float facePredictBeta;
        public float facePredictLatencyMs;
        public float accelerationThreshold;

        public Int32 overlay;
        public Int32 isOverlayDeviceSwitchable;
        public float smooth;
        public float phc;
        public float p1;
        public float p2;
        public float sl1;
        public float sl2;
        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 3)]
        public float[] subpixCenterX;
        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 3)]
        public float[] subpixCenterY;

        public Int32 faceTrackingSingleFaceEnable;
        public float faceTrackingSingleFaceTooFarDistanceThreshold;
        public Int32 faceTrackingSingleFaceTooFarResetTimeoutMs;
        public Int32 faceTrackingMaxNumOfFaces;

        public float faceTrackingHeadPoseZLowPassAlpha;
    }
    public enum FaceTrackingRuntimeType
    {
        IN_SERVICE = 0,
        IN_APP     = 1,
    }
    public class SDKConfig : IDisposable
    {
        public SDKConfig()
        {
            _ptr = leia_core_init_configuration_alloc(Constants.VERSION);
        }
        public void UseOldRenderer()
        {
            leia_core_init_configuration_set_hint(_ptr, "useOldRenderer");
        }
        public void SetEnableValidation(bool enable)
        {
            leia_core_init_configuration_set_enable_validation(_ptr, Convert.ToInt32(enable));
        }
        public void SetPlatformLogLevel(LogLevel logLevel)
        {
            leia_core_init_configuration_set_platform_log_level(_ptr, logLevel);
        }
        public void SetFaceTrackingSharedCameraSink(SharedCameraSink sink)
        {
            IntPtr sinkNative = IntPtr.Zero;
            if (sink != null)
            {
                sinkNative = sink.ReleaseOwnership();
            }
            leia_core_init_configuration_set_face_tracking_shared_camera_sink(_ptr, sinkNative);
        }
        public void SetFaceTrackingRuntime(FaceTrackingRuntimeType runtime)
        {
            leia_core_init_configuration_set_face_tracking_runtime(_ptr, Convert.ToInt32(runtime));
        }
        public void SetFaceTrackingEnable(bool enable)
        {
            leia_core_init_configuration_set_face_tracking_enable(_ptr, Convert.ToInt32(enable));
        }
        public void SetFaceTrackingStart(bool start)
        {
            leia_core_init_configuration_set_face_tracking_start(_ptr, Convert.ToInt32(start));
        }
        public void SetFaceTrackingCheckPermission(bool checkPermission)
        {
            leia_core_init_configuration_set_face_tracking_check_permission(_ptr, Convert.ToInt32(checkPermission));
        }
        public void SetFaceTrackingPermissionDialogKillProcess(bool permissionDialogKillProcess)
        {
            leia_core_init_configuration_set_face_tracking_permission_dialog_kill_process(_ptr, Convert.ToInt32(permissionDialogKillProcess));
        }
        public void SetFaceTrackingServerLogLevel(LogLevel serverLogLevel)
        {
            leia_core_init_configuration_set_face_tracking_server_log_level(_ptr, serverLogLevel);
        }
        public void Dispose()
        {
            leia_core_init_configuration_free(_ptr);
        }
        public IntPtr GetHandle()
        {
            return _ptr;
        }

        private IntPtr _ptr;

        [DllImport(Constants.SDK_DLL_NAME, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr leia_core_init_configuration_alloc([MarshalAs(UnmanagedType.LPStr)] string cnsdkVersion);
        [DllImport(Constants.SDK_DLL_NAME, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_init_configuration_set_hint(IntPtr ptr, [MarshalAs(UnmanagedType.LPStr)] string hint);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_init_configuration_set_enable_validation(IntPtr ptr, Int32 enable);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_init_configuration_set_platform_log_level(IntPtr ptr, LogLevel logLevel);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_init_configuration_set_face_tracking_shared_camera_sink(IntPtr ptr, IntPtr sharedCameraSink);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_init_configuration_set_face_tracking_runtime(IntPtr ptr, Int32 runtime);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_init_configuration_set_face_tracking_enable(IntPtr ptr, Int32 enable);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_init_configuration_set_face_tracking_start(IntPtr ptr, Int32 start);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_init_configuration_set_face_tracking_check_permission(IntPtr ptr, Int32 checkPermission);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_init_configuration_set_face_tracking_permission_dialog_kill_process(IntPtr ptr, Int32 permissionDialogKillProcess);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_init_configuration_set_face_tracking_server_log_level(IntPtr ptr, LogLevel serverLogLevel);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_init_configuration_free(IntPtr ptr);
    }
    public class SDK : IDisposable
    {
        public SDK(SDKConfig config)
        {
            _sdk = leia_core_init_async(config.GetHandle());
            if (_sdk == IntPtr.Zero)
            {
                throw new Exception("Failed to initialize CNSDK");
            }
        }
        public void Dispose()
        {
            if (_sdk != IntPtr.Zero)
            {
                leia_core_shutdown(_sdk);
                _sdk = IntPtr.Zero;
            }
        }
        public bool IsInitialized()
        {
            return leia_core_is_initialized(_sdk) != 0;
        }
        public bool IsValidationEnabled()
        {
            return leia_core_is_validation_enabled(_sdk) != 0;
        }
        public class ConfigHolder : IDisposable
        {
            private SDK sdk;
            private IntPtr configPtr;
            public Config config;
            public ConfigHolder(SDK sdk, IntPtr configPtr)
            {
                this.sdk = sdk;
                this.configPtr = configPtr;
                this.config = new Config();
                Marshal.PtrToStructure(configPtr, this.config);
            }
            public void Sync()
            {
                Marshal.StructureToPtr<Config>(this.config, configPtr, false);
                SDK.leia_core_sync_device_config(sdk.GetNativePtr(), configPtr);
            }
            public void Dispose()
            {
                SDK.leia_core_release_device_config(sdk.GetNativePtr(), configPtr);
            }
        }
        public ConfigHolder GetConfig()
        {
            IntPtr configPtr = leia_core_get_device_config(_sdk);
            if (configPtr == IntPtr.Zero)
            {
                return null;
            }
            return new ConfigHolder(this, configPtr);
        }
        public void SetBacklight(bool enable)
        {
            leia_core_set_backlight(_sdk, Convert.ToInt32(enable));
        }
        public void GetBacklight(out bool isEnabled)
        {
            Int32 isEnabledInt = 0;
            leia_core_get_backlight(_sdk, out isEnabledInt);
            isEnabled = isEnabledInt != 0;
        }
        public bool EnableFacetracking(bool enable)
        {
            return leia_core_enable_face_tracking(_sdk, Convert.ToInt32(enable)) != 0;
        }
        public void StartFacetracking(bool start)
        {
            leia_core_start_face_tracking(_sdk, Convert.ToInt32(start));
        }
        public void SetFaceTrackingConfig(FaceDetectorConfig config)
        {
            leia_core_set_face_detector_config(_sdk, config);
        }
        public void SetFaceTrackingProfiling(bool enable)
        {
            leia_core_set_face_tracking_profiling(_sdk, Convert.ToInt32(enable));
        }
        public bool GetFaceTrackingProfiling(out Leia.HeadTracking.FrameProfiling frameProfiling)
        {
            return leia_core_get_face_tracking_profiling(_sdk, out frameProfiling, Marshal.SizeOf<Leia.HeadTracking.FrameProfiling>()) != 0;
        }
        public void SetFaceTrackingSharedCameraSink(SharedCameraSink sink)
        {
            IntPtr sinkNative = IntPtr.Zero;
            if (sink != null)
            {
                sinkNative = sink.ReleaseOwnership();
            }
            leia_core_set_face_tracking_shared_camera_sink(_sdk, sinkNative);
        }
        public bool GetPrimaryFace(out Vector3 position)
        {
            return leia_core_get_primary_face(_sdk, leia_vector3_to_slice(out position)) != 0;
        }
        public bool GetNonPredictedPrimaryFace(out Vector3 position)
        {
            return leia_core_get_non_predicted_primary_face(_sdk, leia_vector3_to_slice(out position)) != 0;
        }
        public void Resume()
        {
            leia_core_on_resume(_sdk);
        }
        public void Pause()
        {
            leia_core_on_pause(_sdk);
        }
        public IntPtr GetNativePtr()
        {
            return _sdk;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 8)]
        public struct DataSlice
        {
            IntPtr data;
            Int32  length;
        }

        private IntPtr _sdk;

        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr leia_core_init_async(IntPtr config);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern Int32 leia_core_is_initialized(IntPtr sdk);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern Int32 leia_core_is_validation_enabled(IntPtr sdk);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern Int32 leia_core_set_face_detector_config(IntPtr sdk, FaceDetectorConfig config);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern Int32 leia_core_enable_face_tracking(IntPtr sdk, Int32 enable);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_start_face_tracking(IntPtr sdk, Int32 start);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_set_face_tracking_profiling(IntPtr sdk, Int32 enable);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern Int32 leia_core_get_face_tracking_profiling(IntPtr sdk, out Leia.HeadTracking.FrameProfiling profiling, Int32 profilingSizeof);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_set_face_tracking_shared_camera_sink(IntPtr sdk, IntPtr sink);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern Int32 leia_core_get_primary_face(IntPtr sdk, DataSlice position);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern Int32 leia_core_get_non_predicted_primary_face(IntPtr sdk, DataSlice position);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_set_backlight(IntPtr sdk, Int32 enable);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_get_backlight(IntPtr sdk, out Int32 isEnabled);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr leia_core_get_device_config(IntPtr sdk);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_sync_device_config(IntPtr sdk, IntPtr config);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_release_device_config(IntPtr sdk, IntPtr config);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_on_resume(IntPtr sdk);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_on_pause(IntPtr sdk);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_core_shutdown(IntPtr sdk);
        [DllImport(Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern DataSlice leia_vector3_to_slice(out Vector3 v);
    }
}
