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

namespace Leia {
namespace HeadTracking {
    struct RawFaces
    {
        public int numFaces;

        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = Constants.kMaxNumFaces)]
        public RawFace[] faces;
    }
    struct DetectedFaces
    {
        public int numFaces;

        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = Constants.kMaxNumFaces)]
        public DetectedFace[] faces;
    }
    public class Frame : IDisposable
    {
        public Frame(IntPtr unmanagedHandle)
        {
            _unmanagedHandle = unmanagedHandle;
        }
        public void Dispose()
        {
            if (_unmanagedHandle != IntPtr.Zero)
            {
                leia_headtracking_frame_release(_unmanagedHandle);
                _unmanagedHandle = IntPtr.Zero;
            }
        }
        public void GetTrackingResult(out Result trackingResult)
        {
            Utils.HandleNativeCall(leia_headtracking_frame_get_tracking_result(_unmanagedHandle, out trackingResult));
        }

        private IntPtr _unmanagedHandle;

        [DllImport(Leia.Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern Int32 leia_headtracking_frame_get_tracking_result(IntPtr frame, out Result result);
        [DllImport(Leia.Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern Int32 leia_headtracking_frame_get_raw_faces(IntPtr frame, out RawFaces raw);
        [DllImport(Leia.Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern Int32 leia_headtracking_frame_get_detected_faces(IntPtr frame, out DetectedFaces detected);
        [DllImport(Leia.Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern Int32 leia_headtracking_frame_get_profiling(IntPtr frame, out FrameProfiling frameProfiling);
        [DllImport(Leia.Constants.SDK_DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        private static extern void leia_headtracking_frame_release(IntPtr frame);
    }
} // namespace HeadTracking
} // namespace Leia
