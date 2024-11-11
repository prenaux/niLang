#pragma once
#ifndef __GDRV_GPU_H_40BE24B3_D4BE_3B4B_A652_DE433A111C36__
#define __GDRV_GPU_H_40BE24B3_D4BE_3B4B_A652_DE433A111C36__

#include "API/niUI/Experimental/IGpu.h"
#include "API/niUI/GraphicsEnum.h"

const sGpuBlendModeDesc& ToGpuBlendModeDesc(eBlendMode aBlendMode);
iGpuPipelineDesc* CreateGpuPipelineDesc();

#endif // __GDRV_GPU_H_40BE24B3_D4BE_3B4B_A652_DE433A111C36__
