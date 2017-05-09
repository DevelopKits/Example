#ifndef __IMFX_H__
#define __IMFX_H__

#include "sample_defs.h"
#include "imfx_defs.h"
#include "sample_utils.h"
#include "sample_params.h"
#include "hw_device.h"
#include "thread_defs.h"
#include "mfx_buffering.h"
#include "mfxplugin++.h"
#include "vm/thread_defs.h"

#if defined(WIN32) || defined(WIN64)
//#include "d3d_device.h"
#include "d3d_allocator.h"
//#include "d3d11_device.h"
#include "d3d11_allocator.h"
#include "d3d_render.h"
#endif

#if defined LIBVA_SUPPORT
#include "vaapi_allocator.h"
#include "vaapi_device.h"
#endif

#include "general_allocator.h"
#include "sysmem_allocator.h"

#endif
