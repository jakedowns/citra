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
namespace Leia
{
    struct Constants
    {
	    // TODO: dynamic switching
#if UNITY_STANDALONE_WIN || UNITY_EDITOR_WIN
        public const string SDK_DLL_NAME = @"leiaSDK-faceTrackingInApp.dll";
#else
        public const string SDK_DLL_NAME = @"leiaSDK-faceTrackingInService";
#endif
        public const string VERSION = @"0.7.28";
    }
}
