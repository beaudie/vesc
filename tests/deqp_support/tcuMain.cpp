/*-------------------------------------------------------------------------
 * drawElements Quality Program Tester Core
 * ----------------------------------------
 *
 * Copyright (C) 2008-2013 drawElements Ltd. All rights reserved.
 * Use of this material is subject to a licensing agreement between
 * user and drawElements Ltd. Please contact info@drawelements.com
 * for more information.
 *
 *//*!
 * \file
 * \brief Generic main().
 *//*--------------------------------------------------------------------*/

#include "tcuDefs.hpp"
#include "tcuCommandLine.hpp"
#include "tcuPlatform.hpp"
#include "tcuApp.hpp"
#include "tcuResource.hpp"
#include "tcuTestLog.hpp"
#include "deUniquePtr.hpp"

#include <cstdio>

// Implement this in your platform port.
tcu::Platform* createPlatform (void);

__declspec(dllexport) int deqp_main (int argc, const char* argv[])
{
#if (DE_OS != DE_OS_WIN32)
	// Set stdout to line-buffered mode (will be fully buffered by default if stdout is pipe).
	setvbuf(stdout, DE_NULL, _IOLBF, 4*1024);
#endif

	try
	{
		tcu::CommandLine				cmdLine		(argc, argv);
		tcu::DirArchive					archive		(".");
		tcu::TestLog					log			(cmdLine.getLogFileName(), cmdLine.getLogFlags());
		de::UniquePtr<tcu::Platform>	platform	(createPlatform());
		de::UniquePtr<tcu::App>			app			(new tcu::App(*platform, archive, log, cmdLine));

		// Main loop.
		for (;;)
		{
			if (!app->iterate())
				break;
		}
	}
	catch (const std::exception& e)
	{
		tcu::die("%s", e.what());
	}

	return 0;
}
