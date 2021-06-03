/*******************************************************************************
 * Copyright (c) 1991, 2018 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at https://www.eclipse.org/legal/epl-2.0/
 * or the Apache License, Version 2.0 which accompanies this distribution and
 * is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following
 * Secondary Licenses when the conditions for such availability set
 * forth in the Eclipse Public License, v. 2.0 are satisfied: GNU
 * General Public License, version 2 with the GNU Classpath
 * Exception [1] and GNU General Public License, version 2 with the
 * OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/

#if !defined(CONCURRENTMARKINGSATBDELEGATE_HPP_)
#define CONCURRENTMARKINGSATBDELEGATE_HPP_

#include "j9.h"
#include "j9cfg.h"

#if defined(OMR_GC_MODRON_CONCURRENT_MARK)
#include "j9class.h"
#include "j9consts.h"
#include "j9cp.h"
#include "j9modron.h"
#include "j9nongenerated.h"
#include "j9nonbuilder.h"
#include "modron.h"
#include "objectdescription.h"
#include "omrgcconsts.h"


#include "EnvironmentBase.hpp"
#include "GCExtensionsBase.hpp"
#include "MarkingScheme.hpp"
#include "ReferenceObjectBuffer.hpp"

class GC_VMThreadIterator;
class MM_ConcurrentGCSATB;
class MM_MarkingScheme;

/**
 * Provides language-specific support for SATB marking.
 */
class MM_ConcurrentMarkingSATBDelegate
{
	/*
	 * Data members
	 */
private:
	MM_GCExtensions *_extensions;

protected:
	J9JavaVM *_javaVM;
	GC_ObjectModel *_objectModel;
	MM_ConcurrentGCSATB *_collector;
	MM_MarkingScheme *_markingScheme;

public:
#if defined(J9VM_GC_DYNAMIC_CLASS_UNLOADING)
	bool _dynamicClassUnloadingEnabled;
#endif /* J9VM_GC_DYNAMIC_CLASS_UNLOADING */

private:
protected:
public:
	/**
	 * Initialize the delegate.
	 *
	 * @param env environment for calling thread
	 * @return true if delegate initialized successfully
	 */
	bool initialize(MM_EnvironmentBase *env, MM_ConcurrentGCSATB *collector);
	void mainSetupForGC(MM_EnvironmentBase *env);
	void markLiveObjectsRoots(MM_EnvironmentBase *env);

#if defined(J9VM_GC_DYNAMIC_CLASS_UNLOADING)
	MMINLINE bool isDynamicClassUnloadingEnabled() { return _dynamicClassUnloadingEnabled; };
#endif /* J9VM_GC_DYNAMIC_CLASS_UNLOADING */

	void setThreadsScanned(MM_EnvironmentBase *env);

	/**
	 * Constructor.
	 */
	MMINLINE MM_ConcurrentMarkingSATBDelegate()
		: _extensions(NULL)
		, _javaVM(NULL)
		, _objectModel(NULL)
		, _collector(NULL)
		, _markingScheme(NULL)
#if defined(J9VM_GC_DYNAMIC_CLASS_UNLOADING)
		,_dynamicClassUnloadingEnabled(false)
#endif /* J9VM_GC_DYNAMIC_CLASS_UNLOADING */
	{ }
};

#endif /* defined(OMR_GC_MODRON_CONCURRENT_MARK) */
#endif /* CONCURRENTMARKINGSATBDELEGATE_HPP_ */
