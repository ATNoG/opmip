//=============================================================================
// Brief   : Debug Helpers for GNU (glibc and gcc based/compatible)
// Authors : Bruno Santos <bsantos@av.it.pt>
//------------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2009-2011 Universidade Aveiro
// Copyright (C) 2009-2011 Instituto de Telecomunicações - Pólo Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//==============================================================================

#include <opmip/debug.hpp>
#include <cstdio>
#include <cstring>

#include <execinfo.h>
#include <signal.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace opmip {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void signal_handler(int, siginfo_t* info, void* context)
{
	struct sigcontext* mctx = (struct sigcontext*) &((ucontext_t*) context)->uc_mcontext;
	crash_ctx ctx;

	ctx.code = info->si_signo;
#ifdef __i386__
	ctx.address = (void*) mctx->eip;
#elif __amd64__
	ctx.address = (void*) mctx->rip;
#else
#	error "Add support for this architecture"
#endif
	ctx.context = mctx;
	crash(ctx);
}

static void dump_context(void* context)
{
	::sigcontext* ctx = (::sigcontext*) context;

	std::fprintf(stderr,	"\n"
							"== context ==\n"
#ifdef __i386__
							"eax = %08lX  edx = %08lX  ecx = %08lX  ebx = %08lX\n"
							"edi = %08lX  esi = %08lX  ebp = %08lX  esp = %08lX\n"
							"\n"
							"eip = %08lX  flags = %08lX\n",
							ctx->eax, ctx->edx, ctx->ecx, ctx->ebx,
							ctx->edi, ctx->esi, ctx->ebp, ctx->esp,
							ctx->eip, ctx->eflags
#elif __amd64__
							"rax = %016lX  rdx = %016lX  rcx = %016lX\n"
							"rbx = %016lX  rdi = %016lX  rsi = %016lX\n"
							"rbp = %016lX  rsp = %016lX  r8  = %016lX\n"
							"r9  = %016lX  r10 = %016lX  r11 = %016lX\n"
							"r12 = %016lX  r13 = %016lX  r14 = %016lX\n"
							"r15 = %016lX\n"
							"\n"
							"rip = %016lX  flags = %08lX\n",
							ctx->rax, ctx->rdx, ctx->rcx, ctx->rbx,
							ctx->rdi, ctx->rsi, ctx->rbp, ctx->rsp,
							ctx->r8 , ctx->r9 , ctx->r10, ctx->r11,
							ctx->r12, ctx->r13, ctx->r14, ctx->r15,
							ctx->rip, ctx->eflags
#else
#	error "Add support for this architecture"
#endif
	                        );
}

static void dump_backtrace(void* address)
{
	void* trace[128];
	int i = 0;
	int n;

	std::memset(trace, 0, sizeof(trace));
	n = ::backtrace(trace, OPMIP_COUNT_OF(trace));
	if (address)
		for(; i < n; ++i)
			if (trace[i] == address)
				break;

	std::fprintf(stderr, "\n== stack backtrace ==\n");
	std::fflush(stderr);
	::backtrace_symbols_fd(trace + i, n - i, STDERR_FILENO);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup_crash_handler()
{
	struct sigaction handler;

	handler.sa_sigaction = signal_handler;
	handler.sa_flags = SA_SIGINFO;
	sigemptyset(&handler.sa_mask);
	sigaction(SIGFPE, &handler, nullptr);
	sigaction(SIGILL, &handler, nullptr);
	sigaction(SIGINT, &handler, nullptr);
	sigaction(SIGSEGV, &handler, nullptr);
	sigaction(SIGTRAP, &handler, nullptr);
	//SIGSTKFLT
}

void crash(const crash_ctx& ctx)
{
	std::fprintf(stderr,	 "\n"
							 "================================= Crash Dump ==================================\n");

	::psignal(ctx.code, "Reason");

	if (ctx.file)
		std::fprintf(stderr, "Function   : %s\n"
							 "File       : %s\n"
							 "Line       : %u\n",
							 ctx.function,
							 ctx.file,
							 ctx.line);

	if (ctx.expression)
		std::fprintf(stderr, "Expression : %s\n",
							 ctx.expression);

	if (ctx.context)
		dump_context(ctx.context);

	dump_backtrace(ctx.address);

	std::fprintf(stderr,	 "===============================================================================\n"
							 "\n");
	std::fflush(stderr);
	std::abort();
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
