#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2021-2022 by Dolby Laboratories.
 ***************************************************************************/

#ifndef DLB_COMMS_EXPORT
#ifdef WIN32

#ifdef DLB_COMMS_BUILDING_MEDIA
#define DLB_COMMS_MEDIA_EXPORT __declspec(dllexport)
#else  // not building media
#define DLB_COMMS_MEDIA_EXPORT __declspec(dllimport)
#endif

#ifdef DLB_COMMS_BUILDING_SDK
#define DLB_COMMS_EXPORT __declspec(dllexport)
#else  // not building sdk
#define DLB_COMMS_EXPORT __declspec(dllimport)
#endif

#ifdef DLB_COMMS_BUILDING_MULTIMEDIA_ADDON
#define DLB_COMMS_ADDON_EXPORT __declspec(dllexport)
#else  // not building sdk
#define DLB_COMMS_ADDON_EXPORT __declspec(dllimport)
#endif

#define DLB_COMMS_TEMPLATE_EXPORT

#else  // not windows
#define DLB_COMMS_MEDIA_EXPORT __attribute__((visibility("default")))
#define DLB_COMMS_EXPORT __attribute__((visibility("default")))
#define DLB_COMMS_ADDON_EXPORT __attribute__((visibility("default")))
#define DLB_COMMS_TEMPLATE_EXPORT __attribute__((visibility("default")))
#endif
#endif
