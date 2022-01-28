//
// Created by luchu on 2022/1/28.
//

#pragma once

#include "Core/Object.h"


namespace My3D
{
    /// Resource reloading started.
    MY3D_EVENT(E_RELOADSTARTED, ReloadStarted)
    {
    }
    /// Resource reloading finished successfully.
    MY3D_EVENT(E_RELOADFINISHED, ReloadFinished)
    {
    }
    /// Resource reloading failed.
    MY3D_EVENT(E_RELOADFAILED, ReloadFailed)
    {
    }
    /// Tracked file changed in the resource directories.
    MY3D_EVENT(E_FILECHANGED, FileChanged)
    {
        MY3D_PARAM(P_FILENAME, FileName);                    // String
        MY3D_PARAM(P_RESOURCENAME, ResourceName);            // String
    }
    /// Resource loading failed.
    MY3D_EVENT(E_LOADFAILED, LoadFailed)
    {
        MY3D_PARAM(P_RESOURCENAME, ResourceName);            // String
    }
    /// Resource not found.
    MY3D_EVENT(E_RESOURCENOTFOUND, ResourceNotFound)
    {
        MY3D_PARAM(P_RESOURCENAME, ResourceName);            // String
    }
    /// Unknown resource type.
    MY3D_EVENT(E_UNKNOWNRESOURCETYPE, UnknownResourceType)
    {
        MY3D_PARAM(P_RESOURCETYPE, ResourceType);            // StringHash
    }
    /// Resource background loading finished.
    MY3D_EVENT(E_RESOURCEBACKGROUNDLOADED, ResourceBackgroundLoaded)
    {
        MY3D_PARAM(P_RESOURCENAME, ResourceName);            // String
        MY3D_PARAM(P_SUCCESS, Success);                      // bool
        MY3D_PARAM(P_RESOURCE, Resource);                    // Resource pointer
    }
    /// Language changed.
    MY3D_EVENT(E_CHANGELANGUAGE, ChangeLanguage)
    {
    }
}
