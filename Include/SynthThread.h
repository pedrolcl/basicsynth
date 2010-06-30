///////////////////////////////////////////////////////////
/// @file SynthThread.cpp Very simple thread class
//
// Copyright 2010 Daniel R. Mitchell, All Rights Reserved
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
/// @addtogroup grpGeneral
///@{
#if !defined(_SYNTHTHREAD_H)
#define _SYNTHTHREAD_H

class ThreadInfo;

/// @brief Simple thread class.
/// @details This is a portable base class for objects
/// that run on a separate thread. The thread is started
/// with the StartThread function. 
/// The class derived from this class must implement
/// the ThreadProc function.
/// The parent thread should call WaitThread to wait on the
/// child thread to exit. 
class SynthThread
{
public:
	ThreadInfo *info;  ///< Platform specific information

	SynthThread();
	virtual ~SynthThread();

	/// Start the thread.
	/// This will invoke ThreadProc() on the new thread.
	/// The pri parameter sets the thread priority,
	/// 0=normal, 1=above normal, 2=real-time.
	/// @param pri thread priority level
	/// @return 0 on success, -1 on failure
	virtual int StartThread(int pri = 0);

	/// Stop the thread.
	/// This will kill the thread; brutal, not recommended.
	/// @return 0 on success, -1 on failure
	virtual int StopThread();

	/// Wait for the thread to exit.
	/// @return 0 on success, -1 on timeout
	virtual int WaitThread();

	/// Thread procedure.
	/// The default implementation returns immediately.
	/// Derive a class from this one and implement ThreadProc().
	/// @return exit code
	virtual int ThreadProc();

	/// Delay minimum amount.
	virtual void ShortWait();
};

#endif
///@}
