/**
 *  implementation of GetLogController
 */

#pragma once

#include "detail/logcontroller.h"


namespace
{
    __declspec(selectany) CLogController g_controller;

	__declspec(noinline)
	static ILogController* RealGetLogController()
	{
		#pragma warning(push)
		#pragma warning(disable: 4640)
//		static CLogController s_controller;  // 请确保在所有新线程启动之前调用GetLogController
		#pragma warning(pop)

		//return &s_controller;
        return &g_controller;
	}
}

TPLOGAPI inline ILogController* GetLogController()
{
	return ::RealGetLogController();
}


