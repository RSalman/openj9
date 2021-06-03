/*******************************************************************************
 * Copyright (c) 1991, 2020 IBM Corp. and others
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

#include "ClassLoaderManager.hpp"
#include "ConcurrentMarkingSATBDelegate.hpp"
#include "ConcurrentGCSATB.hpp"
#include "MarkingSchemeRootMarker.hpp"
#include "VMInterface.hpp"
#include "VMThreadListIterator.hpp"

#if defined(OMR_GC_MODRON_CONCURRENT_MARK)

bool
MM_ConcurrentMarkingSATBDelegate::initialize(MM_EnvironmentBase *env, MM_ConcurrentGCSATB *collector)
{
	_javaVM = (J9JavaVM *)env->getExtensions()->getOmrVM()->_language_vm;
	_objectModel = &(env->getExtensions()->objectModel);
	_markingScheme = collector->getMarkingScheme();
	_collector = collector;
	_extensions = MM_GCExtensions::getExtensions(_javaVM);

	/* set runtime flag for jniDeleteGlobalReference */
	_javaVM->extendedRuntimeFlags |= J9_EXTENDED_RUNTIME_USER_SATB_ACCESS_BARRIER;

	return true;
}

void
MM_ConcurrentMarkingSATBDelegate::mainSetupForGC(MM_EnvironmentBase *env)
{
#if defined(J9VM_GC_DYNAMIC_CLASS_UNLOADING)
	/* Set the dynamic class unloading flag based on command line and runtime state */
	switch (_extensions->dynamicClassUnloading) {
		case MM_GCExtensions::DYNAMIC_CLASS_UNLOADING_NEVER:
			_extensions->runtimeCheckDynamicClassUnloading = false;
			break;
		case MM_GCExtensions::DYNAMIC_CLASS_UNLOADING_ALWAYS:
			_extensions->runtimeCheckDynamicClassUnloading = true;
			break;
		case MM_GCExtensions::DYNAMIC_CLASS_UNLOADING_ON_CLASS_LOADER_CHANGES:
			 _extensions->runtimeCheckDynamicClassUnloading = (_extensions->aggressive || _extensions->classLoaderManager->isTimeForClassUnloading(env));
			break;
		default:
			break;
	}

	_dynamicClassUnloadingEnabled = ((_extensions->runtimeCheckDynamicClassUnloading != 0) ? true : false);
#endif /* J9VM_GC_DYNAMIC_CLASS_UNLOADING */
}


void
MM_ConcurrentMarkingSATBDelegate::markLiveObjectsRoots(MM_EnvironmentBase *env)
{
	/* Reset MM_RootScanner base class for scanning */
	MM_MarkingSchemeRootMarker rootMarker(env, _markingScheme, _markingScheme->getMarkingDelegate());

	/* String table should be clearable, not a hard root */
	rootMarker.setStringTableAsRoot(!_extensions->collectStringConstants);

#if defined(J9VM_GC_DYNAMIC_CLASS_UNLOADING)
	/* Mark root set classes */
	rootMarker.setClassDataAsRoots(!isDynamicClassUnloadingEnabled());
	if (isDynamicClassUnloadingEnabled()) {
		/* Setting the permanent class loaders to scanned without a locked operation is safe
		 * Class loaders will not be rescanned until a thread synchronize is executed
		 */
		if (env->isMainThread()) {
			J9JavaVM * javaVM = (J9JavaVM*)env->getLanguageVM();
			((J9ClassLoader *)javaVM->systemClassLoader)->gcFlags |= J9_GC_CLASS_LOADER_SCANNED;
			_markingScheme->markObject(env, (J9Object *)((J9ClassLoader *)javaVM->systemClassLoader)->classLoaderObject);
			if (javaVM->applicationClassLoader) {
				((J9ClassLoader *)javaVM->applicationClassLoader)->gcFlags |= J9_GC_CLASS_LOADER_SCANNED;
				_markingScheme->markObject(env, (J9Object *)((J9ClassLoader *)javaVM->applicationClassLoader)->classLoaderObject);
			}
		}
	}
#endif /* J9VM_GC_DYNAMIC_CLASS_UNLOADING */

	/* Scan roots */
	rootMarker.scanRoots(env);
}

void
MM_ConcurrentMarkingSATBDelegate::setThreadsScanned(MM_EnvironmentBase *env)
{
	GC_VMInterface::lockVMThreadList(_extensions);

	J9VMThread *walkThread;
	GC_VMThreadListIterator vmThreadListIterator(_javaVM);
	while ((walkThread = vmThreadListIterator.nextVMThread()) != NULL) {
		MM_EnvironmentBase *walkEnv = MM_EnvironmentBase::getEnvironment(walkThread->omrVMThread);
		walkEnv->setAllocationColor(GC_MARK);
		walkEnv->setThreadScanned(true);
	}
	GC_VMInterface::unlockVMThreadList(_extensions);
}

#endif /* defined(OMR_GC_MODRON_CONCURRENT_MARK) */
