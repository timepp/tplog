/**
 *  implementation of GetLogController
 */

#pragma once

#include "detail/logcontroller.h"


namespace
{
	__declspec(noinline)
	static ILogController* RealGetLogController()
	{
		#pragma warning(push)
		#pragma warning(disable: 4640)
		static CLogController s_controller;  // ��ȷ�����������߳�����֮ǰ����GetLogController
		#pragma warning(pop)

		return &s_controller;
	}
}

TPLOGAPI inline ILogController* GetLogController()
{
	return ::RealGetLogController();
}


