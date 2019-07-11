#ifndef _TES31CONTEXT_HPP
#define _TES31CONTEXT_HPP
//
//  Copyright 2019 The ANGLE Project Authors. All rights reserved.
//  Use of this source code is governed by a BSD-style license that can be
//  found in the LICENSE file.
//
//  tes31Context_override.hpp:
//    Issue 3687: Overrides for dEQP's OpenGL ES 3.1 test context
//

// Keep the delta compared to dEQP at a minimum
// clang-format off

#include "tcuDefs.hpp"
#include "tcuTestContext.hpp"

namespace glu
{
class RenderContext;
class ContextInfo;
}

namespace tcu
{
class RenderTarget;
}

namespace deqp
{
namespace gles31
{

class Context
{
public:
									Context					(tcu::TestContext& testCtx);
									~Context				(void);

	tcu::TestContext&				getTestContext			(void)			{ return m_testCtx;			}
	glu::RenderContext&				getRenderContext		(void)			{ return *m_renderCtx;		}
	const glu::ContextInfo&			getContextInfo			(void) const	{ return *m_contextInfo;	}
	const tcu::RenderTarget&		getRenderTarget			(void) const;

private:
									Context					(const Context& other);
	Context&						operator=				(const Context& other);

	void							createRenderContext		(void);
	void							destroyRenderContext	(void);

	tcu::TestContext&				m_testCtx;
	glu::RenderContext*				m_renderCtx;
	glu::ContextInfo*				m_contextInfo;
};

} // gles31
} // deqp

// clang-format off

#endif // _TES31CONTEXT_HPP
